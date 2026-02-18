//
// Created by Faith Kamaraju on 2026-01-25.
//

#pragma once
#include <glm/glm.hpp>
#include "Rendering/Pipeline.h"
#include "TextureAsset.h"

namespace LE {

    struct PBR_MR_MaterialInstanceHandle {
        int32_t id = -1;
        uint32_t generation{};
    };


    struct PBR_MR_MaterialInstance {

        TextureAssetHandle baseColorTextureAsset;
        uint32_t baseColorTextureTexCoord;
        glm::vec4 baseColorFactor = glm::vec4(1.0f);
        TextureAssetHandle metallicRoughnessTextureAsset;
        uint32_t metallicRoughnessTextureTexCoord;
        float metallicFactor = 0.5f;
        float roughnessFactor = 0.5f;
        TextureAssetHandle normalMapTextureAsset;
        uint32_t normalMapTextureTexCoord;

        CullMode cullMode = CullMode::Back;
        // float occlusionStrength = 1.0f;

    };

}