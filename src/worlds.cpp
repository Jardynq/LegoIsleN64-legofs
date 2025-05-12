#include <cctype>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <climits>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <malloc.h>

#include "worlds.h"
#include "anim.h"
#include "dump_assets.h"
#include "utils.h"

/*
	Unknown color alias: indir-g-dbfbrdy0
	Unknown color alias: indir-g-dbbkfny0
	Unknown color alias: indir-f-dbfrxly0
	Unknown color alias: indir-f-dbbkxly0
	Unknown color alias: indir-g-dbbkxly0
	Unknown color alias: indir-f-dbbkxly0
	Unknown color alias: indir-f-dbfrxly0
	Unknown color alias: indir-g-dbfrxly0
	Unknown color alias: indir-g-dbfbrdy0
	Unknown color alias: indir-g-dbbkfny0
	Unknown color alias: indir-f-chblady0
	Unknown color alias: indir-f-chblady0
	Unknown color alias: indir-f-chblady0
	Unknown color alias: indir-g-chblady0
	Unknown color alias: indir-g-chmotry0
	Unknown color alias: indir-g-rcmotry0
	Unknown color alias: indir-g-rcbacky6
	Unknown color alias: indir-g-rcfrnty6
	Unknown color alias: indir-f-rcstery0
	Unknown color alias: indir-g-rctailya
	Unknown color alias: indir-f-rcwhl1y0
	Unknown color alias: indir-f-rcwhl2y0
	Unknown color alias: indir-g-rcmotry0
	Unknown color alias: indir-g-rcfrmey0
	Unknown color alias: indir-g-rcbacky6
	Unknown color alias: indir-g-rcfrnty6
	Unknown color alias: indir-f-rcwhl1y0
	Unknown color alias: indir-f-rcwhl2y0
	Unknown color alias: indir-g-rctailya
	Unknown color alias: indir-f-rcstery0
	Unknown color alias: indir-f-chblady0
	Unknown color alias: indir-f-chblady0
	Unknown color alias: indir-f-chblady0
	Unknown color alias: indir-g-chblady0
	Unknown color alias: indir-g-chmotry0
	Unknown color alias: indir-g-dbfbrdy0
	Unknown color alias: indir-g-dbbkfny0
	Unknown color alias: indir-f-dbfrxly0
	Unknown color alias: indir-f-dbbkxly0
	Unknown color alias: indir-g-dbbkxly0
	Unknown color alias: indir-f-dbbkxly0
	Unknown color alias: indir-f-dbfrxly0
	Unknown color alias: indir-g-dbfrxly0
	Unknown color alias: indir-g-dbfbrdy0
	Unknown color alias: indir-g-dbbkfny0
	Unknown color alias: indir-g-jsfrnty5
	Unknown color alias: indir-g-jsskiby0
	Unknown color alias: indir-g-jswnshy5
	Unknown color alias: indir-g-jsskiby0
	Unknown color alias: indir-g-jswnshy5
	Unknown color alias: indir-g-jsfrnty5
	Unknown color alias: indir-g-jsskiby0
	Unknown color alias: indir-g-jswnshy5
	Unknown color alias: indir-g-jsskiby0
	Unknown color alias: indir-g-jswnshy5
	Unknown color alias: indir-g-rcmotry0
	Unknown color alias: indir-g-rcbacky6
	Unknown color alias: indir-g-rcfrnty6
	Unknown color alias: indir-f-rcstery0
	Unknown color alias: indir-g-rctailya
	Unknown color alias: indir-f-rcwhl1y0
	Unknown color alias: indir-f-rcwhl2y0
	Unknown color alias: indir-g-rcmotry0
	Unknown color alias: indir-g-rcfrmey0
	Unknown color alias: indir-g-rcbacky6
	Unknown color alias: indir-g-rcfrnty6
	Unknown color alias: indir-f-rcwhl1y0
	Unknown color alias: indir-f-rcwhl2y0
	Unknown color alias: indir-g-rctailya
	Unknown color alias: indir-f-rcstery0
*/

static std::unordered_map<std::string, const char*> g_colorNameMap {
	{"indir-f-dbfrfny4", "inh lego red"},
	{"indir-f-dbfbrdy0", "inh lego red"},
	{"indir-f-dbbkfny0", "inh lego red"},
	{"indir-f-dbhndly0", "inh lego white"},
	{"indir-f-dbltbry0", "inh lego white"},
	{"indir-g-dbltbry0", "inh lego white"},
	{"indir-g-dbflagy0", "inh lego yellow"},
	{"indir-f-dbflagy0", "inh lego yellow"},
	{"indir-f-jsexhy0", "inh lego black"},
	{"indir-g-jsexhy0", "inh lego black"},
	{"indir-g-jsbasey0", "inh lego white"},
	{"indir-f-jsfrnty5", "inh lego black"},
	{"indir-f-jslsidy0", "inh lego black"},
	{"indir-f-jsrsidy0", "inh lego black"},
	{"indir-g-jshndly0", "inh lego red"},
	{"indir-f-jshndly0", "inh lego red"},
	{"indir-f-jsskiby0", "inh lego red"},
	{"indir-f-jsdashy0", "inh lego white"},
	{"indir-f-jswnshy5", "inh lego white"},
	{"indir-f-rcfrmey0", "inh lego red"},
	{"indir-f-rcmotry0", "inh lego white"},
	{"indir-f-rcbacky6", "inh lego green"},
	{"indir-f-rcedgey0", "inh lego green"},
	{"indir-f-rcfrnty6", "inh lego green"},
	{"indir-f-rctailya", "inh lego white"},
	{"indir-f-rcstrpy0", "inh lego yellow"},
	{"indir-f-rcfrmey0", "inh lego red"},
	{"indir-f-rcsidey0", "inh lego green"},
};

static std::unordered_map<std::string, ColorA> g_roiColorAliases = {
	{"inh lego black", ColorA{0x21, 0x21, 0x21, 0}},
	{"inh lego black f", ColorA{0x21, 0x21, 0x21, 0}},
	{"inh lego black flat", ColorA{0x21, 0x21, 0x21, 0}},
	{"inh lego blue", ColorA{0x00, 0x54, 0x8c, 0}},
	{"inh lego blue flat", ColorA{0x00, 0x54, 0x8c, 0}},
	{"inh lego brown", ColorA{0x4a, 0x23, 0x1a, 0}},
	{"inh lego brown flt", ColorA{0x4a, 0x23, 0x1a, 0}},
	{"inh lego brown flat", ColorA{0x4a, 0x23, 0x1a, 0}},
	{"inh lego drk grey", ColorA{0x40, 0x40, 0x40, 0}},
	{"inh lego drk grey flt", ColorA{0x40, 0x40, 0x40, 0}},
	{"inh lego dk grey flt", ColorA{0x40, 0x40, 0x40, 0}},
	{"inh lego green", ColorA{0x00, 0x78, 0x2d, 0}},
	{"inh lego green flat", ColorA{0x00, 0x78, 0x2d, 0}},
	{"inh lego lt grey", ColorA{0x82, 0x82, 0x82, 0}},
	{"inh lego lt grey flt", ColorA{0x82, 0x82, 0x82, 0}},
	{"inh lego lt grey fla", ColorA{0x82, 0x82, 0x82, 0}},
	{"inh lego red", ColorA{0xcb, 0x12, 0x20, 0}},
	{"inh lego red flat", ColorA{0xcb, 0x12, 0x20, 0}},
	{"inh lego white", ColorA{0xfa, 0xfa, 0xfa, 0}},
	{"inh lego white flat", ColorA{0xfa, 0xfa, 0xfa, 0}},
	{"inh lego yellow", ColorA{0xff, 0xb9, 0x00, 0}},
	{"inh lego yellow flat", ColorA{0xff, 0xb9, 0x00, 0}},
};

bool has_inh_prefix(const char* p_name) {
    if (p_name != NULL) {
        if (!strnicmp(p_name, "inh", strlen("inh"))) {
            return true;
        }
    }
    return false;
}

bool ColorAliasLookup(
	const char* p_param,
	float& p_red,
	float& p_green,
	float& p_blue,
	float& p_alpha
) {
	if (g_colorNameMap.find(p_param) != g_colorNameMap.end()) {
		p_param = g_colorNameMap[p_param];
	}

	if (g_roiColorAliases.find(p_param) != g_roiColorAliases.end()) {
		ColorA color = g_roiColorAliases[p_param];
		p_red = color.m_red / 255.0;
		p_green = color.m_green / 255.0;
		p_blue = color.m_blue / 255.0;
		p_alpha = color.m_alpha / 255.0;
		return true;
	}

	printf("Unknown color alias: %s\n", p_param);
	return false;
}

const char* ReadString(int length, char** mem, bool toLower = true) {
	char* str = new char[length + 1];
	memread(str, length, mem);
	str[length] = '\0';
	if (toLower) {
		strlwr(str);
	}
	return str;
}

void InsertLod(const char* p_roiname, Lod* lod) {
	if (g_lod_map.find(p_roiname) == g_lod_map.end()) {
		g_lod_map[p_roiname] = std::vector<Lod*>();
	}
	g_lod_map[p_roiname].push_back(lod);
}
void InsertLods(const char* p_roiname, std::vector<Lod*>& lods) {
	if (g_lod_map.find(p_roiname) == g_lod_map.end()) {
		g_lod_map[p_roiname] = std::vector<Lod*>();
	}
	for (auto& lod : lods) {
		g_lod_map[p_roiname].push_back(lod);
	}
}
std::vector<Lod*>* LookupLods(const char* p_roiname) {
	auto it = g_lod_map.find(p_roiname);
	if (it != g_lod_map.end()) {
		return &g_lod_map[p_roiname];
	}
	return nullptr;
}



bool MeshInfo::Read(char** mem) {
	m_color.m_red = memread(m_color.m_red, mem);
	m_color.m_green = memread(m_color.m_green, mem);
	m_color.m_blue = memread(m_color.m_blue, mem);
	m_alpha = memread(m_alpha, mem);
	m_shading = memread(m_shading, mem);
	m_unk0x0d = memread(m_unk0x0d, mem);
	m_unk0x20 = memread(m_unk0x20, mem);
	m_doMaterialLookup = memread(m_doMaterialLookup, mem);

	unsigned int textureLength = memread(textureLength, mem);
	if (textureLength > 0) {
        m_textureName = ReadString(textureLength, mem);
		replace_end(m_textureName, ".png", ".gif");
    }

	unsigned int materialLength = memread(materialLength, mem);
	if (materialLength > 0) {
		m_materialName = ReadString(materialLength, mem);
    }

    return true;
}

bool Mesh::Read(char** mem) {
	return true;
}

bool Lod::Read(char** mem) {
	float(*normals)[3] = NULL;
	float(*vertices)[3] = NULL;
	float(*textureVertices)[2] = NULL;
	unsigned int(*polyIndices)[3] = NULL;
	unsigned int(*textureIndices)[3] = NULL;
	unsigned char paletteEntries[256];

	m_unk0x08 = memread(m_unk0x08, mem);
	if (m_unk0x08 & 0xffffff04) {
		return true;
	}

	unsigned int numMeshes = memread(numMeshes, mem);
	if (numMeshes == 0) {
		return true;
	}

	unsigned int meshUnd1 = numMeshes - 1;
	unsigned int  meshUnd2 = 0;

	unsigned int tempNumVertsAndNormals = memread(tempNumVertsAndNormals, mem);
	int numTextureVertices = memread(numTextureVertices, mem);
	int numVerts = *((unsigned short*) &tempNumVertsAndNormals) & SHRT_MAX;
	int numNormals = (*((unsigned short*) &tempNumVertsAndNormals + 1) >> 1) & SHRT_MAX;

	if (numVerts > 0) {
		vertices = new float[numVerts][3];
		memread(vertices, numVerts * sizeof(*vertices), mem);
	}
	if (numNormals > 0) {
		normals = new float[numNormals][3];
		memread(normals, numNormals * sizeof(*normals), mem);
	}
	if (numTextureVertices > 0) {
		textureVertices = new float[numTextureVertices][2];
		memread(textureVertices, numTextureVertices * sizeof(*textureVertices), mem);
	}

	for (int i = 0; i < numMeshes; i++) {
		m_melems.push_back(new Mesh());
	}

	for (int i = 0; i < numMeshes; i++) {
		unsigned int numPolys = memread<unsigned short>(mem) & USHRT_MAX;
		unsigned int numVertices = memread<unsigned short>(mem) & USHRT_MAX;
		m_numPolys += numPolys;
		m_numVertices += numVertices;

		polyIndices = new unsigned int[numPolys][sizeOfArray(*polyIndices)];
		memread (polyIndices, numPolys * sizeof(*polyIndices), mem);

		unsigned int numTextureIndices = memread(numTextureIndices, mem);
		if (numTextureIndices > 0) {
			textureIndices = new unsigned int[numPolys][sizeOfArray(*textureIndices)];
			memread(textureIndices, numPolys * sizeof(*textureIndices), mem);
		} else {
			textureIndices = NULL;
		}

		MeshInfo mesh;
		mesh.Read(mem);

		const char* textureName = mesh.m_textureName;
		const char* materialName = mesh.m_materialName;
		unsigned int meshIndex = 0;
		if (has_inh_prefix(textureName) || has_inh_prefix(materialName)) {
			meshIndex = meshUnd1;
			meshUnd1--;
		} else {
			meshIndex = meshUnd2;
			meshUnd2++;
		}

		Mesh* data = m_melems[meshIndex];
		data->faceCount = numPolys & USHRT_MAX;
		data->vertexCount = numVertices & USHRT_MAX;
		data->uvCount = numTextureIndices & USHRT_MAX;
		data->pPositions = vertices;
		data->pNormals = normals;
		data->pTextureCoordinates = textureVertices;
		data->pFaceIndices = polyIndices;
		data->pTextureIndices = textureIndices;
				
		if (textureName != nullptr) {
			char* memtemp = (char*)textureName;
			data->textureName = ReadString(strlen(textureName), &memtemp);
			data->m_red = 1.0f;
			data->m_green = 1.0f;
			data->m_blue = 1.0f;
			data->m_alpha = 1.0f;
			data->m_unk0x04 = true;
		} else {
			float red = 1.0F;
			float green = 0.0F;
			float blue = 1.0F;
			float alpha = 0.0F;
			if (mesh.m_doMaterialLookup) {
				// TODO this might cause issues, since the material name could be reffering to an MX variable
				ColorAliasLookup(materialName, red, green, blue, alpha);
			} else {
				red = mesh.m_color.m_red/ 255.0;
				green = mesh.m_color.m_green / 255.0;
				blue = mesh.m_color.m_blue / 255.0;
				alpha = mesh.m_alpha;
			}
			data->m_red = red;
			data->m_green = green;
			data->m_blue = blue;
			data->m_alpha = 1.0f - alpha;
			/*
			data->pTextureCoordinates = nullptr;
			if (data->pTextureIndices != nullptr) {
				delete[] data->pTextureIndices;
				data->pTextureIndices = nullptr;
			}
			*/
		}
	}
	
	m_unk0x1c = meshUnd2;

	return true;
}

bool TextureData::Read(char** mem) {
	m_width = memread(m_width, mem);
	m_height = memread(m_height, mem);
	unsigned int count = memread(count, mem);

	for (unsigned int i = 0; i < count; i++) {
		Color color;
		color.m_red = memread(color.m_red, mem);
		color.m_green = memread(color.m_green, mem);
		color.m_blue = memread(color.m_blue, mem);
		m_palette.push_back(color);
	}

	m_bits.resize(m_width * m_height);
	memread(m_bits.data(), m_width * m_height, mem);

	return true;
}

bool Texture::Read(char** mem) {
	unsigned int namelen = memread(namelen, mem);
	m_name = ReadString(namelen, mem);

	if (m_name[0] == '^') {
		memmove((char*)m_name, m_name + 1, strlen(m_name));
		m_data.Read(mem);
		
		TextureData discardTexture;
		discardTexture.Read(mem);
	} else {
		m_data.Read(mem);
	}

	replace_end(m_name, ".png", ".gif");
	return true;
}

bool PartRef::Read(char** mem) {
	unsigned int namelen = memread(namelen, mem);
	m_roiname = ReadString(namelen, mem);
	m_partDataLength = memread(m_partDataLength, mem);
	m_partDataOffset = memread(m_partDataOffset, mem);
	return true;
}

bool PartData::Read(char** mem) {
	unsigned int roiNameLength = memread(roiNameLength, mem);
	m_roiname = ReadString(roiNameLength, mem);

	unsigned int numLODs = memread(numLODs, mem);
	m_roiInfoOffset = memread(m_roiInfoOffset, mem);
	for (int i = 0; i < numLODs; i++) {
		Lod* lod = new Lod();
		lod->Read(mem);
		m_lods.push_back(lod);
		InsertLod(m_roiname, lod);
	}

	return true;
}

bool Part::Read(char** mem) {
	char* memstart = *mem;
	unsigned int textureInfoOffset = memread(textureInfoOffset, mem);
	char* memtex = memstart + textureInfoOffset;

	unsigned int numTextures = memread(numTextures, &memtex);
    for (int i = 0; i < numTextures; i++) {
        Texture* tex = new Texture();
		tex->Read(&memtex);
		m_textures.push_back(tex);
    }

	unsigned int numROIs = memread(numROIs, mem);
    for (int i = 0; i < numROIs; i++) {
		PartData* partdata = new PartData;
		partdata->Read(mem);
		m_data.push_back(partdata);
		*mem = memstart + partdata->m_roiInfoOffset;
	}

	return true;
}

bool Roi::Read(char** mem, char* memstart) {
	// If it's parent component with no children, then include that component
	// Otherwise ignore the parent component, and only include the children.
	int numChildren = 0;
	do {
		RoiData* component = new RoiData();
		component->Read(mem, memstart);

		numChildren = memread(numChildren, mem);
		if (numChildren == 0) {
			m_components.push_back(component);
		}
	} while (numChildren-- > 0);
	
	return true;
}

bool RoiData::Read(char** mem, char* memstart) {
	unsigned int namelen = memread(namelen, mem);
	m_name = ReadString(namelen, mem);

	memread(m_circle_center, 3 * sizeof(*m_circle_center), mem);
	m_circle_radius = memread(m_circle_radius, mem);
	memread(m_aabb_min, 3 * sizeof(*m_aabb_min), mem);
	memread(m_aabb_max, 3 * sizeof(*m_aabb_max), mem);

	unsigned int texNameLen = memread(texNameLen, mem);
	const char* textureName = ReadString(texNameLen, mem);
	replace_end(textureName, ".png", ".gif");

	m_lodsAlreadyLoaded = memread(m_lodsAlreadyLoaded, mem);
	if (m_lodsAlreadyLoaded) {
		int roiLength = namelen;
		while (roiLength > 0) {
			if (m_name[roiLength - 1] < '0' || m_name[roiLength - 1] > '9') {
				break;
			}
			roiLength -= 1;
		}
		char* memtemp = (char*)m_name;
		m_roiname = ReadString(roiLength, &memtemp);
	} else {
		unsigned int numLODs = memread(numLODs, mem);
		if (numLODs > 0) {
			const char* roiName = m_name;
			unsigned int offset = memread(offset, mem);
			if (strnicmp(m_name, "bike", 4) == 0) {
				roiName = (char*)"bike";
			} else if (strnicmp(m_name, "moto", 4) == 0) {
				roiName = (char*)"moto";
			}

			std::vector<Lod*>* lodList = LookupLods(roiName);
			if (lodList != nullptr) {
				if (strcmpi(roiName, "rcuser") == 0 ||
					strcmpi(roiName, "jsuser") == 0 || 
					strcmpi(roiName, "dunebugy") == 0 || 
					strcmpi(roiName, "chtrblad") == 0 || 
					strcmpi(roiName, "chtrbody") == 0 || 
					strcmpi(roiName, "chtrshld") == 0
				) {
					lodList->clear();
					for (int i = 0; i < numLODs; i++) {
						Lod* lod = new Lod();
						lod->Read(mem);
						m_lods.push_back(lod);
						InsertLod(roiName, lod);
					}
				}
			} else {
				for (int i = 0; i < numLODs; i++) {
					Lod* lod = new Lod();
					lod->Read(mem);
					m_lods.push_back(lod);
					InsertLod(roiName, lod);
				}
			}

			if (m_roiname != nullptr) {
				delete[] m_roiname;
			}
			char* memtemp = (char*)roiName;
			m_roiname = ReadString(strlen(roiName), &memtemp);
			*mem = memstart + offset;
		}
	}

	// Set texture to all sub components
	if (texNameLen > 0) {
		float red = 1.0F;
		float green = 1.0F;
		float blue = 1.0F;
		float alpha = 0.0F;
		if (!strnicmp(textureName, "t_", 2)) {
			// Contains a texture
			SetMaterial(red, green, blue, 1.0f - alpha, textureName);
		} else {
			// Contains a material
			ColorAliasLookup(textureName, red, green, blue, alpha);
			SetMaterial(red, green, blue, 1.0f - alpha, nullptr);
		}
	}

	return true;
};

void RoiData::SetMaterial(float r, float g, float b, float a, const char* textureName) {
	for (auto lod : m_lods) {
		for (auto mesh : lod->m_melems) {
			mesh->m_red = 1.0f;
			mesh->m_green = 1.0f;
			mesh->m_blue = 1.0f;
			mesh->m_alpha = 1.0f;
			if (mesh->textureName != NULL) {
				delete[] mesh->textureName;
				mesh->textureName = nullptr;
			}
			if (textureName != NULL) {
				char* memtemp = (char*)textureName;
				mesh->textureName = ReadString(strlen(textureName), &memtemp);
			}
		}
	}
}


bool ModelRef::Read(char** mem) {
	unsigned int len = memread(len, mem);
	m_modelName = ReadString(len, mem);

	m_dataLength = memread(m_dataLength, mem);
	m_dataOffset = memread(m_dataOffset, mem);

	len = memread(len, mem);
	m_presenterName = ReadString(len, mem, false);
	
	memread(m_location, sizeof(*m_location) * 3, mem);
	memread(m_direction, sizeof(*m_direction) * 3, mem);
	memread(m_up, sizeof(*m_up) * 3, mem);
	m_isVisible = memread(m_isVisible, mem);

	return true;
}

bool Model::Read(char** mem) {
	char* memstart = *mem;
	unsigned int version = memread(version, mem);
	if (version != 19) {
		return false;
	}

	unsigned int textureInfoOffset = memread(textureInfoOffset, mem);
	char* memtex = memstart + textureInfoOffset;

	unsigned int numTextures = memread(numTextures, &memtex);
	unsigned int skipTextures = memread(skipTextures, &memtex);

	for (int i = 0; i < numTextures; i++) {
		Texture* tex = new Texture();
		tex->Read(&memtex);
		m_textures.push_back(tex);
	}

	char* memroi = *mem;
	unsigned int numROIs = memread(numROIs, mem);
	m_anim.Read(mem, false);
	m_roi.Read(mem, memstart);

	return true;
}

bool WorldRef::Read(char** mem) {
	unsigned int worldNameLen = memread(worldNameLen, mem);
	m_worldName = ReadString(worldNameLen, mem);

	unsigned int numParts = memread(numParts, mem);
	for (int i = 0; i < numParts; i++) {
		PartRef* partref = new PartRef();
		partref->Read(mem);
		m_partrefs.push_back(partref);
	}

	unsigned int numModels = memread(numModels, mem);
	for (int i = 0; i < numModels; i++) {
		ModelRef* model = new ModelRef();
		model->Read(mem);
		m_modelrefs.push_back(model);
	}

	return true;
}

bool World::Read(char** mem, char* memstart) {
	WorldRef* ref = new WorldRef();
	ref->Read(mem);
	m_worldName = ref->m_worldName;

	for (auto ref : ref->m_partrefs) {
		char* mempart = memstart + ref->m_partDataOffset;
		Part* part = new Part();
		part->ref = ref;
		part->Read(&mempart);
		m_parts.push_back(part);
	}
	for (auto ref : ref->m_modelrefs) {
		char* memmodel = memstart + ref->m_dataOffset;
		Model* model = new Model();
		model->ref = ref;
		model->Read(&memmodel);
		m_models.push_back(model);
	}

	return true;
}

WorldDB WorldDB::Read(const char* path) {
	WorldDB result;
	
	FILE* file = fopen(path, "rb");
	if (file == NULL) {
		printf("Failed to open file: %s\n", path);
		return result;
	}
	fseek(file, 0, SEEK_END);
	unsigned int fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);
	char* mem = new char[fileSize];
	if (fread(mem, 1, fileSize, file) != fileSize) {
		printf("Failed to read file: %s\n", path);
		delete[] mem;
		fclose(file);
		return result;
	}
	fclose(file);

	char* memstart = mem;
	unsigned int numWorlds = memread(numWorlds, &mem);
	for (int i = 0; i < numWorlds; i++) {
		World world;
		world.Read(&mem, memstart);
		result.worlds.push_back(world);
	}

	unsigned int size = memread(size, &mem);
	char* memtex = mem;
	mem += size;

	std::vector<Texture*> textures;
	unsigned int numTextures = memread(numTextures, &memtex);
	for (int i = 0; i < numTextures; i++) {
		Texture* texture = new Texture();
		texture->Read(&memtex);
		textures.push_back(texture);
	}

	size = memread(size, &mem);
	Part* part = new Part();
	part->Read(&mem);
	part->ref = new PartRef();
	part->ref->m_roiname = "shared";
	part->m_textures.insert(part->m_textures.end(), textures.begin(), textures.end());

	result.shared.m_worldName = "shared";
	result.shared.m_parts.push_back(part);

	delete[] memstart;
	return result;
}

void write_world(const World &world, const std::string &dest) {
	auto path = dest + "index_" + world.m_worldName;
	FILE* file = fopen(path.c_str(), "wb");
	if (file == NULL) {
		printf("Failed to open file: %s\n", path.c_str());
		return;
	}

	unsigned short size = world.m_parts.size();
	fwrite2(&size, sizeof(size), 1, file);
	for (auto part : world.m_parts) {
		size = strlen(part->ref->m_roiname);
		fwrite2(&size, sizeof(size), 1, file);
		fwrite2(part->ref->m_roiname, sizeof(char), size, file);
		unsigned char lod_count = part->m_data[0]->m_lods.size();
		fwrite2(&lod_count, sizeof(lod_count), 1, file);
	}

	size = world.m_models.size();
	fwrite2(&size, sizeof(size), 1, file);
	for (auto model : world.m_models) {
		auto& ref = model->ref;
		size = strlen(ref->m_modelName);
		fwrite2(&size, sizeof(size), 1, file);
		fwrite2(ref->m_modelName, sizeof(char), size, file);
		
		size = strlen(ref->m_presenterName);
		fwrite2(&size, sizeof(size), 1, file);
		fwrite2(ref->m_presenterName, sizeof(char), size, file);

		fwrite2(&ref->m_center[0], sizeof(ref->m_center[0]), 1, file);
		fwrite2(&ref->m_center[1], sizeof(ref->m_center[1]), 1, file);
		fwrite2(&ref->m_center[2], sizeof(ref->m_center[2]), 1, file);
		fwrite2(&ref->m_location[0], sizeof(ref->m_location[0]), 1, file);
		fwrite2(&ref->m_location[1], sizeof(ref->m_location[1]), 1, file);
		fwrite2(&ref->m_location[2], sizeof(ref->m_location[2]), 1, file);
		fwrite2(&ref->m_direction[0], sizeof(ref->m_direction[0]), 1, file);
		fwrite2(&ref->m_direction[1], sizeof(ref->m_direction[1]), 1, file);
		fwrite2(&ref->m_direction[2], sizeof(ref->m_direction[2]), 1, file);
		fwrite2(&ref->m_up[0], sizeof(ref->m_up[0]), 1, file);
		fwrite2(&ref->m_up[1], sizeof(ref->m_up[1]), 1, file);
		fwrite2(&ref->m_up[2], sizeof(ref->m_up[2]), 1, file);

		fwrite2(&ref->m_isVisible, sizeof(ref->m_isVisible), 1, file);

		size = model->m_roi.m_components.size();
		fwrite2(&size, sizeof(size), 1, file);
		for (auto comp : model->m_roi.m_components) {
			size = strlen(comp->m_roiname);
			fwrite2(&size, sizeof(size), 1, file);
			fwrite2(comp->m_roiname, sizeof(char), size, file);
			unsigned char lod_count = comp->m_lods.size();
			fwrite2(&lod_count, sizeof(lod_count), 1, file);
		}
	}
	fclose(file);
}

void handle_world(World& world, const std::string& dest, std::unordered_map<std::string, std::mutex*>* mutexes) {
	for (auto model : world.m_models) {
		for (auto texture : model->m_textures) {
			std::string texture_path = dest + texture->m_name;
			(*mutexes)[texture->m_name]->lock();
			replace_end(texture_path.c_str(), ".bmp", ".png");
			dump_texture(*texture, texture_path.c_str());
			(*mutexes)[texture->m_name]->unlock();
		}
		
		dump_components(model, dest, mutexes);
	}

	for (auto part : world.m_parts) {
		for (auto texture : part->m_textures) {
			(*mutexes)[texture->m_name]->lock();
			std::string texture_path = dest + texture->m_name;
			replace_end(texture_path.c_str(), ".bmp", ".png");
			dump_texture(*texture, texture_path.c_str());
			(*mutexes)[texture->m_name]->unlock();
		}
		for (auto data : part->m_data) {
			int i_lod = 0;
			for (auto lod : data->m_lods) {
				float zero[3] = {0.0f, 0.0f, 0.0f};
				dump_lod(*lod, zero, dest, data->m_roiname, i_lod, (*mutexes)[data->m_roiname]);
				i_lod += 1;
			}
		}
	}

	write_world(world, dest);
	//world.free();
}
