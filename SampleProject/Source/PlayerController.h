//
// Created by Faith Kamaraju on 2026-03-07.
//

#pragma once

#include "Scripting/NativeScript.h"


class PlayerController : public LE::NativeScript {

    LE_SCRIPT_CLASS(PlayerController)

    public:
        void OnCreate() override;

};