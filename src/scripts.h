#pragma once
#include "core.h"


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
	Path = 10,
	Texture = 11,
	Model = 11,
};

struct Node {
	Type type;
	unsigned short index;
	const char *name;
	const char *path;
	const char *presenter;
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
	std::unordered_map<const char *, Node *> name_to_node;
	std::unordered_map<const char *, Node *> path_to_node;
};

void handle_node(si::Core *core_parent, Node *parent, Index *tree, const std::string &dest);
void fill_holes(Index &tree, const std::string &dest);
void write_index(Index &tree, const std::string &dest);
