//
// Created by Faith Kamaraju on 2026-01-21.
//

#include "FrameData.h"
#include "Rendering/RenderingConstants.h"
#include "Descriptors.h"

vk::DescriptorSetLayout LE::FrameData::sceneDescriptorLayout = nullptr;
vk::DescriptorSetLayout LE::FrameData::bindlessTexturesLayout = nullptr;
vk::DescriptorSetLayout LE::FrameData::bindlessSamplersLayout = nullptr;
uint32_t LE::FrameData::maxSets = 6;
std::vector<LE::PoolSize> LE::FrameData::sizes =
{
    { vk::DescriptorType::eUniformBuffer, MAX_FRAMES_IN_FLIGHT },
    { vk::DescriptorType::eSampledImage, MAX_FRAMES_IN_FLIGHT * MAX_BINDLESS_TEXTURES },
    { vk::DescriptorType::eSampler, MAX_FRAMES_IN_FLIGHT * MAX_BINDLESS_SAMPLERS },
};

void LE::FrameData::constructDescriptorSetLayouts(VulkanContext& ctx) {
    {
        DescriptorBuilder builder{};
        builder.AddBinding(0, vk::DescriptorType::eUniformBuffer, 1,
            vk::ShaderStageFlagBits::eVertex, nullptr );
        sceneDescriptorLayout =  builder.Build(&ctx,{},nullptr);
    }

    {
        DescriptorBuilder builder{};
        builder.AddBinding(0, vk::DescriptorType::eSampledImage, MAX_BINDLESS_TEXTURES ,vk::ShaderStageFlagBits::eFragment,
            nullptr );
        vk::DescriptorBindingFlags bindingFlags = vk::DescriptorBindingFlagBits::eUpdateAfterBind |
                                                vk::DescriptorBindingFlagBits::eUpdateUnusedWhilePending |
                                                vk::DescriptorBindingFlagBits::eVariableDescriptorCount |
                                                vk::DescriptorBindingFlagBits::ePartiallyBound;
        bindlessTexturesLayout = builder.Build(&ctx,vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool, &bindingFlags);
    }

    {
        DescriptorBuilder builder{};
        builder.AddBinding(0, vk::DescriptorType::eSampler, MAX_BINDLESS_SAMPLERS ,vk::ShaderStageFlagBits::eFragment,
            nullptr );
        vk::DescriptorBindingFlags bindingFlags = vk::DescriptorBindingFlagBits::eUpdateAfterBind |
                                                vk::DescriptorBindingFlagBits::eUpdateUnusedWhilePending |
                                                vk::DescriptorBindingFlagBits::eVariableDescriptorCount |
                                                vk::DescriptorBindingFlagBits::ePartiallyBound;
        bindlessSamplersLayout = builder.Build(&ctx,vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool, &bindingFlags);
    }
}

void LE::FrameData::initSyncObjects(VulkanContext &ctx) {

    vk::SemaphoreCreateInfo semaphoreInfo{};
    vk::FenceCreateInfo fenceInfo{ .flags = vk::FenceCreateFlagBits::eSignaled};
    inFlightFence = ctx.device.createFence(fenceInfo, nullptr);
    acquireSemaphore = ctx.device.createSemaphore(semaphoreInfo);

    vk::CommandPoolCreateInfo gpoolInfo{
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = ctx.queueFamilyIndices.graphicsFamily.value(),
    };

    graphicsCommandPool = ctx.device.createCommandPool(gpoolInfo, nullptr);

    vk::CommandBufferAllocateInfo bufInfo{
        .commandPool = graphicsCommandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1};
    graphicsCommandBuffer = ctx.device.allocateCommandBuffers(bufInfo)[0];
}


void LE::FrameData::cleanup(VulkanContext &ctx) const {

    ctx.device.destroySemaphore(acquireSemaphore, nullptr);
    ctx.device.destroyFence(inFlightFence, nullptr);
    ctx.device.destroyCommandPool(graphicsCommandPool, nullptr);
}
