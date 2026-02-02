//
// Created by Faith Kamaraju on 2026-01-27.
//

#include "Rendering/Image.h"
#include "Rendering/Sampler.h"

#pragma once

namespace LE {

    struct SerializableImage {
        ImageFormat format;
        ImageExtent3D extent;
        std::vector<uint8_t> pixeldata;
        // std::vector<TextureMip> mips;
    };

    struct TextureAsset {
        ImageHandle imgHandle{};
        SamplerHandle samplerHandle;
    };
}