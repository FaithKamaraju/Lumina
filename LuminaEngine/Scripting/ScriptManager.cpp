//
// Created by Faith Kamaraju on 2026-03-07.
//

#include "ScriptManager.h"
#include "Core/EngineStatics.h"
#include "ECS/Coordinator.h"
#include "ECS/Components/ScriptComponent.h"

LE::ScriptManager::ScriptManager() {
}

LE::ScriptManager::~ScriptManager() {
}

void LE::ScriptManager::SetSystemSignature() {
    Signature signature;
    auto coordinator = Globals::GetCoordinator();
    signature.set(coordinator->GetComponentType<ScriptComponent>());
    coordinator->SetSystemSignature<ScriptManager>(signature);
}


void LE::ScriptManager::UpdateScripts(float ts) {

    for (auto& entity : mEntities) {
        auto& script = Globals::GetCoordinator()->GetComponent<ScriptComponent>(entity);
        script.scriptInstance->OnUpdate(ts);
    }
}
