#include "core.h"
#include "interleaf.h"
#include "object.h"
#include "sitypes.h"

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <stdio.h>
#include <string>

static std::string destination;

namespace fs = std::filesystem;
using namespace si;

enum class Type : unsigned char {
  Bitmap = 0,
  Wave = 1,
  Smacker = 2,
  Object = 3,
  Flic = 4,
  Presenter = 5,
  Animation = 6,
  World = 7,
  Event = 8,
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

const char *to_owned(const std::string &str) {
  char *result = (char *)malloc(str.length() + 1);
  memcpy(result, str.c_str(), str.length());
  result[str.length()] = 0;
  return result;
}

void handle_node(Core *core_parent, Node *parent, Index *tree) {
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
        break;
      case MxOb::STL:
        node.type = Type::Bitmap;
        break;
      case MxOb::FLC:
        node.type = Type::Flic;
        break;
      case MxOb::SMK:
        node.type = Type::Smacker;
        break;
      case MxOb::OBJ:
        node.type = Type::Object;
        break;
      };
      break;

    case MxOb::Animation:
      node.type = Type::Animation;
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

    if (object->HasChildren()) {
      handle_node(object, &node, tree);
    } else {
      bool is_empty = true;
      for (auto &chunk : object->data_) {
        if (chunk.size() > 0) {
          is_empty = false;
          break;
        }
      }
      if (!is_empty) {
        auto object_path = destination + std::to_string(node.index);
        object->ExtractToFile(object_path.c_str());
      }
    }

    if (parent != nullptr) {
      parent->children.push_back(node.index);
    }

    tree->nodes.push_back(node);
  }
}

void handle(Interleaf &weaver, Index *tree, const char *path) {
  printf("Reading %s\n", path);
  if (weaver.Read(path) != Interleaf::ERROR_SUCCESS) {
    printf("Failed to read %s", path);
  }
  handle_node((Core *)&weaver, nullptr, tree);
  printf("\n");
}

void write_node(const Node &node, FILE *file) {
  fwrite(&node.type, sizeof(node.type), 1, file);
  fwrite(&node.index, sizeof(node.index), 1, file);

  unsigned short size = strlen(node.name);
  fwrite(&size, sizeof(size), 1, file);
  fwrite(node.name, sizeof(char), strlen(node.name), file);

  size = strlen(node.path);
  fwrite(&size, sizeof(size), 1, file);
  fwrite(node.path, sizeof(char), strlen(node.path), file);

  size = strlen(node.presenter);
  fwrite(&size, sizeof(size), 1, file);
  fwrite(node.presenter, sizeof(char), strlen(node.presenter), file);

  fwrite(&node.start_time, sizeof(node.start_time), 1, file);
  fwrite(&node.duration, sizeof(node.duration), 1, file);
  fwrite(&node.loops, sizeof(node.loops), 1, file);
  fwrite(&node.flags, sizeof(node.flags), 1, file);
  fwrite(&node.location[0], sizeof(node.location[0]), 1, file);
  fwrite(&node.location[1], sizeof(node.location[1]), 1, file);
  fwrite(&node.location[2], sizeof(node.location[2]), 1, file);
  fwrite(&node.direction[0], sizeof(node.direction[0]), 1, file);
  fwrite(&node.direction[1], sizeof(node.direction[1]), 1, file);
  fwrite(&node.direction[2], sizeof(node.direction[2]), 1, file);
  fwrite(&node.up[0], sizeof(node.up[0]), 1, file);
  fwrite(&node.up[1], sizeof(node.up[1]), 1, file);
  fwrite(&node.up[2], sizeof(node.up[2]), 1, file);

  size = node.extra.size();
  fwrite(&size, sizeof(size), 1, file);
  for (auto byte : node.extra) {
    fwrite(&byte, sizeof(byte), 1, file);
  }

  size = node.children.size();
  fwrite(&size, sizeof(size), 1, file);
  for (auto child : node.children) {
    fwrite(&child, sizeof(child), 1, file);
  }
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Usage: sidump <destination> <assets>\n");
    return 0;
  }

  destination = std::string(argv[1]);
  if (destination.back() != '/') {
    destination += '/';
  }

  const char *path = argv[2];
  if (!fs::exists(path)) {
    fprintf(stderr, "Path does not exist\n");
    return -1;
  }

  fs::create_directory(destination);
  auto index_file = fopen((destination + "index").c_str(), "w");
  if (index_file == nullptr) {
    fprintf(stderr, "Failed to create index file\n");
    return -1;
  }

  Index tree;
  Interleaf weaver;
  if (fs::is_directory(path)) {
    return 0;
    // TODO
    auto walker = fs::recursive_directory_iterator(path);
    for (const auto &file : walker) {
      if (!file.is_regular_file()) {
        continue;
      }
      handle(weaver, &tree, file.path().c_str());
    }
  } else {
    handle(weaver, &tree, path);
  }

  std::sort(tree.nodes.begin(), tree.nodes.end(),
            [](const Node &a, const Node &b) { return a.index < b.index; });

  unsigned short size = tree.nodes.size();
  fwrite(&size, sizeof(size), 1, index_file);
  for (auto node : tree.nodes) {
    write_node(node, index_file);
  }

  return 0;
}
