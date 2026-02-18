//
// Created by Faith Kamaraju on 2026-01-14.
//

#pragma once
#include "Core/LE_Types.h"
#include "Core/Config.h"
#include "Scene/Scenegraph.h"

namespace LE {
    class Game;
    class Window;
    class EventManager;
    class LayerStack;
    class ResourceManager;
    class AssetRegistry;
    class RHI;
    class Renderer;

    class Engine {

    public:
        Engine() = default;
        ~Engine() = default;

        LEBool InitializeSubSystems();
        void StartEngineLoop();
        void HandleShutdown();
        void SetGameInstance(Game* gi){m_GameInstance = gi;}


        [[nodiscard]] Game* GetGameInstance() const {return m_GameInstance;}
        [[nodiscard]] EventManager * GetEventManager() const {return m_EventManager;}
        [[nodiscard]] Window* GetWindowObj() const {return m_IWindow;}
        [[nodiscard]] RHI* GetRHI() const {return m_RHIDevice;}
        [[nodiscard]] LayerStack* GetLayerStack() const {return m_LayerStack;}
        [[nodiscard]] AssetRegistry* GetAssetRegistry() const {return m_AssetRegistry;}
        [[nodiscard]] ResourceManager* GetResourceManager() const {return m_ResourceManager;}
        [[nodiscard]] Config& GetConfig() {return m_Config;}
        [[nodiscard]] Scenegraph& GetScenegraph() {return m_Scenegraph;}


    private:
        Config m_Config;
        Scenegraph m_Scenegraph;
        Game* m_GameInstance = nullptr;
        EventManager* m_EventManager = nullptr;
        Window* m_IWindow = nullptr;
        RHI* m_RHIDevice = nullptr;
        AssetRegistry* m_AssetRegistry = nullptr;
        Renderer* m_Renderer = nullptr;
        LayerStack* m_LayerStack = nullptr;
        ResourceManager* m_ResourceManager = nullptr;


    };

}
