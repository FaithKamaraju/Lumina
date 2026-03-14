//
// Created by Faith Kamaraju on 2026-01-14.
//

#pragma once
#include "Core/LE_Types.h"
#include "Core/Config.h"
#include <memory>


namespace LE {
    class Game;
    class Window;
    class EventManager;
    class LayerStack;
    class ResourceManager;
    class AssetRegistry;
    class RHI;
    class Renderer;
    class Coordinator;
    class Scenegraph;
    class ModuleRegistry;
    class ScriptManager;

    class Engine {

    public:
        Engine() = default;
        ~Engine() = default;

        LEBool InitializeSubSystems();
        void StartEngineLoop();
        void HandleShutdown();
        void SetGameInstance(Game* gi){m_GameInstance = gi;}

        [[nodiscard]] ModuleRegistry* GetModuleRegistry() const { return m_ModuleRegistry.get(); }
        [[nodiscard]] Game* GetGameInstance() const { return m_GameInstance; }
        [[nodiscard]] EventManager * GetEventManager() const { return m_EventManager.get(); }
        [[nodiscard]] Window* GetWindowObj() const { return m_IWindow.get(); }
        [[nodiscard]] RHI* GetRHI() const { return m_RHIDevice.get(); }
        [[nodiscard]] LayerStack* GetLayerStack() const { return m_LayerStack.get(); }
        [[nodiscard]] AssetRegistry* GetAssetRegistry() const { return m_AssetRegistry.get(); }
        [[nodiscard]] ResourceManager* GetResourceManager() const { return m_ResourceManager.get(); }
        [[nodiscard]] Coordinator* GetCoordinator() const { return m_Coordinator.get(); }
        [[nodiscard]] Scenegraph* GetScenegraph() const { return m_Scenegraph.get(); }

        [[nodiscard]] Config& GetConfig() { return m_Config; }


    private:
        Config m_Config;
        Ref<ModuleRegistry> m_ModuleRegistry = nullptr;
        Ref<Scenegraph> m_Scenegraph = nullptr;;
        Game* m_GameInstance = nullptr;
        Ref<EventManager> m_EventManager = nullptr;
        Ref<Window> m_IWindow = nullptr;
        Ref<RHI> m_RHIDevice = nullptr;
        Ref<AssetRegistry> m_AssetRegistry = nullptr;
        Ref<ScriptManager> m_ScriptManager = nullptr;
        Ref<Renderer> m_Renderer = nullptr;
        Ref<LayerStack> m_LayerStack = nullptr;
        Ref<ResourceManager> m_ResourceManager = nullptr;
        Ref<Coordinator> m_Coordinator = nullptr;


    };

}
