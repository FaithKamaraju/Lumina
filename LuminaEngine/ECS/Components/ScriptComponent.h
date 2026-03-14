//
// Created by Faith Kamaraju on 2026-03-05.
//

#pragma once
#include "Scripting/NativeScript.h"

namespace LE {

    struct ScriptComponent {

        uint64_t hashedName;
        NativeScript* scriptInstance = nullptr;
    };

}
