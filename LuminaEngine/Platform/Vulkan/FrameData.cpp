//
// Created by Faith Kamaraju on 2026-01-21.
//

#include "FrameData.h"
#include "Descriptors.h"

void LE::FrameData::initSyncObjects(VulkanContext* ctx) {

    vk::SemaphoreCreateInfo semaphoreInfo{};
    vk::FenceCreateInfo fenceInfo{ .flags = vk::FenceCreateFlagBits::eSignaled};
    inFlightFence = ctx->device.createFence(fenceInfo, nullptr);
    acquireSemaphore = ctx->device.createSemaphore(semaphoreInfo);

    vk::CommandPoolCreateInfo gpoolInfo{
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = ctx->queueFamilyIndices.graphicsFamily.value(),
    };

    graphicsCommandPool = ctx->device.createCommandPool(gpoolInfo, nullptr);

    vk::CommandBufferAllocateInfo bufInfo{
        .commandPool = graphicsCommandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1};
    graphicsCommandBuffer = ctx->device.allocateCommandBuffers(bufInfo)[0];
}


void LE::FrameData::cleanup(VulkanContext* ctx) const {

    ctx->device.destroySemaphore(acquireSemaphore, nullptr);
    ctx->device.destroyFence(inFlightFence, nullptr);
    ctx->device.destroyCommandPool(graphicsCommandPool, nullptr);
}
