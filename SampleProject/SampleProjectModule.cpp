//
// Created by Faith Kamaraju on 2026-03-07.
//

#include "Scripting/ModuleRegistry.h"
#include "Source/SampleGame.h"
#include "Source/PlayerController.h"

#ifdef LE_PLATFORM_WIN
    #define LUMINA_EXPORT extern "C" __declspec(dllexport)
#else
    #define LUMINA_EXPORT extern "C" __attribute__((visibility("default")))
#endif


LUMINA_EXPORT void RegisterGameModule(LE::ModuleRegistry* registry)
{
    LE::SetCreateGameFunction(&CreateGame);
    registry->RegisterScript<PlayerController>();
    // registry.Register<EnemyAI>("EnemyAI");
}

