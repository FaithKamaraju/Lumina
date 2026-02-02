//
// Created by Faith Kamaraju on 2026-01-25.
//

#pragma once
#include <glm/glm.hpp>
#include "TextureAsset.h"

namespace LE {

    struct GLTFMetallicRoughnessMaterialAsset {

        uint32_t baseColorTextureIndex;
        uint32_t baseColorTextureTexCoord;
        std::optional<uint32_t> metallicRoughnessTextureIndex;
        std::optional<uint32_t> metallicRoughnessTextureTexCoord;
        glm::vec4 baseColorFactor = glm::vec4(1.0f);
        float metallicFactor = 1.0f;
        float roughnessFactor = 1.0f;
        // std::optional<TextureAsset> normalMap;
        // float occlusionStrength = 1.0f;



    };

}