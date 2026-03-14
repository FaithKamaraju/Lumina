//
// Created by Faith Kamaraju on 2026-03-14.
//

#include "Scripting/ModuleRegistry.h"
#include "src/EditorCamera.h"

#ifdef LE_PLATFORM_WIN
    #define LUMINA_EXPORT extern "C" __declspec(dllexport)
#else
    #define LUMINA_EXPORT extern "C" __attribute__((visibility("default")))
#endif


LUMINA_EXPORT void RegisterGameModule(LE::ModuleRegistry* registry)
{
    registry->RegisterScript<LE::EditorCamera>();
}