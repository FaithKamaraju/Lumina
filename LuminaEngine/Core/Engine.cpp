//
// Created by Faith Kamaraju on 2026-01-14.
//

#include "Engine.h"

#include "Logger.h"
#include "GLFW/glfw3.h"

#include "Events/EventManager.h"
#include "Core/Window.h"
#include "../Game.h"
#include "Layers/LayerStack.h"
#include "Rendering/Renderer.h"
#include "Rendering/RHI.h"
#include "Resource/AssetRegistry.h"
#include "Resource/ResourceManager.h"



LEBool LE::Engine::InitializeSubSystems() {

    m_Config = LoadConfig("../../../../Config/DefaultConfig.ini");

    m_EventManager = new EventManager();

    m_IWindow = CreateWindow(m_Config.window_config.width,
                            m_Config.window_config.height,
                        m_Config.window_config.title.c_str(),
                        m_Config.window_config.vsync,
                        m_Config.window_config.fullscreen);

    m_RHIDevice = CreateRHI(m_Config.graphics_config.graphicsAPI);
    m_RHIDevice->InitDevice(m_IWindow->GetGLFWWindow());

    m_AssetRegistry = new AssetRegistry(m_RHIDevice);

    m_LayerStack = new LayerStack();

    m_Renderer = new Renderer(m_AssetRegistry, m_RHIDevice);
    m_ResourceManager = new ResourceManager(m_RHIDevice, m_AssetRegistry);

    // m_ResourceManager->ImportGLTFFile("../../../Content/Models/DamagedHelmet.gltf","DamagedHelmet");
    m_ResourceManager->LoadSceneAsset("../../../Content/Assets/DamagedHelmet.LEASSET", m_Scenegraph.nodes[0]);
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
        m_Renderer->WalkScenegraph(m_Scenegraph, time);


    }


}

void LE::Engine::HandleShutdown() {
    delete m_ResourceManager;
    delete m_LayerStack;
    delete m_Renderer;
    delete m_AssetRegistry;
    delete m_RHIDevice;
    delete m_IWindow;
    m_EventManager->Shutdown();
    delete m_EventManager;
    delete m_GameInstance;
    if (m_Config.changed) {
        SaveConfig("../../../Config/DefaultConfig.ini", m_Config);
    }
}
