//
// Created by Faith Kamaraju on 2026-01-18.
//

#include "VulkanUtils.h"

void LE::Utils::createMoltenVkLayerSettings(vk::InstanceCreateInfo& instanceInfo) {

    vk::LayerSettingEXT layerSetting;
    layerSetting.pLayerName   = "MoltenVK";
    layerSetting.pSettingName = "MVK_CONFIG_USE_METAL_ARGUMENT_BUFFERS";
    layerSetting.type         = vk::LayerSettingTypeEXT::eInt32;
    layerSetting.valueCount   = 1;

    static constexpr int32_t useMetalArgumentBuffers = 1;
    layerSetting.pValues                         = &useMetalArgumentBuffers;

    vk::LayerSettingsCreateInfoEXT layerSettingsCreateInfo;
    layerSettingsCreateInfo.settingCount = 1;
    layerSettingsCreateInfo.pSettings    = &layerSetting;
    layerSettingsCreateInfo.pNext        = instanceInfo.pNext;
    instanceInfo.pNext                  = &layerSettingsCreateInfo;

}

bool LE::Utils::CheckValidationLayerSupport() {

    auto instanceValidationLayers = vk::enumerateInstanceLayerProperties();

    for (const char *layer: instanceLayers) {
        bool layerFound = false;
        for (int i = 0; i < instanceValidationLayers.size(); i++) {
            if (strcmp(layer, instanceValidationLayers[i].layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        if (!layerFound) {
            return false;
        }
    }

    return true;
}

std::vector<const char *> LE::Utils::GetRequiredExtensions() {

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    auto extensionProperties = vk::enumerateInstanceExtensionProperties();
    std::vector<const char*> requiredExtensions;
    requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    if (enableValidationLayers) {
        requiredExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    for (int i = 0; i < glfwExtensionCount; i++) {
        if (std::ranges::none_of(extensionProperties,
                             [glfwExtension = glfwExtensions[i]](auto const& extensionProperty)
                             { return strcmp(extensionProperty.extensionName, glfwExtension) == 0; }))
        {
            throw std::runtime_error("Required GLFW extension not supported: " + std::string(glfwExtensions[i]));
        }
        else {
            requiredExtensions.emplace_back(glfwExtensions[i]);
        }
    }
    LE_CORE_INFO("Activate extensions:");
    for (const auto& extension : requiredExtensions) {
        LE_CORE_INFO("\t{0}", extension);
    }

    return requiredExtensions;
}

int LE::Utils::ScoreDevices(const vk::PhysicalDevice& device) {
    int score = 0;
    vk::PhysicalDeviceProperties deviceProperties = device.getProperties();
    // deviceProperties.deviceType

    // Discrete GPUs have a significant performance advantage
    if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
        score += 1000;

    }
    if (deviceProperties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu) {
        score += 500;
    }

    // Maximum possible size of textures affects graphics quality
    // score += deviceProperties.limits.maxImageDimension2D;

    LE_CORE_INFO("Score for device: {0} : {1}", deviceProperties.deviceName.data(), score);

    return score;
}

bool LE::Utils::CheckDeviceExtensionsSupport(const vk::PhysicalDevice& device) {

    auto extensions = device.enumerateDeviceExtensionProperties();
    std::set<std::string> requiredExtensions(deviceExtensions.begin(),deviceExtensions.end());

    for (const auto& extension : extensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

LE::SwapChainSupportDetails LE::Utils::QuerySwapChainSupportDetails(const vk::PhysicalDevice& device,
                                                                            const vk::SurfaceKHR& surface)
{
    SwapChainSupportDetails details {
    .capabilities = device.getSurfaceCapabilitiesKHR(surface),
    .formats = device.getSurfaceFormatsKHR(surface),
    .presentModes = device.getSurfacePresentModesKHR(surface),
    };
    return details;
}

LE::QueueFamilyIndices LE::Utils::CheckQueueFamilySupport(const vk::PhysicalDevice& device,
                                                                    const vk::SurfaceKHR& surface) {

    auto queueFamilies = device.getQueueFamilyProperties();

    QueueFamilyIndices temp{};
    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (temp.isComplete()) break;

        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
            temp.graphicsFamily = i;
        }

        if ((queueFamily.queueFlags & vk::QueueFlagBits::eTransfer)
            && queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
            temp.transferFamily = i;
        }

        auto presentSupport = device.getSurfaceSupportKHR(i, surface);
        if (presentSupport) {
            temp.presentFamily = i;
        }
        i++;
    }

    return temp;
}

vk::SurfaceFormatKHR LE::Utils::ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats) {

    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == vk::Format::eB8G8R8A8Srgb
            && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return availableFormat;
        }
    }
    return availableFormats[0];
}

vk::PresentModeKHR LE::Utils::ChoosePresentMode(const std::vector<vk::PresentModeKHR> &presentModes) {

    for (const auto& presentMode : presentModes) {
        if (presentMode == vk::PresentModeKHR::eMailbox) {
            return presentMode;
        }
    }

    return vk::PresentModeKHR::eFifo ;
}

vk::Extent2D LE::Utils::ChooseSwapExtent(GLFWwindow *window, const vk::SurfaceCapabilitiesKHR &capabilities) {

    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }
    else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        vk::Extent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width,
            capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height,
            capabilities.maxImageExtent.height);

        return actualExtent;
    }
}





vk::ShaderModule LE::Utils::CreateShaderModule(vk::Device device, std::vector<uint32_t> &code) {

    vk::ShaderModuleCreateInfo createInfo{};
    createInfo.codeSize = code.size() * sizeof(uint32_t);
    createInfo.pCode = code.data();

    vk::ShaderModule module = device.createShaderModule(createInfo);

    return module;
}

uint32_t LE::Utils::FindMemoryType(vk::PhysicalDevice physicalDevice, uint32_t typeFilter,
                                        vk::MemoryPropertyFlags properties)
{
    vk::PhysicalDeviceMemoryProperties memProps =  physicalDevice.getMemoryProperties();
    for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProps.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    throw std::runtime_error("failed to find suitable memory type!");
}

vk::Format LE::Utils::findSupportedFormat(vk::PhysicalDevice primaryPhysicalDevice, const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) {
    for (const auto format : candidates) {
        vk::FormatProperties props = primaryPhysicalDevice.getFormatProperties(format);

        if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
            return format;
        }
        if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}


