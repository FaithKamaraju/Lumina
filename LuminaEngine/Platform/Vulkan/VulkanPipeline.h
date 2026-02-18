//
// Created by Faith Kamaraju on 2026-01-23.
//

#pragma once
#include <vulkan/vulkan.hpp>
#include "Rendering/Pipeline.h"

namespace LE {

    inline std::vector DYNAMIC_STATES = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor,
    };

    inline vk::PrimitiveTopology GetVKTopology(LE::Topology topology) {
        switch (topology) {
            case Topology::Points:
                return vk::PrimitiveTopology::ePointList;
            case Topology::Lines:
                return vk::PrimitiveTopology::eLineList;
            case Topology::LineLoop:
                return vk::PrimitiveTopology::eLineStrip;
            case Topology::LineStrip:
                return vk::PrimitiveTopology::eLineStrip;
            case Topology::Triangles:
                return vk::PrimitiveTopology::eTriangleList;
            case Topology::TriangleStrip:
                return vk::PrimitiveTopology::eTriangleStrip;
            case Topology::TriangleFan:
                return vk::PrimitiveTopology::eTriangleFan;
        }
        return vk::PrimitiveTopology::eTriangleList;
    }

    inline vk::CullModeFlags GetVKCullMode (CullMode mode) {
        switch (mode) {
            case CullMode::None:
                return vk::CullModeFlagBits::eNone;
            case CullMode::Front:
                return vk::CullModeFlagBits::eFront;
            case CullMode::Back:
                return vk::CullModeFlagBits::eBack;
            case CullMode::FrontAndBack:
                return vk::CullModeFlagBits::eFront | vk::CullModeFlagBits::eBack;
        }
        return vk::CullModeFlagBits::eBack;
    }



}
