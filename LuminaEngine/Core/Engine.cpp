//
// Created by Faith Kamaraju on 2026-01-14.
//

#include "Engine.h"

#include "Logger.h"
#include "GLFW/glfw3.h"

#include "Scripting/ModuleRegistry.h"
#include "Events/EventManager.h"
#include "Core/Window.h"
#include "../Game.h"
#include "Layers/LayerStack.h"
#include "Rendering/Renderer.h"
#include "Rendering/RHI.h"
#include "Resource/AssetRegistry.h"
#include "Resource/ResourceManager.h"
#include "ECS/Coordinator.h"
#include "Scene/Scenegraph.h"
#include "Scripting/ScriptManager.h"
#include "DynamicLibrary.h"

#if LE_HOT_RELOAD == 0
extern "C" void RegisterGameModule(LE::ModuleRegistry* registry);
#endif

LEBool LE::Engine::InitializeSubSystems() {

    m_Config = LoadConfig("../../../../Config/DefaultConfig.ini");

    m_ModuleRegistry = CreateRef<ModuleRegistry>();
    DynamicLibrary lib;

#if LE_HOT_RELOAD == 0
    RegisterGameModule(m_ModuleRegistry.get());

#else
    using RegisterModuleFn = void(*)(ModuleRegistry*);
    std::string err;
    if (!lib.Load("../../../../SampleProject/Binaries/RelWithDebInfo/lib/libSampleProject.dylib", &err)) {
        // log err
    }

    auto RegisterModule = lib.GetSymbol<RegisterModuleFn>("RegisterGameModule", &err);
    if (!RegisterModule) {
        // log err
    }
    RegisterModule(m_ModuleRegistry.get());

#endif


    m_EventManager = CreateRef<EventManager>();

    m_IWindow = CreateWindow(m_Config.window_config.width,
                            m_Config.window_config.height,
                        m_Config.window_config.title.c_str(),
                        m_Config.window_config.vsync,
                        m_Config.window_config.fullscreen);

    m_RHIDevice = CreateRHI(m_Config.graphics_config.graphicsAPI);
    m_RHIDevice->InitDevice(m_IWindow->GetGLFWWindow());

    m_Coordinator = CreateRef<Coordinator>();
    m_Coordinator->Init();
    m_Coordinator->RegisterDefaultComponents();

    m_Scenegraph = CreateRef<Scenegraph>();
    m_AssetRegistry = CreateRef<AssetRegistry>(m_RHIDevice.get());

    m_LayerStack = CreateRef<LayerStack>();

    m_Renderer = CreateRef<Renderer>(m_AssetRegistry.get(), m_RHIDevice.get());
    m_ResourceManager = CreateRef<ResourceManager>(m_RHIDevice.get(), m_AssetRegistry.get());

    m_ScriptManager = m_Coordinator->RegisterSystem<ScriptManager>();
    m_ScriptManager->SetSystemSignature();

    // m_ResourceManager->CreateSceneAssetFromGLTF("../../../Content/Models/DamagedHelmet.gltf",
    //     "../../../Content/Assets/", "DamagedHelmet");
    // m_ResourceManager->CreateSceneAssetFromGLTF("../../../Content/Models/Lantern.gltf",
    //     "../../../Content/Assets/","Lantern");
    m_ResourceManager->LoadSceneAsset("../../../Content/Assets/DamagedHelmet.LEASSET", 0, -1 );
    // m_ResourceManager->LoadSceneAsset("../../../Content/Assets/Fox.LEASSET", {});
    // m_ResourceManager->LoadSceneAsset("../../../Content/Assets/Lantern.LEASSET", 0, -1);
    // m_LayerStack->PushLayer(new ImguiLayer());


    return LE_SUCCESS;
}

void LE::Engine::StartEngineLoop() {

    while(!glfwWindowShouldClose(m_IWindow->GetGLFWWindow())) {

        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        glfwPollEvents();
        m_EventManager->DispatchEvents();
        // for (auto& layer : *m_LayerStack) {
        //     layer->OnImGuiRender();
        // }
        m_Renderer->WalkScenegraph(time);

    }

}

void LE::Engine::HandleShutdown() {

    delete m_GameInstance;
    if (m_Config.changed) {
        SaveConfig("../../../Config/DefaultConfig.ini", m_Config);
    }
}
