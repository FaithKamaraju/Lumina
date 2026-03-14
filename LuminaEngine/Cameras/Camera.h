//
// Created by Faith Kamaraju on 2026-02-15.
//

#pragma once
#include <glm/glm.hpp>

namespace LE {

    struct Camera {
        glm::mat4x4 view{1.f};
        glm::mat4x4 projection{1.f};

    };

}
