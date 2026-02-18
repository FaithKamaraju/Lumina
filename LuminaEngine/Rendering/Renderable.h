//
// Created by Faith Kamaraju on 2026-01-22.
//

#pragma once

namespace LE {

    struct Renderable {
        glm::mat4 modelMatrix{1.f};
        PipelineHandle pipelineHandle{};
        BufferHandle vertexBufferHandle{};
        BufferHandle indexBufferHandle{};
        uint32_t indexOffset{};
        uint32_t indexCount{};
        uint32_t materialIndex{};
    };
}