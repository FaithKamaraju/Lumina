//
// Created by Faith Kamaraju on 2026-01-25.
//

#pragma once
#include <glm/glm.hpp>
#include "ECS/Entity.h"
#include "Resource/MeshAsset.h"

constexpr int MAX_NODE_LEVEL = 16;

namespace LE {

    struct SceneNode {
        MeshAssetHandle meshHandle{};
        glm::mat4 localTransform{1.f};
        glm::mat4 globalTransform{1.f};
        int32_t parent = -1;
        int32_t firstChild = -1;
        int32_t nextSibling = -1;
        int32_t lastChild = -1;
        int32_t level = 0;
        std::string debug_name;
    };

    struct Hierarchy {
        Entity entityID;
        int32_t parent = -1;
        int32_t firstChild = -1;
        int32_t nextSibling = -1;
        int32_t lastChild = -1;
        int32_t level = 0;
        std::string debug_name;
    };

}
