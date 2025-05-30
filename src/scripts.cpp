#include "scripts.h"

#include "core.h"
#include "dump_assets.h"
#include "object.h"
#include "sitypes.h"

#include <cctype>
#include <cstdio>
#include <cstring>
#include <stdio.h>
#include <string>

#include "types.h"
#include "utils.h"
#include "worlds.h"

using namespace si;

void dump_node(const Node &node, const char* extension, Object* object, const std::string &dest) {
    auto object_path = dest + std::to_string(node.index) + extension;

    bool is_empty = true;
    for (auto chunk : object->data()) {
        if (chunk.size() != 0) {
            is_empty = false;
            break;
        }
    }
    if (is_empty) {
        return;
    }

    if (node.type == Type::Model) {
        auto buf = object->ExtractToMemory();
        char* mem = buf.data();
        Model* model = new Model();
        model->Read(&mem);
        for (auto texture : model->m_textures) {
            std::string texture_path = dest + texture->m_name;
            replace_end(texture_path.c_str(), ".bmp", ".png");
            dump_texture(*texture, texture_path.c_str());
        }
        dump_components(model, dest, nullptr);
        model->free();
        delete model;
        buf.clear();
    } else {
        object->ExtractToFile(object_path.c_str());
    }
}

const char* parse_node(Object* object, Node& node) {
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

    const char *extension = nullptr;
    switch (pack_extension(object->filename())) {
    case packconst(".wav"):
        extension = ".wav";
        node.type = Type::Wave;
        break;
    case packconst(".smk"):
        extension = ".smk";
        node.type = Type::Smacker;
        break;
    case packconst(".flc"):
        // Stolen from the qt viewer.
        // Used to flip the flc upside down.
        if (object->name().find("_Pho_") != std::string::npos) {
            extension = ".pho";
        }
        else {
            extension = ".flc";
        }
        node.type = Type::Flic;
        break;
    case packconst(".bmp"):
        extension = ".bmp";
        node.type = Type::Bitmap;
        break;
    case packconst(".obj"):
        extension = ".obj";
        node.type = Type::Object;
        break;
    case packconst(".ani"):
        extension = ".ani";
        node.type = Type::Animation;
        break;
    case packconst(".gph"):
        extension = ".path";
        node.type = Type::Path;
        break;
    case packconst(".tex"):
        extension = ".tex";
        node.type = Type::Texture;
        break;
    case packconst(".evt"):
        extension = ".event";
        node.type = Type::Event;
        break;
    case packconst(".mod"):
        extension = ".model";
        node.type = Type::Model;
        break;
    default:
        switch (object->type()) {
        case MxOb::World:
            extension = ".world";
            node.type = Type::World;
            break;
        case MxOb::Presenter:
            extension = ".pres";
            node.type = Type::Presenter;
            break;
        case MxOb::Event:
            extension = ".event";
            node.type = Type::Event;
            break;
        case MxOb::Object:
            extension = ".obj";
            node.type = Type::Object;
            break;
        case MxOb::Null:
            return nullptr;
        default:
            extension = ".unknown";
            node.type = Type::Null;
            break;
        }
    }
    return extension;
}

void handle_node(Core *core_parent, Node *parent, Index *tree, const std::string &dest) {
	if (core_parent == nullptr) {
		return;
	}

	for (auto child : core_parent->GetChildren()) {
		auto object = (Object *)child;
        if (object == nullptr) {
            continue;
        }

		Node node;
		const char* extension = parse_node(object, node);
        if (extension == nullptr) {
            continue;
        }

		if (object->HasChildren()) {
			handle_node(object, &node, tree, dest);
		} else {
            dump_node(node, extension, object, dest);
		}

		if (parent != nullptr) {
			parent->children.push_back(node.index);
		}

		tree->nodes.push_back(node);
	}
}



void write_node(const Node &node, FILE *file) {
	fwrite2(&node.type, sizeof(node.type), 1, file);
	fwrite2(&node.index, sizeof(node.index), 1, file);
    if (node.type == Type::Null) {
        return;
    }

	unsigned short size = strlen(node.name);
	fwrite2(&size, sizeof(size), 1, file);
	fwrite2(node.name, sizeof(char), size, file);

	size = strlen(node.path);
	fwrite2(&size, sizeof(size), 1, file);
	fwrite2(node.path, sizeof(char), size, file);

	size = strlen(node.presenter);
	fwrite2(&size, sizeof(size), 1, file);
	fwrite2(node.presenter, sizeof(char), size, file);

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

void fill_holes(Index &tree, const std::string &dest) {
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
		auto object_path = dest + std::to_string(node.index);
	}
}

void write_index(Index &tree, const std::string &dest) {
	FILE *file = fopen((dest + "index").c_str(), "wb");
	if (file == nullptr) {
		fprintf(stderr, "Failed to open index file for writing: %s\n", (dest + "index").c_str());
		return;
	}

	unsigned short size = tree.nodes.size();
	fwrite2(&size, sizeof(size), 1, file);
	for (auto node : tree.nodes) {
        write_node(node, file);
	}

	fclose(file);
}
