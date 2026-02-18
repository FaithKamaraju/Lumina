//
// Created by Faith Kamaraju on 2026-01-27.
//

#include "Rendering/Image.h"
#include "Rendering/Sampler.h"

#pragma once

namespace LE {

    struct TextureAssetHandle {
        int32_t id = -1;
        uint32_t generation{};
    };

    struct TextureAsset {
        ImageHandle imgHandle{};
        SamplerHandle samplerHandle{};
    };
}