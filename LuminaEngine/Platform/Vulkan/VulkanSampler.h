//
// Created by Faith Kamaraju on 2026-01-21.
//

#pragma once
#include <vulkan/vulkan.hpp>

#include "Rendering/Sampler.h"

namespace LE {

    inline vk::Filter ToVkFilter(const Filter filter) {

        switch (filter) {
            case Filter::Linear:    return vk::Filter::eLinear;
            case Filter::Nearest:   return vk::Filter::eNearest;
            case Filter::CubicIMG:  return vk::Filter::eCubicIMG;
            default:                return vk::Filter::eLinear;
        }
    }

    inline vk::SamplerMipmapMode ToVkSamplerMipMapMode(const SamplerMipmapMode mode) {
        switch (mode) {
            case SamplerMipmapMode::Linear:     return vk::SamplerMipmapMode::eLinear;
            case SamplerMipmapMode::Nearest:    return vk::SamplerMipmapMode::eNearest;
            default:                            return vk::SamplerMipmapMode::eLinear;
        }
    }

    inline vk::SamplerAddressMode ToVkSamplerAddressMode(const SamplerAddressMode mode) {
        switch (mode) {
            case SamplerAddressMode::Repeat:                    return vk::SamplerAddressMode::eRepeat;
            case SamplerAddressMode::MirroredRepeat:            return vk::SamplerAddressMode::eMirroredRepeat;
            case SamplerAddressMode::ClampToEdge:               return vk::SamplerAddressMode::eClampToEdge;
            case SamplerAddressMode::ClampToBorder:             return vk::SamplerAddressMode::eClampToBorder;
            case SamplerAddressMode::MirrorClampToEdge:         return vk::SamplerAddressMode::eMirrorClampToEdge;
            default:                                            return vk::SamplerAddressMode::eRepeat;
        }
    }

    inline vk::CompareOp ToVkCompareOp(const CompareOp op) {
        switch (op) {
            case CompareOp::Never:              return vk::CompareOp::eNever;
            case CompareOp::Less:               return vk::CompareOp::eLess;
            case CompareOp::Equal:              return vk::CompareOp::eEqual;
            case CompareOp::LessOrEqual:        return vk::CompareOp::eLessOrEqual;
            case CompareOp::Greater:            return vk::CompareOp::eGreater;
            case CompareOp::NotEqual:           return vk::CompareOp::eNotEqual;
            case CompareOp::GreaterOrEqual:     return vk::CompareOp::eGreaterOrEqual;
            case CompareOp::Always:             return vk::CompareOp::eAlways;
            default:                            return vk::CompareOp::eAlways;
        }
    }


    inline vk::SamplerCreateInfo ToVulkanSamplerCreateInfo(const SamplerKey &samplerInfo) {

        vk::SamplerCreateInfo createInfo{};
        createInfo.magFilter = ToVkFilter(samplerInfo.magFilter);
        createInfo.minFilter = ToVkFilter(samplerInfo.minFilter);
        createInfo.mipmapMode = ToVkSamplerMipMapMode(samplerInfo.mipmapMode);
        createInfo.addressModeU = ToVkSamplerAddressMode(samplerInfo.addressModeU);
        createInfo.addressModeV = ToVkSamplerAddressMode(samplerInfo.addressModeV);
        createInfo.addressModeW = ToVkSamplerAddressMode(samplerInfo.addressModeW);
        createInfo.mipLodBias = samplerInfo.mipLodBias;
        createInfo.anisotropyEnable = samplerInfo.anisotropyEnable;
        createInfo.maxAnisotropy = samplerInfo.maxAnisotropy;
        createInfo.compareEnable = samplerInfo.compareEnable;
        createInfo.compareOp = ToVkCompareOp(samplerInfo.compareOp);
        createInfo.minLod = samplerInfo.minLod;
        createInfo.maxLod = samplerInfo.maxLod;
        createInfo.borderColor = vk::BorderColor::eIntTransparentBlack;
        createInfo.unnormalizedCoordinates = samplerInfo.unnormalizedCoordinates;

        return createInfo;
    }
}