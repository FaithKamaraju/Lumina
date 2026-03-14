//
// Created by Faith Kamaraju on 2026-02-25.
//

#pragma once
#include "InputActionAsset.h"

namespace LE {

    struct Binding {
        InputActionAssetHandle action{};
        std::string path{};
    };

    struct InputActionMapping : BaseResource {
        float priority = 1.f;
        std::unordered_map<int, size_t> scanCodesToIndex;
        std::vector<Binding> actions;
        std::string mappingDescription;
    };

    struct InputActionMappingAssetHandle {
        int32_t id = -1;
        uint32_t generation = 0;
    };

}
