//
// Created by Faith Kamaraju on 2026-02-10.
//

#pragma once
#include "Resource/MaterialAsset.h"

namespace LE {

    struct PBRMaterialGPUData {
        float baseColorFactor[4] = {1.f, 1.f, 1.f, 1.f};
        float metallicFactor = 0.5f;
        float roughnessFactor = 0.5f;
        uint32_t baseColorImageIndex = 0;
        uint32_t baseColorImageSamplerIndex = 0;
        uint32_t baseColorTextureTexCoord = 0;
        uint32_t metallicRoughnessImageIndex = 0;
        uint32_t metallicRoughnessImageSamplerIndex = 0;
        uint32_t metallicRoughnessTextureTexCoord = 0;
        uint32_t normalMapImageIndex = 0;
        uint32_t normalMapImageSamplerIndex = 0;
        uint32_t normalMapTextureTexCoord = 0;
    };
}