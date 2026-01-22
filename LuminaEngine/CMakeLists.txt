cmake_minimum_required(VERSION 4.0)
project(LuminaEngine C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)

if(MVSC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /EHsc")
    set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "/EHsc")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
endif(MVSC)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(LE_PLATFORM "LE_PLATFORM_WIN")

    #    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
endif(CMAKE_SYSTEM_NAME STREQUAL "Windows")

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(LE_PLATFORM "LE_PLATFORM_MAC")
    #    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_MACOS_MVK)
endif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")

#set(SHADERC_ENABLE_SHARED_CRT ON)
#set(IMGUI_DIR external/imgui)
##set(SHADERC_SKIP_INSTALL ON)
#set(SHADERC_SKIP_TESTS ON)
#set(SHADERC_SKIP_EXAMPLES ON)


find_package(Vulkan REQUIRED)
add_subdirectory(external/glfw/)
add_subdirectory(external/glm/)
add_subdirectory(external/spdlog/)
#add_subdirectory(external/shaderc/)
#add_subdirectory(external/fastgltf/)
#add_subdirectory(external/volk/)

add_library(VulkanCppModule INTERFACE)
add_library(Vulkan::cppm ALIAS VulkanCppModule)
target_link_libraries(VulkanCppModule INTERFACE Vulkan::Vulkan)
target_compile_definitions(VulkanCppModule
        INTERFACE   VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
        VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
        VULKAN_HPP_NO_SPACESHIP_OPERATOR=1
        VK_ENABLE_BETA_EXTENSIONS=1
)

add_library(${PROJECT_NAME} STATIC

        # mINI files

        external/mINI/src/mini/ini.h

        # stb_image files
        external/stbImage/stb_image.cpp

        # Vulkan Memory Allocator
        external/vma/src/VmaUsage.h
        external/vma/src/VmaUsage.cpp

        #Engine source files
        LuminaEngine.h
        Core/EntryPoint.cpp
        Core/EntryPoint.h
        Game.cpp
        Game.h
        Core/EngineStatics.h
        Core/EngineStatics.cpp
        Core/Logger.cpp
        Core/Logger.h
        Core/Engine.cpp
        Core/Engine.h
        Core/LE_Types.h
        Core/Window.cpp
        Core/Window.h
        Layers/Layer.h
        Layers/Layer.cpp
        Layers/LayerStack.h
        Layers/LayerStack.cpp
        Rendering/RHI.h
        Core/Events/Event.h
        Core/Events/EventHandler.h
        Core/Events/EventManager.cpp
        Core/Events/EventManager.h
        Core/Events/KeyEvent.h
        Core/Events/WindowEvent.h
        Resource/ResourceManager.cpp
        Resource/ResourceManager.h
        Rendering/Renderer.cpp
        Rendering/Renderer.h
        Core/Config.cpp
        Core/Config.h
        Platform/Vulkan/VulkanRHI.cpp
        Platform/Vulkan/VulkanRHI.h
        Platform/Vulkan/VulkanUtils.cpp
        Platform/Vulkan/VulkanUtils.h
        Platform/Vulkan/QueueFamilyIndices.h
        Platform/Vulkan/VulkanImage.cpp
        Platform/Vulkan/VulkanImage.h
        Rendering/Image.h
        Platform/Vulkan/VulkanBuffer.cpp
        Platform/Vulkan/VulkanBuffer.h
        Rendering/Buffer.h
        Platform/Vulkan/VulkanContext.h
        Platform/Vulkan/VulkanSwapChain.cpp
        Platform/Vulkan/VulkanSwapChain.h
        Rendering/VertexInput.h
        Rendering/RenderingConstants.h
        Rendering/Sampler.h
        Platform/Vulkan/FrameData.cpp
        Platform/Vulkan/FrameData.h
        Platform/Vulkan/Descriptors.cpp
        Platform/Vulkan/Descriptors.h
        Platform/Vulkan/VulkanSampler.h
        Rendering/Renderable.h
)

target_link_libraries(${PROJECT_NAME} PUBLIC glfw glm spdlog Vulkan::cppm )

target_precompile_headers(${PROJECT_NAME} PRIVATE lepch.h)



set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
target_compile_definitions(${PROJECT_NAME} PUBLIC ${LE_PLATFORM})

target_include_directories(${PROJECT_NAME} PUBLIC external/glfw/include/)
target_include_directories(${PROJECT_NAME} PUBLIC external/glm/)
target_include_directories(${PROJECT_NAME} PUBLIC external/spdlog/include/)
target_include_directories(${PROJECT_NAME} PUBLIC external/mINI/src/)
target_include_directories(${PROJECT_NAME} PRIVATE external/stbImage/)
target_include_directories(${PROJECT_NAME} PRIVATE external/vma/include/)
target_include_directories(${PROJECT_NAME} PRIVATE external/vma/src/)
#target_include_directories(${PROJECT_NAME} PRIVATE external/fastgltf/include)

target_include_directories(${PROJECT_NAME} PRIVATE ./)

