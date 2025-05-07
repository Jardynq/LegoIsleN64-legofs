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
#include <filesystem>

#include "utils.h"
#include "worlds.h"

namespace fs = std::filesystem;
using namespace si;

void handle_node(Core *core_parent, Node *parent, Index *tree, const std::string &dest) {
	if (core_parent == nullptr) {
		return;
	}

	for (auto child : core_parent->GetChildren()) {
		auto object = (Object *)child;

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
			extension = ".bmp";
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
			    continue;
			default:
				extension = ".unknown";
				node.type = Type::Null;
			    break;
			}
		}

        /*
        if (node.extra.size() > 0) {
            for (auto byte : node.extra) {
                printf("%c", (char)byte);
            }
            printf("\n");
        }
        */

		auto object_path = dest + std::to_string(node.index) + extension;
		if (object->HasChildren()) {
			handle_node(object, &node, tree, dest);
		}
		else {
			bool is_empty = true;
			for (auto chunk : object->data()) {
				if (chunk.size() != 0) {
					is_empty = false;
					break;
				}
			}
			if (!is_empty) {
				if (node.type == Type::Model) {
                    fs::create_directories(object_path);

                    auto buf = object->ExtractToMemory();
					auto f = fmemopen(buf.data(), buf.size(), "r");
					if (f == nullptr) {
						printf("Failed to open memory stream\n");
						return;
					}

                    char* mem = buf.data();
                    Model* model = new Model();
                    model->Read(&mem);
                    for (auto texture : model->m_textures) {
                        std::string texture_path = object_path + "/" + texture->m_name;
                        dump_texture(*texture, texture_path.c_str());
                    }
                    for (auto comp : model->m_roi.m_components) {
                        int i_lod = 0;
                        for (auto lod: comp->m_lods) {
                            std::string lod_path = object_path + "/" + std::string(comp->m_roiname) + "_" + std::to_string(i_lod) + ".obj";
                            dump_lod(*lod, lod_path.c_str());
                            i_lod += 1;
                        }
                    }
                } else if (node.type == Type::Texture) {
                    auto buf = object->ExtractToMemory();
					auto f = fmemopen(buf.data(), buf.size(), "r");
					if (f == nullptr) {
						printf("Failed to open memory stream\n");
						return;
					}

                    char* mem = buf.data();
                    Texture* tex = new Texture();
                    tex->Read(&mem);
                    dump_texture(*tex, object_path.c_str());
				} else {
					object->ExtractToFile(object_path.c_str());
				}
			}
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
