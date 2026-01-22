//
// Created by Faith Kamaraju on 2026-01-21.
//

#pragma once
#include <glm/glm.hpp>
#include "VulkanBuffer.h"

namespace LE {

    struct PerObjectConstants {
        glm::mat4 model;
        int albedoIndex;
        uint64_t bufferAddress;
    };

    struct SceneUniformBuffer {
        glm::mat4 view;
        glm::mat4 proj;
        // glm::mat4 viewProj;
        glm::vec3 cameraPosition;
        float time;
    };

    struct FrameData {

        static vk::DescriptorSetLayout sceneDescriptorLayout;
        static vk::DescriptorSetLayout bindlessTexturesLayout;
        static vk::DescriptorSetLayout bindlessSamplersLayout;
        static uint32_t maxSets;
        static std::vector<PoolSize> sizes;

        vk::Fence inFlightFence = nullptr;
        vk::Semaphore acquireSemaphore = nullptr;
        vk::CommandPool graphicsCommandPool = nullptr;
        vk::CommandBuffer  graphicsCommandBuffer = nullptr;
        vk::DescriptorSet sceneDescriptorSet = nullptr;
        vk::DescriptorSet bindlessTexturesDescriptorSet = nullptr;
        vk::DescriptorSet bindlessSamplersDescriptorSet = nullptr;
        BufferHandle uniformBuffer;

        static void constructDescriptorSetLayouts(VulkanContext& ctx);

        void initSyncObjects(VulkanContext& ctx);
        void cleanup(VulkanContext& ctx) const;

    };




}
