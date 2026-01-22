//
// Created by Faith Kamaraju on 2026-01-18.
//

#include "VulkanSwapChain.h"
#include "VulkanUtils.h"
#include "VulkanContext.h"

LE::SwapChain::~SwapChain() {

   destroySwapChain();
}

void LE::SwapChain::CreateSwapChain(VulkanContext* context, GLFWwindow *windowRef) {

    ctx = context;
    m_WindowRef = windowRef;

    SwapChainSupportDetails details = Utils::QuerySwapChainSupportDetails(ctx->primaryPhysicalDevice, ctx->surface);
    vk::SurfaceFormatKHR format = Utils::ChooseSwapSurfaceFormat(details.formats);
    vk::PresentModeKHR presentMode = Utils::ChoosePresentMode(details.presentModes);
    vk::Extent2D swapExtent = Utils::ChooseSwapExtent(m_WindowRef, details.capabilities);

    minImageCount = details.capabilities.minImageCount;
    imageCount = details.capabilities.minImageCount + 1;
    if (details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount) {
        imageCount = details.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR createInfo{
    .surface = ctx->surface,
    .minImageCount = imageCount,
    .imageFormat = format.format,
    .imageColorSpace = format.colorSpace,
    .imageExtent = swapExtent,
    .imageArrayLayers = 1,
    .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
    };

    uint32_t queueFamilyIndices[] = {ctx->queueFamilyIndices.graphicsFamily.value(),
                                    ctx->queueFamilyIndices.presentFamily.value()};
    if (ctx->queueFamilyIndices.graphicsFamily.value() != ctx->queueFamilyIndices.presentFamily.value()) {
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
    }

    createInfo.preTransform = details.capabilities.currentTransform;
    createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = m_SwapChain;

    m_SwapChain = ctx->device.createSwapchainKHR(createInfo, nullptr);
    LE_CORE_INFO("Swapchain successfully created");

    m_SwapChainImageFormat = format.format;
    m_SwapChainExtent = swapExtent;
    m_SwapChainImages = ctx->device.getSwapchainImagesKHR(m_SwapChain);

    LE_CORE_INFO("Swapchain images acquired!");

    createSwapChainImageViews();
    createDepthResources();
}

void LE::SwapChain::createSwapChainImageViews() {

    m_SwapChainImageViews.resize(m_SwapChainImages.size());

    for (size_t i = 0; i < m_SwapChainImages.size(); i++) {
        vk::ImageViewCreateInfo createInfo{
            .image = m_SwapChainImages[i],
            .viewType = vk::ImageViewType::e2D,
            .format = m_SwapChainImageFormat,
            .components = {
                .r = vk::ComponentSwizzle::eIdentity,
                .g = vk::ComponentSwizzle::eIdentity,
                .b = vk::ComponentSwizzle::eIdentity,
                .a = vk::ComponentSwizzle::eIdentity,
            },
            .subresourceRange = {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0 ,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
                }
        };
        m_SwapChainImageViews[i] = ctx->device.createImageView(createInfo, nullptr);
    }
    LE_CORE_INFO("Swapchain ImageViews created!");
}
void LE::SwapChain::RecreateSwapChain(GLFWwindow *windowRef) {

    m_WindowRef = windowRef;
    m_OldSwapChain = m_SwapChain;
    for (auto imageView : m_SwapChainImageViews) {
        ctx->device.destroyImageView(imageView, nullptr);
    }
    Images::destroyImage(*ctx, m_DepthBuffer.image);

    CreateSwapChain(ctx,m_WindowRef);

    ctx->device.destroySwapchainKHR(m_OldSwapChain, nullptr);

}

void LE::SwapChain::createDepthResources() {

    m_DepthBuffer = Images::CreateDepthBuffer(*ctx, m_SwapChainExtent);
}


void LE::SwapChain::destroySwapChain() {

    for (auto imageView : m_SwapChainImageViews) {
        ctx->device.destroyImageView(imageView, nullptr);
    }
    Images::destroyImage(*ctx, m_DepthBuffer.image);
    ctx->device.destroySwapchainKHR(m_SwapChain, nullptr);


}
