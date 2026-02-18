//
// Created by Faith Kamaraju on 2026-01-25.
//

#pragma once
#include <glm/glm.hpp>
#include <glm/detail/type_quat.hpp>
#include "Resource/MeshAsset.h"

constexpr int MAX_NODE_LEVEL = 16;

namespace LE {

    struct Hierarchy {
        int parent = -1;
        int firstChild = -1;
        int nextSibling = -1;
        int lastSibling = -1;
        int level = 0;

    };

    struct SceneNode {
        MeshAssetHandle meshHandle{};
        glm::mat4 localTransform{1.f};
        glm::mat4 globalTransform{1.f};
        Hierarchy hierarchy{};
        std::string debug_name;
    };

}
