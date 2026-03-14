//
// Created by Faith Kamaraju on 2026-01-14.
//

#include "EngineStatics.h"

LE::Game* LE::Globals::GetGameInstance() {
    return gEngine->GetGameInstance();
}

LE::EventManager* LE::Globals::GetEventManager() {
    return gEngine->GetEventManager();
}

LE::Window* LE::Globals::GetWindow() {
    return gEngine->GetWindowObj();
}

LE::RHI* LE::Globals::GetRHI() {
    return gEngine->GetRHI();
}

LE::LayerStack* LE::Globals::GetLayerStack() {
    return gEngine->GetLayerStack();
}

LE::AssetRegistry* LE::Globals::GetAssetRegistry() {
    return gEngine->GetAssetRegistry();
}

LE::ResourceManager* LE::Globals::GetResourceManager() {
    return gEngine->GetResourceManager();
}

LE::Coordinator * LE::Globals::GetCoordinator() {
    return gEngine->GetCoordinator();
}

LE::Config& LE::Globals::GetConfig() {
    return gEngine->GetConfig();
}

LE::Scenegraph* LE::Globals::GetCurrentSceneGraph() {
    return gEngine->GetScenegraph();
}