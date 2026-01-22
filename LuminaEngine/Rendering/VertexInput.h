//
// Created by Faith Kamaraju on 2026-01-21.
//

#pragma once

#include <glm/glm.hpp>

namespace LE {

    struct alignas(16) Vertex {
        glm::vec3 pos;
        float pad1;
        glm::vec3 color;
        float pad2;
        glm::vec2 texCoord;
        float pad[2];
    };
}