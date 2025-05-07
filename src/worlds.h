#pragma once

#include "anim.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>


struct ColorA {
	int m_red = 255;
	int m_green = 0;
	int m_blue = 255;
	int m_alpha = 255;
};

struct Color {
	unsigned char m_red = 255;
	unsigned char m_green = 0;
	unsigned char m_blue = 255;
};

struct TextureData {
	bool Read(char** mem);

	unsigned int m_width = 0;
	unsigned int m_height = 0;
	unsigned int m_count = 0;
	std::vector<Color> m_palette;
	unsigned char* m_bits = nullptr;
};

struct Texture {
	bool Read(char** mem);

	const char* m_name = nullptr;
	TextureData m_data;
};

struct MeshInfo {
	bool Read(char** mem);

	enum { e_flat, e_gouraud, e_wireframe };
	struct Unknown {
		unsigned char m_unk0x00[8];
		unsigned char* m_unk0x08 = nullptr;
		unsigned char* m_unk0x0c = nullptr;
		unsigned char* m_unk0x10 = nullptr;
		unsigned char* m_unk0x14 = nullptr;
		unsigned char* m_unk0x18 = nullptr;
	};
	
	Color m_color;
	float m_alpha = 0.0f;
	unsigned char m_shading = 0;
	unsigned char m_unk0x0d = 0;
	Unknown* m_unk0x10  = nullptr;
	unsigned int m_unk0x14 = 0;
	const char* m_textureName = nullptr;
	const char* m_materialName = nullptr;
	unsigned char m_unk0x20 = 0;
	bool m_doMaterialLookup = 0;
};

struct Mesh {
	bool Read(char** mem);

	const char* textureName = nullptr;
	float m_red = 0.0f;
	float m_green = 0.0f;
	float m_blue = 0.0f;
	float m_alpha = 0.0f;
	int m_unk0x04 = 0;
	unsigned int faceCount = 0;
	unsigned int vertexCount = 0;
	unsigned int uvCount = 0;
	float (*pPositions)[3] = nullptr;
	float (*pNormals)[3] = nullptr;
	float (*pTextureCoordinates)[2] = nullptr;
	unsigned int (*pFaceIndices)[3] = nullptr;
	unsigned int (*pTextureIndices)[3] = nullptr;
};

struct Lod {
	bool Read(char** mem);

	std::vector<Mesh*> m_melems;
	unsigned int m_numVertices = 0;
	unsigned int m_numPolys = 0;
	unsigned int m_unk0x1c = 0;
	unsigned int m_unk0x08 = 0;
};

struct PartRef {
	bool Read(char** mem);

	const char* m_roiname = nullptr;
	unsigned int m_partDataLength = 0;
	unsigned int m_partDataOffset = 0;
};

struct PartData {
	bool Read(char** mem);

	const char* m_roiname = nullptr;
	std::vector<Lod*> m_lods;
	unsigned int m_roiInfoOffset = 0;
};

struct Part {
	bool Read(char** mem);

	PartRef* ref = nullptr;
	std::vector<Texture*> m_textures;
	std::vector<PartData*> m_data;
};

struct RoiData {
	bool Read(char** mem, char* memstart);
	void SetMaterial(float r, float g, float b, float a, const char* textureName);
	
	bool m_lodsAlreadyLoaded = 0;
	const char* m_name = nullptr;
	const char* m_roiname = nullptr;
	std::vector<Lod*> m_lods;

	float m_circle_center[3] = {0.0f, 0.0f, 0.0f};
	float m_circle_radius = 0.0f;

	float m_aabb_min[3] = {0.0f, 0.0f, 0.0f};
	float m_aabb_max[3] = {0.0f, 0.0f, 0.0f};
};

struct Roi {
	bool Read(char** mem, char* memstart);
	
	std::vector<RoiData*> m_components;
};

struct ModelRef {
	bool Read(char** mem);

	const char* m_modelName = nullptr;
	unsigned int m_dataLength = 0;
	unsigned int m_dataOffset = 0;
	const char* m_presenterName = nullptr;
	float m_location[3] = {0.0f, 0.0f, 0.0f};
	float m_direction[3] = {0.0f, 0.0f, 0.0f};
	float m_up[3] = {0.0f, 0.0f, 0.0f};
	bool m_isVisible = 0;
};

struct Model {
	bool Read(char** mem);

	ModelRef* ref;
	const char* m_roiname = nullptr;
	std::vector<Texture*> m_textures;
	Anim m_anim;
	Roi m_roi;
};

struct WorldRef {
	bool Read(char** mem);

	const char* m_worldName = nullptr;
	std::vector<PartRef*> m_partrefs;
	std::vector<ModelRef*> m_modelrefs;
};

struct World {
	bool Read(char** mem, char* memstart);

	const char* m_worldName = nullptr;
	std::vector<Part*> m_parts;
	std::vector<Model*> m_models;
};

struct WorldDB {
	static WorldDB Read(const char* path);

	World shared;
	std::vector<WorldRef> refs;
	std::vector<World> worlds;
};

static std::unordered_map<const char*, std::vector<Lod*>> g_lod_map;

void handle_world(World& world, const std::string dest);
