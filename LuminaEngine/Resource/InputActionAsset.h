//
// Created by Faith Kamaraju on 2026-02-25.
//

#pragma once

#include <cereal/cereal.hpp>
#include "BaseResource.h"

namespace LE {

    enum class InputValueType : uint8_t {
        Digital = 0,
        Axis1D = 1,
        Axis2D = 3,
        Axis3D = 4,
    };

    enum class InputTrigger : uint8_t {
        None = 0,
        Pressed = 1,
        Released = 2,
        Down = 3,
        Hold = 4,
        HoldAndReleased = 5,
        // Tap = 6,
        // Combo = 7,
    };

    enum class InputModifier : uint8_t {
        None = 0,
        Scalar = 1,
        DeadZone = 2,
        SwizzleInputAxisValues = 3,
    };

    struct InputActionAsset : BaseResource {
        bool bConsumeInput = false;
        bool bTriggerWhenPause = false;
        InputValueType type = InputValueType::Digital;
        InputTrigger trigger = InputTrigger::None;
        InputModifier modifier = InputModifier::None;
        std::string actionDescription;
    };

    struct InputActionAssetHandle {
        int32_t id = -1;
        uint32_t generation = 0;
    };

    template<class Archive>
        void serialize(Archive& archive, InputActionAsset& action)
    {
        archive(
            action.bConsumeInput,
            action.bTriggerWhenPause,
            action.type,
            action.trigger,
            action.modifier,
            action.actionDescription
            );
    }
}
