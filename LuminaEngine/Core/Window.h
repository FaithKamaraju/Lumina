//
// Created by Faith Kamaraju on 2026-01-14.
//

#pragma once
#include "Core/LE_Types.h"

struct GLFWwindow;

namespace LE {

    class Window{

    private:
        struct WindowData {
            const char* Title;
            uint16_t Width { 0 };
            uint16_t Height { 0 };
            bool VSync { false };
            bool FullScreen { false };
            uint16_t FramebufferWidth { 0 };
            uint16_t FramebufferHeight { 0 };

        };

    public:
        Window(int width, int height, const char* title, bool vsync, bool fullscreen);
        ~Window();

        void InitWindow();

        GLFWwindow* GetGLFWWindow() {return m_Window;}

    private:
        GLFWwindow* m_Window;
        WindowData m_Data;

    };


    Ref<Window> CreateWindow(int width, int height, const char* title, bool vsync, bool fullscreen);


}
