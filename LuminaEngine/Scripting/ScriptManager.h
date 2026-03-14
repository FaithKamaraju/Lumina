//
// Created by Faith Kamaraju on 2026-03-07.
//

#pragma once
#include "ECS/System.h"

namespace LE {

    class ScriptManager : public System {

    public:
        ScriptManager();
        ~ScriptManager() override;

        void SetSystemSignature() override;

        void UpdateScripts(float ts);

    private:

    };
}
