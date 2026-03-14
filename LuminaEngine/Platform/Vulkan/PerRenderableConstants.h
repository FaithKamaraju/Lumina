//
// Created by Faith Kamaraju on 2026-02-10.
//

#pragma once

namespace LE {
    struct PerRenderableConstants {
        glm::mat4 modelMatrix{1.f};
        uint64_t vertexBufferAddress{};
        uint32_t materialIndex {};
    };
}