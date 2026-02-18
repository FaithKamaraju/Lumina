//
// Created by Faith Kamaraju on 2026-01-18.
//

#pragma once
#include <vulkan/vulkan.hpp>
#include "VulkanImage.h"

struct GLFWwindow;

namespace LE {

    struct SwapChain {

        ~SwapChain() = default;
        SwapChain() = default;
        void CreateSwapChain(VulkanContext* context, GLFWwindow *windowRef);
        void RecreateSwapChain(GLFWwindow *windowRef);


        void createDepthResources();
        void createSwapChainImageViews();
        void destroySwapChain();

        bool m_FramebufferResized = false;
        vk::SwapchainKHR m_SwapChain = nullptr;
        vk::Format m_SwapChainImageFormat{};
        vk::Extent2D m_SwapChainExtent{};
        std::vector<vk::Image> m_SwapChainImages;
        std::vector<vk::ImageView> m_SwapChainImageViews;
        DepthImage m_DepthBuffer{};
        uint32_t minImageCount = 0;
        uint32_t imageCount = 0;

    private:
        VulkanContext* ctx = nullptr;
        GLFWwindow* m_WindowRef = nullptr;
        vk::SwapchainKHR m_OldSwapChain = nullptr;

    };
}
