//
// Created by Faith Kamaraju on 2026-01-21.
//

#pragma once

#include <glm/glm.hpp>
#include "Rendering/RenderingConstants.h"

namespace LE {

    struct Vertex {
        Vertex() {uv.fill(glm::vec2{0.0f, 0.0f});}
        glm::vec3 position{0.f};
        float padding1_ = 0;
        glm::vec3 normal{0.f};
        float padding2_ = 0;
        std::array<glm::vec2, MAX_NUM_TEX_COORDS> uv{};
        glm::vec4 tangent{0.f};
        glm::vec4 color{0.f};
    };
}