//
// Created by Faith Kamaraju on 2026-01-18.
//

#pragma once
#include "Rendering/Buffer.h"
#include <vulkan/vulkan.hpp>
#include <VmaUsage.h>
#include <Core/LE_Types.h>
#include "VulkanContext.h"


namespace LE {

    struct VulkanBuffer{
        VkBuffer buffer = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;
        VmaAllocationInfo info{};
    };



    namespace Buffers {

        VulkanBuffer allocateBuffer(VulkanContext& ctx,
            const vk::BufferCreateInfo& bufferInfo,
            VmaMemoryUsage vmaMemoryUsage = VMA_MEMORY_USAGE_AUTO,
            VmaAllocationCreateFlags vmaFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT |
                                            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                                            VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT);
        Scope<VulkanBuffer> allocateBufferScoped(VulkanContext& ctx,
            const vk::BufferCreateInfo& bufferInfo,
            VmaMemoryUsage vmaMemoryUsage = VMA_MEMORY_USAGE_AUTO,
            VmaAllocationCreateFlags vmaFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT |
                                            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                                            VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT);
        void copyData(VulkanContext& ctx, VulkanBuffer& buffer, void* data, size_t dataSize);
        void placeBufferMemoryBarrier(
                    vk::CommandBuffer& cmdbuf,
                    vk::AccessFlags2 srcAccessMask,
                    vk::AccessFlags2 dstAccessMask,
                    vk::PipelineStageFlags2 srcStageMask,
                    vk::PipelineStageFlags2 dstStageMask,
                    VkBuffer& buffer,
                    vk::DeviceSize offset,
                    vk::DeviceSize size,
                    uint32_t srcQueueIndex = VK_QUEUE_FAMILY_IGNORED,
                    uint32_t dstQueueIndex = VK_QUEUE_FAMILY_IGNORED);
        void destroyBuffer(VulkanContext& ctx, const VulkanBuffer& buffer);




        // Buffer allocateVertexBuffer(vk::Device device,const std::vector<Vertex>& vertices);
        // Buffer allocateIndexBuffer(vk::Device device,const std::vector<uint32_t>& indices);
        // GPUMeshBuffers UploadMesh(vk::Device device,const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
        // void destroyGPUMeshBuffer(vk::Device device, const GPUMeshBuffers& meshBuf);

    }

}
