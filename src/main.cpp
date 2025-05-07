#include <cctype>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <stdio.h>
#include <string>

#include "core.h"
#include "interleaf.h"
#include "scripts.h"
#include "worlds.h"
#include "dump_assets.h"

namespace fs = std::filesystem;
using namespace si;

void handle_scripts(Interleaf &weaver, const char *path, const std::string &dest) {
	Index tree;
	printf("Reading %s\n", path);
	if (weaver.Read(path) != Interleaf::ERROR_SUCCESS) {
		printf("Failed to read %s", path);
	}
	handle_node((Core *)&weaver, nullptr, &tree, dest);
	printf("\n");

	// Ensure any skipped indeces are filled with empty nodes and files
	fill_holes(tree, dest);

	std::sort(
		tree.nodes.begin(),
		tree.nodes.end(),
		[](const Node &a, const Node &b) { return a.index < b.index; });
	write_index(tree, dest);
}

void handle_worlds(const char *path, const std::string &dest) {
	auto db = WorldDB::Read(path);

	fs::create_directory(dest);

	db.worlds.push_back(db.shared);
	for (auto& world : db.worlds) {
		printf("World: %s\n", world.m_worldName);

		std::string world_path = dest + world.m_worldName + "/";
		fs::create_directory(world_path);

		std::string model_root = world_path + "models/";
		fs::create_directory(model_root);
		for (auto model : world.m_models) {
			std::string model_path = model_root + model->ref->m_modelName + "/";
			fs::create_directory(model_path);
			
			printf("  Model: %s\n", model->ref->m_modelName);
			for (auto texture : model->m_textures) {
				printf("    Texture: %s\n", texture->m_name);
				std::string texture_path = model_path + texture->m_name;
				dump_texture(*texture, texture_path.c_str());
			}

			for (auto comp : model->m_roi.m_components) {
				printf("    Component: %s\n", comp->m_roiname);
				int i_lod = 0;
				for (auto lod: comp->m_lods) {
					printf("      Lod %d: %s\n", i_lod, comp->m_roiname);
					std::string lod_path = model_path + std::string(comp->m_roiname) + "_" + std::to_string(i_lod) + ".obj";
					dump_lod(*lod, lod_path.c_str());
					i_lod += 1;
				}
			}
		}

		std::string part_root = world_path + "parts/";
		fs::create_directory(part_root);
		for (auto part : world.m_parts) {
			std::string part_path = part_root + part->ref->m_roiname + "/";
			fs::create_directory(part_path);

			printf("  Part: %s\n", part->ref->m_roiname);
			for (auto texture : part->m_textures) {
				printf("    Texture: %s\n", texture->m_name);
				std::string texture_path = part_path + texture->m_name;
				dump_texture(*texture, texture_path.c_str());
			}
			for (auto data : part->m_data) {
				int i_lod = 0;
				for (auto lod : data->m_lods) {
					printf("    Lod: %s %d\n", data->m_roiname, i_lod);
					std::string lod_path = part_path + data->m_roiname + "_" + std::to_string(i_lod) + ".obj";
					dump_lod(*lod, lod_path.c_str());
					i_lod += 1;
				}
			}
		}
	}
}

int main(int argc, char *argv[]) {
	if (argc != 4) {
		printf("Usage: sidump [scripts|worlds] <destination> <interleaf file>\n");
		return 0;
	}

	bool is_scripts = strcmp(argv[1], "scripts") == 0;
	bool is_worlds = strcmp(argv[1], "worlds") == 0;
	if (!is_scripts && !is_worlds) {
		fprintf(stderr, "Invalid type. Use 'scripts' or 'worlds'\n");
		return -1;
	}

	std::string destination = std::string(argv[2]);
	if (destination.back() != '/') {
		destination += '/';
	}
	fs::create_directory(destination);
	if (!fs::is_directory(destination)) {
		fprintf(stderr, "Destination is not a directory\n");
		return -1;
	}

	const char *path = argv[3];
	if (!fs::exists(path)) {
		fprintf(stderr, "Path does not exist\n");
		return -1;
	}

	Interleaf weaver;
	if (fs::is_directory(path)) {
		if (is_worlds) {
			fprintf(stderr, "In 'worlds' mode, the path must be a file\n");
			return -1;
		}
		for (const auto& file : fs::recursive_directory_iterator(path)) {
			if (fs::is_regular_file(file)) {
				fs::path rel_path = fs::relative(file.path(), path);
				fs::path dest_path = destination / rel_path;
				fs::create_directories(dest_path);

				std::string dest_str = dest_path.string() + "/";
				handle_scripts(weaver, file.path().c_str(), dest_str);
			}
		}
	}
	else {
		if (is_scripts) {
			handle_scripts(weaver, path, destination);
		} else if (is_worlds) {
			handle_worlds(path, destination);
		}
	}

	return 0;
}
