#pragma once
#include <vector>
#include "utils.h"

/*
    Despite being called anim, I believe this is just used for skeleton bones.
*/

struct AnimKey {
    bool Read(char** mem);
	void free() {}

	enum Flags { c_bit1 = 0x01, c_bit2 = 0x02, c_bit3 = 0x04 };
    unsigned char m_flags = 0;
	float m_time = 0.0f;
};
struct TranslationKey : AnimKey {
    bool Read(char** mem);

    float m_x = 0.0f;
    float m_y = 0.0f;
    float m_z = 0.0f;
};
struct RotationKey : AnimKey {
    bool Read(char** mem);

    float m_angle = 0.0f;
    float m_x = 0.0f;
    float m_y = 0.0f;
    float m_z = 0.0f;
};
struct ScaleKey : AnimKey {
    bool Read(char** mem);

    float m_x = 0.0f;
    float m_y = 0.0f;
    float m_z = 0.0f;
};
struct MorphKey : AnimKey {
    bool Read(char** mem);

    bool m_unk0x08 = 0;
};
struct DepthKey : AnimKey {
    bool Read(char** mem);

    float m_z = 0;
};

struct AnimNode {
    bool Read(char** mem);
	void free() {
		FREE_ARR(m_name);
		FREE_OBJ_VEC(m_translationKeys);
		FREE_OBJ_VEC(m_rotationKeys);
		FREE_OBJ_VEC(m_scaleKeys);
		FREE_OBJ_VEC(m_morphKeys);
		FREE_PTR_VEC(m_children);
	}

	char* m_name = nullptr;
	unsigned short m_numTranslationKeys = 0;
	unsigned short m_numRotationKeys = 0;
	unsigned short m_numScaleKeys = 0;
	unsigned short m_numMorphKeys = 0;
	std::vector<TranslationKey> m_translationKeys;
	std::vector<RotationKey> m_rotationKeys;
	std::vector<ScaleKey> m_scaleKeys;
	std::vector<MorphKey> m_morphKeys;
	unsigned short m_unk0x20 = 0;
	unsigned short m_unk0x22 = 0;
	unsigned int m_translationIndex = 0;
	unsigned int m_rotationIndex = 0;
	unsigned int m_scaleIndex = 0;
	unsigned int m_morphIndex = 0;

	unsigned int m_numChildren;
	std::vector<AnimNode*> m_children;
};

struct AnimScene {
    bool Read(char** mem);
	void free() {
		FREE_OBJ_VEC(m_translationKeys);
		FREE_OBJ_VEC(m_translationKeys2);
		FREE_OBJ_VEC(m_depthKeys);
	}

	unsigned short m_numTranslationKeys = 0;
	unsigned short m_numTranslationKeys2 = 0;
	unsigned short m_numdepthKeys = 0;
	std::vector<TranslationKey> m_translationKeys;
	std::vector<TranslationKey> m_translationKeys2;
	std::vector<DepthKey> m_depthKeys;
	unsigned int m_unk0x18 = 0;
	unsigned int m_unk0x1c = 0;
	unsigned int m_unk0x20 = 0;
};

struct AnimActorRef {
    bool Read(char** mem);
	void free() {
		FREE_ARR(m_name);
	}

	char* m_name = nullptr;
	unsigned int m_unk0x04 = 0;
};

struct Anim {
    bool Read(char** mem, bool p_parseScene);
    bool ReadTree(char** mem, AnimNode*& p_node);
	void free() {
		FREE_PTR(m_camAnim);
		FREE_PTR(m_root);
		FREE_PTR_VEC(m_actorList);
	}

    int m_duration = 0;
	unsigned int m_numActors = 0;
	std::vector<AnimActorRef*> m_actorList;
	AnimScene* m_camAnim = nullptr;
    AnimNode* m_root = nullptr;
};
