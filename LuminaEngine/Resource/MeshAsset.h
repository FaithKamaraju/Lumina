//
// Created by Faith Kamaraju on 2026-01-25.
//

#pragma once
#include "MaterialAsset.h"
#include "Rendering/Buffer.h"

namespace LE {



    struct MeshAssetHandle {
        int32_t id = -1;
        uint32_t generation{};
    };

    struct SubMesh {
        Topology topology = Topology::Triangles;
        uint32_t vertexOffset{};
        uint32_t indexOffset{};
        uint32_t indexCount{};
        PBR_MR_MaterialInstanceHandle materialHandle{};
    };

    struct MeshAsset {
        BufferHandle vertexBufferHandle{};
        BufferHandle indicesBufferHandle{};
        uint32_t vertexCount{};     // unique vertices
        uint32_t indexCount{};      // indices for DrawIndexed
        std::vector<SubMesh> subMeshes;
    };
}