#include "core.h"
#include "interleaf.h"
#include "object.h"
#include "sitypes.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <stdio.h>
#include <string>

static std::string destination;

namespace fs = std::filesystem;
using namespace si;

enum class Type : unsigned char {
	Null = 0,
	Bitmap = 1,
	Wave = 2,
	Smacker = 3,
	Object = 4,
	Flic = 5,
	Presenter = 6,
	Animation = 7,
	World = 8,
	Event = 9,
};

struct Node {
	Type type;
	unsigned short index;
	const char* name;
	const char* path;
	const char* presenter;
	unsigned short start_time;
	unsigned short duration;
	unsigned char loops;
	unsigned char flags;
	float location[3];
	float up[3];
	float direction[3];
	std::vector<unsigned char> extra;
	std::vector<unsigned short> children;
};

struct Index {
	std::vector<Node> nodes;
	std::unordered_map<const char*, Node*> name_to_node;
	std::unordered_map<const char*, Node*> path_to_node;
};

const char* to_owned(const std::string& str) {
	char* result = (char*) malloc(str.length() + 1);
	memcpy(result, str.c_str(), str.length());
	result[str.length()] = 0;
	return result;
}

void handle_node(Core* core_parent, Node* parent, Index* tree) {
	if (core_parent == nullptr) {
		return;
	}

	for (auto child : core_parent->GetChildren()) {
		auto object = (Object*) child;

		Node node;
		node.index = object->id();
		node.path = to_owned(object->filename());
		node.name = to_owned(object->name());
		node.presenter = to_owned(object->presenter_);
		for (auto byte : object->extra_) {
			node.extra.push_back(byte);
		}

		node.duration = object->duration_;
		node.loops = object->loops_;
		node.start_time = object->time_offset_;
		node.flags = object->flags_;
		node.location[0] = object->location_.x;
		node.location[1] = object->location_.y;
		node.location[2] = object->location_.z;
		node.direction[0] = object->direction_.x;
		node.direction[1] = object->direction_.y;
		node.direction[2] = object->direction_.z;
		node.up[0] = object->up_.x;
		node.up[1] = object->up_.y;
		node.up[2] = object->up_.z;

		const char* extension = "";
		switch (object->type()) {
		case MxOb::Null:
			continue;
		case MxOb::Video:
		case MxOb::Sound:
		case MxOb::Bitmap:
		case MxOb::Object:
			switch (object->filetype()) {
			case MxOb::WAV:
				node.type = Type::Wave;
				extension = ".wav";
				break;
			case MxOb::STL:
				node.type = Type::Bitmap;
				extension = ".bmp";
				break;
			case MxOb::FLC:
				node.type = Type::Flic;
				// Stolen from the qt viewer.
				// Used to flip the flc upside down.
				if (object->name().find("_Pho_") != std::string::npos) {
					extension = ".pho";
				} else {
					extension = ".flc";
				}
				break;
			case MxOb::SMK:
				node.type = Type::Smacker;
				extension = ".smk";
				break;
			case MxOb::OBJ:
				node.type = Type::Object;
				extension = ".obj";
				break;
			};
			break;

		case MxOb::Animation:
			node.type = Type::Animation;
			extension = ".anim";
			break;

		case MxOb::World:
			node.type = Type::World;
			break;
		case MxOb::Presenter:
			node.type = Type::Presenter;
			break;
		case MxOb::Event:
			node.type = Type::Event;
			break;

		case MxOb::TYPE_COUNT:
			break;
		}

		printf("\rHandling index %hu", node.index);
		fflush(stdout);

		auto object_path = destination + std::to_string(node.index) + extension;
		if (object->HasChildren()) {
			handle_node(object, &node, tree);
		} else {
			bool is_empty = true;
			for (auto chunk : object->data()) {
				if (chunk.size() != 0) {
					is_empty = false;
					break;
				}
			}
			if (!is_empty) {
				object->ExtractToFile(object_path.c_str());
			}
		}

		if (parent != nullptr) {
			parent->children.push_back(node.index);
		}

		tree->nodes.push_back(node);
	}
}

void handle(Interleaf& weaver, Index* tree, const char* path) {
	printf("Reading %s\n", path);
	if (weaver.Read(path) != Interleaf::ERROR_SUCCESS) {
		printf("Failed to read %s", path);
	}
	handle_node((Core*) &weaver, nullptr, tree);
	printf("\n");
}

// fwrite2 is an fwrite wrapper that swaps endiannes before writing.
// N64 uses big endian while x86_64 uses little endian.
#define BYTE(x) ((uint8_t) ((x) & 0xFF))
template <typename T>
void fwrite2(T* ptr, size_t size, size_t count, FILE* file) {
	uint8_t* data = (uint8_t*) ptr;
	if (size == 2) {
		uint16_t d = *(uint16_t*) ptr;
		data[0] = BYTE(d >> 8);
		data[1] = BYTE(d >> 0);
	} else if (size == 4) {
		uint32_t d = *(uint32_t*) ptr;
		data[0] = BYTE(d >> 24);
		data[1] = BYTE(d >> 16);
		data[2] = BYTE(d >> 8);
		data[3] = BYTE(d >> 0);
	} else if (size == 8) {
		uint64_t d = *(uint64_t*) ptr;
		data[0] = BYTE(d >> 56);
		data[1] = BYTE(d >> 48);
		data[2] = BYTE(d >> 40);
		data[3] = BYTE(d >> 32);
		data[4] = BYTE(d >> 24);
		data[5] = BYTE(d >> 16);
		data[6] = BYTE(d >> 8);
		data[7] = BYTE(d >> 0);
	}
	fwrite(data, size, count, file);
}

void write_node(const Node& node, FILE* file) {
	fwrite2(&node.type, sizeof(node.type), 1, file);
	fwrite2(&node.index, sizeof(node.index), 1, file);

	unsigned short size = strlen(node.name);
	fwrite2(&size, sizeof(size), 1, file);
	fwrite2(node.name, sizeof(char), strlen(node.name), file);

	size = strlen(node.path);
	fwrite2(&size, sizeof(size), 1, file);
	fwrite2(node.path, sizeof(char), strlen(node.path), file);

	size = strlen(node.presenter);
	fwrite2(&size, sizeof(size), 1, file);
	fwrite2(node.presenter, sizeof(char), strlen(node.presenter), file);

	fwrite2(&node.start_time, sizeof(node.start_time), 1, file);
	fwrite2(&node.duration, sizeof(node.duration), 1, file);
	fwrite2(&node.loops, sizeof(node.loops), 1, file);
	fwrite2(&node.flags, sizeof(node.flags), 1, file);
	fwrite2(&node.location[0], sizeof(node.location[0]), 1, file);
	fwrite2(&node.location[1], sizeof(node.location[1]), 1, file);
	fwrite2(&node.location[2], sizeof(node.location[2]), 1, file);
	fwrite2(&node.direction[0], sizeof(node.direction[0]), 1, file);
	fwrite2(&node.direction[1], sizeof(node.direction[1]), 1, file);
	fwrite2(&node.direction[2], sizeof(node.direction[2]), 1, file);
	fwrite2(&node.up[0], sizeof(node.up[0]), 1, file);
	fwrite2(&node.up[1], sizeof(node.up[1]), 1, file);
	fwrite2(&node.up[2], sizeof(node.up[2]), 1, file);

	size = node.extra.size();
	fwrite2(&size, sizeof(size), 1, file);
	for (auto byte : node.extra) {
		fwrite2(&byte, sizeof(byte), 1, file);
	}

	size = node.children.size();
	fwrite2(&size, sizeof(size), 1, file);
	for (auto child : node.children) {
		fwrite2(&child, sizeof(child), 1, file);
	}
}

void fill_holes(Index& tree) {
	int max_index = 0;
	for (unsigned int i = 0; i < tree.nodes.size(); i++) {
		if (tree.nodes[i].index > max_index) {
			max_index = tree.nodes[i].index;
		}
	}

	auto holes = std::vector<unsigned int>();
	for (int i = 0; i < max_index; i++) {
		bool found = false;
		for (unsigned int j = 0; j < tree.nodes.size(); j++) {
			if (tree.nodes[j].index == i) {
				found = true;
				break;
			}
		}
		if (!found) {
			holes.push_back(i);
		}
	}

	for (auto index : holes) {
		Node node;
		node.type = Type::Null;
		node.index = index;
		node.name = "";
		node.path = "";
		node.presenter = "";

		tree.nodes.push_back(node);
		auto object_path = destination + std::to_string(node.index);
	}
}

void write_index(Index& tree) {
	FILE* file = fopen((destination + "index").c_str(), "wb");
	if (file == nullptr) {
		fprintf(stderr, "Failed to open index file for writing\n");
		return;
	}

	unsigned short size = tree.nodes.size();
	fwrite2(&size, sizeof(size), 1, file);
	for (auto node : tree.nodes) {
		write_node(node, file);
	}

	fclose(file);
}

int main(int argc, char* argv[]) {
	if (argc != 3) {
		printf("Usage: sidump <destination> <interleaf file>\n");
		return 0;
	}

	const char* path = argv[2];
	if (!fs::exists(path)) {
		fprintf(stderr, "Path does not exist\n");
		return -1;
	}

	destination = std::string(argv[1]);
	if (destination.back() != '/') {
		destination += '/';
	}
	fs::create_directory(destination);
	if (!fs::is_directory(destination)) {
		fprintf(stderr, "Destination is not a directory\n");
		return -1;
	}

	Index tree;
	Interleaf weaver;
	if (!fs::is_directory(path)) {
		handle(weaver, &tree, path);
	} else {
		fprintf(stderr, "Path is a directory\n");
		return -1;
	}

	// Ensure any skipped indeces are filled with empty nodes and files
	fill_holes(tree);

	std::sort(
		tree.nodes.begin(),
		tree.nodes.end(),
		[](const Node& a, const Node& b) { return a.index < b.index; }
	);
	write_index(tree);

	return 0;
}
