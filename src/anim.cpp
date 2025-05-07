#include "anim.h"
#include "utils.h"


bool AnimKey::Read(char** mem) {
	int timeAndFlags = memread(timeAndFlags, mem);
	m_flags = (unsigned int) timeAndFlags >> 24;
	m_time = timeAndFlags & 0xffffff;
	return true;
}

bool TranslationKey::Read(char** mem) {
	AnimKey::Read(mem);
    m_x = memread(m_x, mem);
    m_y = memread(m_y, mem);
    m_z = memread(m_z, mem);
	if (m_x > 1e-05F || m_x < -1e-05F || m_y > 1e-05F || m_y < -1e-05F ||
		m_z > 1e-05F || m_z < -1e-05F) {
		m_flags |= c_bit1;
	}
	return true;
}
bool RotationKey::Read(char** mem) {
	AnimKey::Read(mem);
    m_angle = memread(m_angle, mem);
    m_x = memread(m_x, mem);
    m_y = memread(m_y, mem);
    m_z = memread(m_z, mem);
	if (m_angle != 1.0F) {
		m_flags |= c_bit1;
	}
	return true;
}
bool ScaleKey::Read(char** mem) {
	AnimKey::Read(mem);
    m_x = memread(m_x, mem);
    m_y = memread(m_y, mem);
    m_z = memread(m_z, mem);
	if (m_x > 1.00001 || m_x < 0.99999 || m_y > 1.00001 || m_y < 0.99999 ||
		m_z > 1.00001 || m_z < 0.99999) {
		m_flags |= c_bit1;
	}
	return true;
}
bool MorphKey::Read(char** mem) {
	AnimKey::Read(mem);
    m_unk0x08 = memread(m_unk0x08, mem);
	return true;
}
bool DepthKey::Read(char** mem) {
	AnimKey::Read(mem);
    m_z = memread(m_z, mem);
	return true;
}

bool AnimNode::Read(char** mem) {
	unsigned int namelen = memread(namelen, mem);
    m_name = new char[namelen + 1];
    memread(m_name, namelen, mem);
    m_name[namelen] = '\0';

    m_numTranslationKeys = memread(m_numTranslationKeys, mem);
	if (m_numTranslationKeys) {
		for (int i = 0; i < m_numTranslationKeys; i++) {
			TranslationKey key;
            key.Read(mem);
            m_translationKeys.push_back(key);
		}
	}
    
    m_numRotationKeys = memread(m_numRotationKeys, mem);
	if (m_numRotationKeys) {
		for (int i = 0; i < m_numRotationKeys; i++) {
			RotationKey key;
            key.Read(mem);
            m_rotationKeys.push_back(key);
		}
	}
    
    m_numScaleKeys = memread(m_numScaleKeys, mem);
	if (m_numScaleKeys) {
		for (int i = 0; i < m_numScaleKeys; i++) {
			ScaleKey key;
            key.Read(mem);
            m_scaleKeys.push_back(key);
		}
	}

    m_numMorphKeys = memread(m_numMorphKeys, mem);
	if (m_numMorphKeys) {
		for (int i = 0; i < m_numMorphKeys; i++) {
			MorphKey key;
            key.Read(mem);
            m_morphKeys.push_back(key);
		}
	}

	return true;
}

bool AnimScene::Read(char** mem) {
    m_numTranslationKeys = memread(m_numTranslationKeys, mem);
    for (int i = 0; i < m_numTranslationKeys; i++) {
        TranslationKey key;
        key.Read(mem);
        m_translationKeys.push_back(key);
    }

    m_numTranslationKeys2 = memread(m_numTranslationKeys2, mem);
    for (int i = 0; i < m_numTranslationKeys2; i++) {
        TranslationKey key;
        key.Read(mem);
        m_translationKeys2.push_back(key);
    }

    m_numdepthKeys = memread(m_numdepthKeys, mem);
    for (int i = 0; i < m_numdepthKeys; i++) {
        DepthKey key;
        key.Read(mem);
        m_depthKeys.push_back(key);
    }

	return true;
}

bool AnimActorRef::Read(char** mem) {
    unsigned int length = memread(length, mem);
    m_name = new char[length + 1];
    memread(m_name, length, mem);
    m_name[length] = '\0';
    m_unk0x04 = memread(m_unk0x04, mem);
    return true;
}

bool Anim::Read(char** mem, bool p_parseScene) {
    m_numActors = memread(m_numActors, mem);
    for (int i = 0; i < m_numActors; i++) {
        AnimActorRef* actor = new AnimActorRef();
        actor->Read(mem);
        m_actorList.push_back(actor);
	}
    
    m_duration = memread(m_duration, mem);

	if (p_parseScene) {
		m_camAnim = new AnimScene();
        m_camAnim->Read(mem);
	}

    m_root = new AnimNode();
    m_root->Read(mem);
    ReadTree(mem, m_root);

    return true;
}

bool Anim::ReadTree(char** mem, AnimNode*& p_node) {
    unsigned int numChildren = memread(numChildren, mem);
    for (int i = 0; i < numChildren; i++) {
        AnimNode* node = new AnimNode();
        node->Read(mem);
        ReadTree(mem, node);
        p_node->m_numChildren += 1;
        p_node->m_children.push_back(node);
    }
    return true;
}
