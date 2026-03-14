//
// Created by Faith Kamaraju on 2025-12-21.
//

#pragma once
#include <glm/glm.hpp>

namespace LE {

    struct TransformComponent{

        glm::mat4 localTransform{1.f};
        glm::mat4 globalTransform{1.f};
    };

}