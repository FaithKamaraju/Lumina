//
// Created by Faith Kamaraju on 2026-01-14.
//

#include "Window.h"
#include <GLFW/glfw3.h>
#include "Core/Logger.h"
#include "Events/EventManager.h"
#include "Events/WindowEvent.h"

// #include "backends/imgui_impl_glfw.h"


LE::Window::Window(int width, int height, const char* title, bool vsync, bool fullscreen)
{
    m_Data.Title = title;
    m_Data.Width = width;
    m_Data.Height = height;
    m_Data.VSync = vsync;
    m_Data.FullScreen = fullscreen;

}

LE::Window::~Window() {


    glfwDestroyWindow(m_Window);
    glfwTerminate();
}

void LE::Window::InitWindow() {

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());
    // m_Window = glfwCreateWindow((int)(main_scale * m_Data.Width),
    //     (int)(main_scale * m_Data.Height), m_Data.Title, nullptr, nullptr);
    // LE_CORE_INFO("Created a window of {} x {} pixels", (int)(main_scale * m_Data.Width),
    //     (int)(main_scale * m_Data.Height));
    m_Window = glfwCreateWindow(m_Data.Width, m_Data.Height, m_Data.Title, nullptr, nullptr);
    LE_CORE_INFO("Created a window of {} x {} pixels", m_Data.Width, m_Data.Height);

    glfwSetWindowUserPointer(m_Window, &m_Data);

    glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height){

        WindowData* windowData = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
        windowData->Width = width;
        windowData->Height = height;

        const WindowResizeEvent resize_event = WindowResizeEvent(width, height);
        Events::TriggerEvent(resize_event);
    });

    glfwSetWindowFocusCallback(m_Window, [](GLFWwindow* window, int focused) {

        if (focused) {
            Events::QueueEvent(std::make_unique<WindowFocusEvent>(WindowFocusEvent{}));
        }
        else {
            Events::QueueEvent(std::make_unique<WindowLostFocusEvent>(WindowLostFocusEvent{}));
        }
    });

    glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {

        WindowData* windowData = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
        windowData->FramebufferWidth = width;
        windowData->FramebufferHeight = height;

        const WindowFramebufferResizeEvent resize_event = WindowFramebufferResizeEvent(width, height);
        Events::TriggerEvent(resize_event);
    });


}

LE::Ref<LE::Window> LE::CreateWindow(int width, int height, const char* title, bool vsync, bool fullscreen)
{
    auto temp = CreateRef<Window>(width, height, title, vsync, fullscreen);
    temp->InitWindow();
    return temp;
}


