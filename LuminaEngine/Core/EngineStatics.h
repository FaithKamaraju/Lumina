//
// Created by Faith Kamaraju on 2026-01-14.
//

#pragma once
#include "Engine.h"

namespace LE {

    class EventManager;
    class Window;

    inline Engine* gEngine;

    namespace Globals {

        inline Engine* GetEngine() { return gEngine;};
        EventManager* GetEventManager();
        Window* GetWindow();

    }
}
