#include <climits>
#include <cstdio>
#include <cstring>
#include <string>

#include "dump_assets.h"
#include "utils.h"


bool dump_texture(const Texture& texture, const char* path) {
    FILE* fp = fopen(path, "wb");
    if (!fp) return false;

    const TextureData& data = texture.m_data;

    unsigned char bmpHeader[14] = {
        'B', 'M', 0, 0, 0, 0, 0, 0, 0, 0,
        54, 0, 0, 0 
    };

    unsigned char dibHeader[40] = {0};
    int rowSize = ((data.m_width * 3 + 3) / 4) * 4;
    int imageSize = rowSize * data.m_height;
    int fileSize = 54 + imageSize;

    bmpHeader[2] = fileSize & 0xFF;
    bmpHeader[3] = (fileSize >> 8) & 0xFF;
    bmpHeader[4] = (fileSize >> 16) & 0xFF;
    bmpHeader[5] = (fileSize >> 24) & 0xFF;

    dibHeader[0] = 40;
    dibHeader[4] = data.m_width & 0xFF;
    dibHeader[5] = (data.m_width >> 8) & 0xFF;
    dibHeader[6] = (data.m_width >> 16) & 0xFF;
    dibHeader[7] = (data.m_width >> 24) & 0xFF;

    dibHeader[8]  = data.m_height & 0xFF;
    dibHeader[9]  = (data.m_height >> 8) & 0xFF;
    dibHeader[10] = (data.m_height >> 16) & 0xFF;
    dibHeader[11] = (data.m_height >> 24) & 0xFF;

    dibHeader[12] = 1;
    dibHeader[14] = 24;
    dibHeader[20] = imageSize & 0xFF;
    dibHeader[21] = (imageSize >> 8) & 0xFF;
    dibHeader[22] = (imageSize >> 16) & 0xFF;
    dibHeader[23] = (imageSize >> 24) & 0xFF;

    fwrite(bmpHeader, 1, 14, fp);
    fwrite(dibHeader, 1, 40, fp);

    unsigned char* row = new unsigned char[rowSize];
    for (int y = data.m_height - 1; y >= 0; --y) {
        memset(row, 0, rowSize);
        for (unsigned int x = 0; x < data.m_width; ++x) {
            unsigned char index = data.m_bits[y * data.m_width + x];
            row[x * 3 + 0] = data.m_palette[index].m_blue;
            row[x * 3 + 1] = data.m_palette[index].m_green;
            row[x * 3 + 2] = data.m_palette[index].m_red;
        }
        fwrite(row, 1, rowSize, fp);
    }

    delete[] row;
    fclose(fp);
    return true;
}


struct Vertex {
    float position[3];
    float normal[3];
    float tu;
    float tv;
    float color[3];
};
void CreateMesh(
	unsigned int faceCount,
	unsigned int vertexCount,
	float (*pPositions)[3],
	float (*pNormals)[3],
	float (*pTextureCoordinates)[2],
	unsigned int (*pFaceIndices)[3],
	unsigned int (*pTextureIndices)[3],
    unsigned int** out_indices,
    Vertex** out_vertices
) {
	unsigned int* faceIndices = (unsigned int*)pFaceIndices;
	int count = faceCount * 3;
	int index = 0;

	unsigned int* fData = new unsigned int[count];

	Vertex* vertices = new Vertex[vertexCount];
	memset(vertices, 0, sizeof(*vertices) * vertexCount);
	for (int i = 0; i < count; i++) {
        unsigned short* indexPtr = (unsigned short*)&faceIndices[i];
		if ((*(indexPtr + 1) >> 0x0f) & 0x01) {
            unsigned int j = *indexPtr;
			vertices[index].position[0] = pPositions[j][0];
			vertices[index].position[1] = pPositions[j][1];
			vertices[index].position[2] = pPositions[j][2];
			j = *(indexPtr + 1) & SHRT_MAX;
			vertices[index].normal[0] = pNormals[j][0];
			vertices[index].normal[1] = pNormals[j][1];
			vertices[index].normal[2] = pNormals[j][2];

            if (pTextureIndices != NULL && pTextureCoordinates != NULL) {
				j = ((unsigned int*) pTextureIndices)[i];
                vertices[index].tu = pTextureCoordinates[j][0];
                vertices[index].tv = pTextureCoordinates[j][1];
            }

			fData[i] = index;
			index++;
		}
		else {
			fData[i] = *indexPtr;
		}
	}

    *out_indices = fData;
    *out_vertices = vertices;
}
bool dump_mtl(const Lod& lod, const char* filepath) {
    FILE* f = fopen(filepath, "w");
    if (!f) return false;

    for (unsigned int m = 0; m < lod.m_melems.size(); m++) {
        const Mesh* mesh = lod.m_melems[m];
        fprintf(f, "newmtl mesh_%u_material\n", m);
        fprintf(f, "Kd %f %f %f\n", mesh->m_red, mesh->m_green, mesh->m_blue);
        fprintf(f, "d %f\n", mesh->m_alpha);
        if (mesh->textureName) {
            fprintf(f, "map_Kd %s\n", mesh->textureName);
        }
        fprintf(f, "\n");
    }

    fclose(f);
    return true;
}
bool dump_lod(const Lod& lod, const char* filepath) {
    if (lod.m_melems.empty()) return false;

    FILE* f = fopen(filepath, "w");
    if (!f) return false;

    const char* matfile = strdup(filepath);
    replace_end(matfile, ".mtl", ".obj");
    dump_mtl(lod, matfile);
    fprintf(f, "mtllib %s\n", matfile);
    
    unsigned int vertexOffset = 1;
    for (unsigned int m = 0; m < lod.m_melems.size(); ++m) {
        const Mesh* mesh = lod.m_melems[m];

        Vertex* vertices = nullptr;
        unsigned int* indices = nullptr;
        CreateMesh(
            mesh->faceCount,
            mesh->vertexCount,
            mesh->pPositions,
            mesh->pNormals,
            mesh->pTextureCoordinates,
            mesh->pFaceIndices,
            mesh->pTextureIndices,
            &indices,
            &vertices
        );

        fprintf(f, "g mesh_%u\n", m);
        fprintf(f, "usemtl mesh_%u_material\n", m);

        for (unsigned int i = 0; i < mesh->vertexCount; ++i) {
            auto p = vertices[i].position;
            fprintf(f, "v %f %f %f\n", p[0], p[1], p[2]);
        }

        for (unsigned int i = 0; i < mesh->vertexCount; ++i) {
            auto n = vertices[i].normal;
            fprintf(f, "vn %f %f %f\n", n[0], n[1], n[2]);
        }

        if (mesh->pTextureCoordinates) {
            for (unsigned int i = 0; i < mesh->vertexCount; ++i) {
                float u = vertices[i].tu;
                float v = vertices[i].tv;
                fprintf(f, "vt %f %f\n", u, 1.0f - v);
            }
        }

        for (unsigned int i = 0; i < mesh->faceCount * 3; i += 3) {
            unsigned int a = indices[i] + vertexOffset;
            unsigned int b = indices[i + 1] + vertexOffset;
            unsigned int c = indices[i + 2] + vertexOffset;

            // TODO vertex normals are not correct
            if (mesh->pTextureCoordinates) {
                fprintf(f, "f %u/%u/%u %u/%u/%u %u/%u/%u\n", a, a, a, b, b, b, c, c, c);
            } else {
                fprintf(f, "f %u//%u %u//%u %u//%u\n", a, a, b, b, c, c);
            }
        }

        vertexOffset += mesh->vertexCount;

        delete[] vertices;
        delete[] indices;
    }

    fclose(f);
    return true;
}
