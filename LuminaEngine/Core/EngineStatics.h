//
// Created by Faith Kamaraju on 2026-01-14.
//

#pragma once
#include "Engine.h"

namespace LE {

    inline Engine* gEngine;

    namespace Globals {

        inline Engine* GetEngine() { return gEngine;};
        Game* GetGameInstance();
        EventManager* GetEventManager();
        Window* GetWindow();
        RHI* GetRHI();
        LayerStack* GetLayerStack();
        AssetRegistry* GetAssetRegistry();
        ResourceManager* GetResourceManager();
        Config& GetConfig();

        Scenegraph& GetCurrentSceneGraph();



        static void LoadLevel();

    }
}
