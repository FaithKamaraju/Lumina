//
// Created by Faith Kamaraju on 2026-01-21.
//

#pragma once
#include "VulkanBuffer.h"

namespace LE {

    struct FrameData {

        vk::Fence           inFlightFence =                 nullptr;
        vk::Semaphore       acquireSemaphore =              nullptr;
        vk::CommandPool     graphicsCommandPool =           nullptr;
        vk::CommandBuffer   graphicsCommandBuffer =         nullptr;
        vk::DescriptorSet   sceneDescriptorSet =            nullptr;
        vk::DescriptorSet   bindlessTexturesDescriptorSet = nullptr;
        vk::DescriptorSet   bindlessSamplersDescriptorSet = nullptr;
        vk::DescriptorSet   materialDataDescriptorSet =     nullptr;
        BufferHandle        cameraAndSceneBufferHandle{};
        BufferHandle        materialBufferHandle{};

        void initSyncObjects(VulkanContext* ctx);
        void cleanup(VulkanContext* ctx) const;

    };




}
