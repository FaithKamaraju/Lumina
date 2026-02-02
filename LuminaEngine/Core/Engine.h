//
// Created by Faith Kamaraju on 2026-01-14.
//

#pragma once
#include "Core/LE_Types.h"
#include "Core/Config.h"

namespace LE {

    class Game;
    class Window;
    class EventManager;
    class LayerStack;
    class ResourceManager;

    class Engine {

    public:
        Engine() = default;
        ~Engine() = default;

        LEBool InitializeSubSystems();
        void StartEngineLoop();
        void HandleShutdown();
        void SetGameInstance(Game* gi){m_GameInstance = gi;}


        Game* GetGameInstance() const {return m_GameInstance;}
        Window* GetWindowObj() const {return m_IWindow;}
        EventManager * GetEventManager() const {return m_EventManager;}
        LayerStack* GetLayerStack() const {return m_LayerStack;}


    private:
        Game* m_GameInstance;
        Window* m_IWindow;
        EventManager* m_EventManager;
        LayerStack* m_LayerStack;
        ResourceManager* m_ResourceManager;
        Config m_Config;
        // Vulkan::RHI* m_RHIDevice;

    };

}
