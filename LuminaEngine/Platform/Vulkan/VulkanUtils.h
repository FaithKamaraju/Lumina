//
// Created by Faith Kamaraju on 2026-01-18.
//

#pragma once

#include <vulkan/vulkan.hpp>
#include "Core/Logger.h"
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "shaderc/shaderc.hpp"
#include "Platform/Vulkan/QueueFamilyIndices.h"
#include "Rendering/VertexInput.h"



inline shaderc::CompileOptions* g_CompileOptions;
inline shaderc::Compiler* g_ShaderCompiler;

inline constexpr int MAX_FRAMES_IN_FLIGHT = 2;

struct GLFWwindow;

namespace LE {

    struct SwapChainSupportDetails {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentModes;
    };
    struct PoolSize{
        vk::DescriptorType type;
        uint32_t count;
    };
}

namespace LE::Utils {

#ifdef NDEBUG
    inline const bool enableValidationLayers = false;
#else
    inline constexpr bool enableValidationLayers = true;
#endif

    inline const std::vector<const char*> instanceLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
#ifdef LE_PLATFORM_MAC
    inline const std::vector<const char*> deviceExtensions = {
        vk::KHRPortabilitySubsetExtensionName,
        vk::KHRSwapchainExtensionName,
        vk::KHRSpirv14ExtensionName,
        vk::KHRSynchronization2ExtensionName,
        vk::KHRCreateRenderpass2ExtensionName,
    };
#elif defined LE_PLATFORM_WIN
    inline const std::vector<const char*> deviceExtensions = {
        vk::KHRSwapchainExtensionName,
        vk::KHRSpirv14ExtensionName,
        vk::KHRSynchronization2ExtensionName,
        vk::KHRCreateRenderpass2ExtensionName};
#endif

    inline std::vector<vk::DynamicState> dynamicStates = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor,
    };

    static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
            vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
            vk::DebugUtilsMessageTypeFlagsEXT type,
            const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void*)
    {
        if (severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError) {
            LE_CORE_ERROR("validation layer: {0}", pCallbackData->pMessage);
        }
        else if (severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning) {
            LE_CORE_WARN("validation layer: {0}", pCallbackData->pMessage);
        }
        else if (severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo) {
            LE_CORE_INFO("validation layer: {0}", pCallbackData->pMessage);
        }

        return VK_FALSE;
    }

    inline vk::VertexInputBindingDescription GetVertexInputBindingDescription() {
        vk::VertexInputBindingDescription descInfo{
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = vk::VertexInputRate::eVertex,
        };
        return descInfo;
    }

    inline std::array<vk::VertexInputAttributeDescription, 3> GetVertexInputAttributeDescriptions() {

        return {
            vk::VertexInputAttributeDescription( 0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos) ),
            vk::VertexInputAttributeDescription( 1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color) ),
            vk::VertexInputAttributeDescription( 2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, texCoord) )
        };
    }
    void createMoltenVkLayerSettings(vk::InstanceCreateInfo& instanceInfo);
    bool CheckValidationLayerSupport();
    std::vector<const char*> GetRequiredExtensions();

    int ScoreDevices(const vk::PhysicalDevice& device);
    // void CheckForUnifiedMemory(Context* ctx, const vk::PhysicalDevice& device);
    bool CheckDeviceExtensionsSupport(const vk::PhysicalDevice& device);
    SwapChainSupportDetails QuerySwapChainSupportDetails(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface);
    QueueFamilyIndices CheckQueueFamilySupport(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface);

    vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    vk::PresentModeKHR ChoosePresentMode(const std::vector<vk::PresentModeKHR>& presentModes);
    vk::Extent2D ChooseSwapExtent(GLFWwindow* window, const vk::SurfaceCapabilitiesKHR& capabilities);

    void InitializeShadercCompiler(shaderc_optimization_level optimization_level = shaderc_optimization_level_zero, shaderc_source_language sourceLanguage = shaderc_source_language_glsl);
    void CleanupShaderc();
    std::vector<char> ReadShaderSource(const char* filePath);
    std::vector<uint32_t> CompileShader(const char* code, shaderc_shader_kind shaderKind, const char* source_name, const shaderc::CompileOptions* compileOptions = g_CompileOptions);
    std::vector<char> ReadSPVFile(const std::string& filename);
    vk::ShaderModule CreateShaderModule(vk::Device device, std::vector<uint32_t>& code);

    uint32_t FindMemoryType(vk::PhysicalDevice physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties);
    vk::Format findSupportedFormat(vk::PhysicalDevice primaryPhysicalDevice, const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);

}
