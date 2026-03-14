//
// Created by Faith Kamaraju on 2026-02-22.
//

#pragma once
#include "Resource/InputActionMappingAsset.h"
#include "Core/LE_Types.h"

namespace LE {

    class Window;

    class InputManager {

    public:

        InputManager();
        ~InputManager() = default;

        void PollInputEvents(Window* window);

        void ActivateInputActionMapping(InputActionMappingAssetHandle mapping);
        void DeactivateInputActionMapping(InputActionMappingAssetHandle mapping);



    private:

        // std::unordered_map<>



    };

}
