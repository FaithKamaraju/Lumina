//
// Created by Faith Kamaraju on 2026-01-18.
//

#include "VulkanBuffer.h"
#include "Core/Logger.h"

LE::VulkanBuffer LE::Buffers::allocateBuffer(
    VulkanContext* ctx,
    const vk::BufferCreateInfo& bufferInfo,
    VmaMemoryUsage vmaMemoryUsage,
    VmaAllocationCreateFlags vmaFlags) {

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = vmaMemoryUsage;
    allocCreateInfo.flags = vmaFlags ;

    VulkanBuffer newBuffer{};
    VkResult result = vmaCreateBuffer(ctx->vmaAllocator, &*bufferInfo, &allocCreateInfo, &newBuffer.buffer, &newBuffer.allocation,
        &newBuffer.info);

    // LE_ASSERT(result && "Couldn't allocate buffer!");

    return newBuffer;

}

LE::Scope<LE::VulkanBuffer> LE::Buffers::allocateBufferScoped(
    VulkanContext* ctx,
    const vk::BufferCreateInfo& bufferInfo,
    VmaMemoryUsage vmaMemoryUsage,
    VmaAllocationCreateFlags vmaFlags) {

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = vmaMemoryUsage;
    allocCreateInfo.flags = vmaFlags ;

    Scope<VulkanBuffer> newBuffer = CreateScope<VulkanBuffer>();
    VkResult result = vmaCreateBuffer(ctx->vmaAllocator, &*bufferInfo, &allocCreateInfo, &newBuffer.get()->buffer, &newBuffer.get()->allocation,
        &newBuffer.get()->info);

    // LE_ASSERT(result && "Couldn't allocate buffer!");

    return newBuffer;

}

void LE::Buffers::copyData(VulkanContext* ctx, VulkanBuffer &buffer, void *data, size_t dataSize) {

    vk::CommandBufferAllocateInfo allocInfo{ .commandPool = ctx->transferCommandPool,
                                             .level = vk::CommandBufferLevel::ePrimary,
                                             .commandBufferCount = 1 };
    vk::CommandBuffer cmdBuf = ctx->device.allocateCommandBuffers(allocInfo).front();
    cmdBuf.begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
    VkMemoryPropertyFlags memPropFlags;
    vmaGetAllocationMemoryProperties(ctx->vmaAllocator, buffer.allocation, &memPropFlags);

    if(memPropFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
    {
        vmaCopyMemoryToAllocation(ctx->vmaAllocator, data, buffer.allocation,
            0, dataSize);

    }
    else
    {
        vk::BufferCreateInfo stagingInfo{
            .size = dataSize,
            .usage = vk::BufferUsageFlagBits::eTransferSrc,
            .sharingMode = ctx->queueIndicesArr.size() > 1 ?  vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive,
            .queueFamilyIndexCount = static_cast<uint32_t>(ctx->queueIndicesArr.size()),
            .pQueueFamilyIndices = ctx->queueIndicesArr.data()
        };
        VulkanBuffer staging = allocateBuffer(ctx, stagingInfo);
        vmaCopyMemoryToAllocation(ctx->vmaAllocator, data,
            staging.allocation, 0, dataSize);

        cmdBuf.copyBuffer(staging.buffer, buffer.buffer, vk::BufferCopy(0, 0,
            dataSize));

        if (ctx->bUnifiedGraphicsAndTransferQueues) {
            placeBufferMemoryBarrier(
            cmdBuf,
            vk::AccessFlagBits2::eMemoryWrite,
            vk::AccessFlagBits2::eMemoryRead,
            vk::PipelineStageFlagBits2::eTransfer,
            vk::PipelineStageFlagBits2::eVertexShader,
            buffer.buffer,
            0,
            VK_WHOLE_SIZE);
            cmdBuf.end();
            ctx->transferQueue.submit(vk::SubmitInfo{ .commandBufferCount = 1, .pCommandBuffers = &cmdBuf },
                nullptr);
            ctx->transferQueue.waitIdle();
        }
        else {
            placeBufferMemoryBarrier(
            cmdBuf,
            vk::AccessFlagBits2::eMemoryWrite,
            {},
            vk::PipelineStageFlagBits2::eTransfer,
            {},
            buffer.buffer,
            0,
            VK_WHOLE_SIZE,
            ctx->queueFamilyIndices.transferFamily.value(),
            ctx->queueFamilyIndices.graphicsFamily.value());
            cmdBuf.end();
            ctx->transferQueue.submit(vk::SubmitInfo{ .commandBufferCount = 1, .pCommandBuffers = &cmdBuf },
                nullptr);

            vk::CommandBufferAllocateInfo bufInfo{
                .commandPool = ctx->globalGraphicsCmdPool,
                .level = vk::CommandBufferLevel::ePrimary,
                .commandBufferCount = 1};
            vk::CommandBuffer graphicsCmdBuf = ctx->device.allocateCommandBuffers(bufInfo).front();
            graphicsCmdBuf.begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
            placeBufferMemoryBarrier(
            cmdBuf,
            {},
            vk::AccessFlagBits2::eMemoryRead,
            {},
            vk::PipelineStageFlagBits2::eVertexShader,
            buffer.buffer,
            0,
            VK_WHOLE_SIZE,
            ctx->queueFamilyIndices.transferFamily.value(),
            ctx->queueFamilyIndices.graphicsFamily.value());
            graphicsCmdBuf.end();
            ctx->graphicsQueue.submit(vk::SubmitInfo{ .commandBufferCount = 1, .pCommandBuffers = &cmdBuf },
                nullptr);
        }

        destroyBuffer(ctx, staging);
    }

}
void LE::Buffers::placeBufferMemoryBarrier(
        vk::CommandBuffer& cmdbuf,
        vk::AccessFlags2 srcAccessMask,
        vk::AccessFlags2 dstAccessMask,
        vk::PipelineStageFlags2 srcStageMask,
        vk::PipelineStageFlags2 dstStageMask,
        VkBuffer& buffer,
        vk::DeviceSize offset,
        vk::DeviceSize size,
        uint32_t srcQueueIndex,
        uint32_t dstQueueIndex)
{
    vk::BufferMemoryBarrier2 barrier {
        .srcStageMask = srcStageMask,
        .srcAccessMask = srcAccessMask,
        .dstStageMask = dstStageMask,
        .dstAccessMask = dstAccessMask,
        .srcQueueFamilyIndex = srcQueueIndex,
        .dstQueueFamilyIndex = dstQueueIndex,
        .buffer = buffer,
        .offset =  offset,
        .size = size,
    };

    vk::DependencyInfo dependencyInfo = {
        .dependencyFlags = {},
        .bufferMemoryBarrierCount = 1,
        .pBufferMemoryBarriers = &barrier,
    };
    cmdbuf.pipelineBarrier2(dependencyInfo);
}


void LE::Buffers::destroyBuffer(VulkanContext* ctx, const VulkanBuffer &buffer) {

    vmaDestroyBuffer(ctx->vmaAllocator, buffer.buffer, buffer.allocation);

}

