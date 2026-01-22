//
// Created by Faith Kamaraju on 2026-01-18.
//

#pragma once

#include <optional>

namespace LE {

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
        std::optional<uint32_t> transferFamily;
        [[nodiscard]] bool isComplete() const {
            return graphicsFamily.has_value() && presentFamily.has_value() && transferFamily.has_value();
        }
    };

}