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
#include "Resource/ResourceManager.h"



LEBool LE::Engine::InitializeSubSystems() {

    m_Config = LoadConfig("../../../Config/DefaultConfig.ini");

    m_EventManager = new EventManager();

    m_IWindow = CreateWindow(m_Config.window_config.width,
                            m_Config.window_config.height,
                        m_Config.window_config.title.c_str(),
                        m_Config.window_config.vsync,
                        m_Config.window_config.fullscreen);

    m_LayerStack = new LayerStack();

    m_ResourceManager = new ResourceManager(nullptr);

    // auto model = m_ResourceManager->ImportGLTFFile("../../../Resources/Models/DamagedHelmet.gltf", "DH_gltf");
    // auto model = m_ResourceManager->LoadSceneAsset("../../../Assets/DH_gltf.LEASSET");
    // m_LayerStack->PushLayer(new ImguiLayer());

    // m_RHIDevice = new Vulkan::RHI();
    // m_RHIDevice->InitDevice(m_IWindow->GetGLFWWindow());

    return LE_SUCCESS;
}

void LE::Engine::StartEngineLoop() {

    while(!glfwWindowShouldClose(m_IWindow->GetGLFWWindow())) {

        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        glfwPollEvents();
        m_EventManager->DispatchEvents();
        for (auto& layer : *m_LayerStack) {
            layer->OnImGuiRender();
        }


    }


}

void LE::Engine::HandleShutdown() {
    if (m_Config.changed) {
        SaveConfig("../../../Config/DefaultConfig.ini", m_Config);
    }
    // delete m_LayerStack;
    // delete m_RHIDevice;
    delete m_IWindow;
    m_EventManager->Shutdown();
    delete m_EventManager;
    delete m_GameInstance;
}
