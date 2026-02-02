//
// Created by Faith Kamaraju on 2026-01-21.
//

#pragma once

#include <glm/glm.hpp>
#include "Rendering/RenderingConstants.h"

namespace LE {

    struct Vertex {
        glm::vec3 position{0.f};
        float padding1_ = 0;
        glm::vec3 normal{0.f};
        float padding2_ = 0;
        glm::vec2 uv[MAX_NUM_TEX_COORDS]{{},{},{},{}};
        glm::vec4 tangent{0.f};
        glm::vec4 color{0.f};
    };
}