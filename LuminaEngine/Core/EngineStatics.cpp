//
// Created by Faith Kamaraju on 2026-01-14.
//

#include "EngineStatics.h"
#include "Events/EventManager.h"
#include "Core/Window.h"

LE::EventManager* LE::Globals::GetEventManager() {

    return gEngine->GetEventManager();
}

LE::Window* LE::Globals::GetWindow() {

    return gEngine->GetWindowObj();
}