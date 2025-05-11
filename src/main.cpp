#include <cctype>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <stdio.h>
#include <string>
#include <thread>

#include "core.h"
#include "interleaf.h"
#include "scripts.h"
#include "worlds.h"

namespace fs = std::filesystem;
using namespace si;

void handle_scripts(std::string path, std::string dest) {
	Index tree;
	Interleaf weaver;
	if (weaver.Read(path.c_str()) != Interleaf::ERROR_SUCCESS) {
		printf("Failed to read %s\n", path.c_str());
	}
	handle_node((Core *)&weaver, nullptr, &tree, dest);
	fill_holes(tree, dest);
	std::sort(
		tree.nodes.begin(),
		tree.nodes.end(),
		[](const Node &a, const Node &b) { return a.index < b.index; });
	write_index(tree, dest);
}

void handle_worlds(const char *path, const std::string &dest, bool is_sync) {
	fs::create_directory(dest);
	
	auto db = WorldDB::Read(path);
	db.worlds.push_back(db.shared);
	
	std::vector<std::thread> threads;
	std::unordered_map<std::string, std::mutex*> mutexes;
	FILE* index_file = fopen((dest + "index_w").c_str(), "wb");
	for (auto& world : db.worlds) {
		write_world(world, index_file);

		for (auto& model : world.m_models) {
			for (auto& texture : model->m_textures) {
				mutexes.insert({texture->m_name, new std::mutex()});
			}
			for (auto& comp : model->m_roi.m_components) {
				mutexes.insert({comp->m_roiname, new std::mutex()});
			}
		}
		for (auto& part : world.m_parts) {
			for (auto& texture : part->m_textures) {
				mutexes.insert({texture->m_name, new std::mutex()});
			}
			for (auto& data : part->m_data) {
				mutexes.insert({data->m_roiname, new std::mutex()});
			}
		}
	}

	for (auto& world : db.worlds) {
		if (is_sync) {
			handle_world(world, dest, mutexes);
		} else {
			threads.push_back(std::thread(handle_world, std::ref(world), dest, std::ref(mutexes)));
		}
	}
	for (auto& thread : threads) {
		thread.join();
	}
}

int main(int argc, char *argv[]) {
	if (argc != 4 && argc != 5) {
		printf("Usage: sidump [scripts|worlds] <destination> <interleaf file> [sync]\n");
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

	bool is_sync = false;
	if (argc == 5) {
		if (strcmp(argv[4], "sync") == 0) {
			is_sync = true;
		} else {
			fprintf(stderr, "Invalid argument: %s\n", argv[4]);
			return -1;
		}
	}


	if (fs::is_directory(path)) {
		if (is_worlds) {
			fprintf(stderr, "In 'worlds' mode, the path must be a file\n");
			return -1;
		}
		auto thread_data = std::vector<std::tuple<std::string, std::string>>();
		for (const auto& file : fs::recursive_directory_iterator(path)) {
			if (fs::is_regular_file(file)) {
				fs::path rel_path = fs::relative(file.path(), path);
				fs::path dest_path = destination / rel_path;
				fs::create_directories(dest_path);
				
				std::string file_str = file.path().string();
				std::string dest_str = dest_path.string() + "/";
				if (is_sync) {
					handle_scripts(file_str, dest_str);
				} else {
					thread_data.push_back(std::tuple(file_str, dest_str));
				}
			}
		}
		
		auto threads = std::vector<std::thread>();
		for (auto& data : thread_data) {
			threads.push_back(std::thread(handle_scripts, std::get<0>(data), std::get<1>(data)));
		}
		for (auto& thread : threads) {
			thread.join();
		}
	}
	else {
		if (is_scripts) {
			handle_scripts(path, destination);
		} else if (is_worlds) {
			handle_worlds(path, destination, is_sync);
		}
	}

	return 0;
}
