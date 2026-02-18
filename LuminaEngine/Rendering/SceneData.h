//
// Created by Faith Kamaraju on 2026-02-10.
//

#pragma once
#include <glm/glm.hpp>

namespace LE {

    struct CameraAndSceneData {
        glm::mat4 view{1.f};
        glm::mat4 proj{1.f};
        // glm::mat4 viewProj;
        // glm::vec3 cameraPosition;
        float time;
    };
}