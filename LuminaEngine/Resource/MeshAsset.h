//
// Created by Faith Kamaraju on 2026-01-25.
//

#pragma once
#include "Rendering/Buffer.h"
#include "Resource/MaterialAsset.h"
#include "Resource/TextureAsset.h"

namespace LE {

    struct SubMesh {
        uint32_t vertexOffset;
        uint32_t indexOffset;
        uint32_t indexCount;
        uint32_t materialIndex;
    };

    struct MeshAsset {
        uint32_t vertexCount{};     // unique vertices
        uint32_t indexCount{};      // indices for DrawIndexed
        std::vector<SubMesh> subMeshes;
        std::vector<uint32_t> materials;
    };

}