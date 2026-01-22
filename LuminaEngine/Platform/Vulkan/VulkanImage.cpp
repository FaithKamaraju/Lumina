//
// Created by Faith Kamaraju on 2026-01-18.
//

#include "VulkanImage.h"
#include "VulkanBuffer.h"
#include "VulkanContext.h"
#include "VulkanUtils.h"
#include "Core/Logger.h"


LE::VulkanImage LE::Images::allocateImage(VulkanContext& ctx, const vk::ImageCreateInfo &imageInfo,
    VmaMemoryUsage vmaMemoryUsage, VmaAllocationCreateFlags vmaFlags) {

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = vmaMemoryUsage;
    allocCreateInfo.flags = vmaFlags;

    VulkanImage newImage{};
    VkResult result = vmaCreateImage(ctx.vmaAllocator, &*imageInfo, &allocCreateInfo, &newImage.image, &newImage.allocation,
        &newImage.info);

    LE_ASSERT(result && "Couldn't allocate Image!");

    newImage.extent = imageInfo.extent;
    newImage.format = imageInfo.format;

    return newImage;
}

LE::Scope<LE::VulkanImage> LE::Images::allocateImageScoped(VulkanContext& ctx, const vk::ImageCreateInfo &imageInfo,
    VmaMemoryUsage vmaMemoryUsage, VmaAllocationCreateFlags vmaFlags) {

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = vmaMemoryUsage;
    allocCreateInfo.flags = vmaFlags;

    Scope<VulkanImage> newImage = CreateScope<VulkanImage>();
    VkResult result = vmaCreateImage(ctx.vmaAllocator, &*imageInfo, &allocCreateInfo, &newImage.get()->image, &newImage.get()->allocation,
        &newImage.get()->info);

    LE_ASSERT(result && "Couldn't allocate Image!");

    newImage.get()->extent = imageInfo.extent;
    newImage.get()->format = imageInfo.format;

    return newImage;
}


void LE::Images::createImageView(VulkanContext& ctx, VulkanImage &image,vk::ImageAspectFlags imageAspectMask) {

    vk::ImageViewCreateInfo createInfo{
            .image = image.image,
            .viewType = vk::ImageViewType::e2D,
            .format = image.format,
            .components = {
                .r = vk::ComponentSwizzle::eIdentity,
                .g = vk::ComponentSwizzle::eIdentity,
                .b = vk::ComponentSwizzle::eIdentity,
                .a = vk::ComponentSwizzle::eIdentity,
            },
            .subresourceRange = {
                .aspectMask = imageAspectMask,
                .baseMipLevel = 0 ,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
                }
        };
        image.imageView = ctx.device.createImageView(createInfo, nullptr);
}

void LE::Images::destroyImage(VulkanContext& ctx, VulkanImage &image) {

    ctx.device.destroyImageView(image.imageView);
    vmaDestroyImage(ctx.vmaAllocator, image.image, image.allocation);
}

void LE::Images::transitionImageLayout(
        vk::CommandBuffer buffer,
        vk::Image image,
        vk::ImageLayout oldLayout,
        vk::ImageLayout newLayout,
        vk::AccessFlags2 srcAccessMask,
        vk::AccessFlags2 dstAccessMask,
        vk::PipelineStageFlags2 srcStageMask,
        vk::PipelineStageFlags2 dstStageMask,
        uint32_t srcQueueIndex,
        uint32_t dstQueueIndex)
{
    vk::ImageMemoryBarrier2 barrier = {
        .srcStageMask = srcStageMask,
        .srcAccessMask = srcAccessMask,
        .dstStageMask = dstStageMask,
        .dstAccessMask = dstAccessMask,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = srcQueueIndex,
        .dstQueueFamilyIndex = dstQueueIndex,
        .image = image,
        .subresourceRange = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };
    vk::DependencyInfo dependencyInfo = {
        .dependencyFlags = {},
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrier
    };
    buffer.pipelineBarrier2(dependencyInfo);
}

void LE::Images::copyData(VulkanContext& ctx, VulkanImage &image, void *data, size_t dataSize) {

    vk::CommandBufferAllocateInfo allocInfo{ .commandPool = ctx.transferCommandPool,
                                             .level = vk::CommandBufferLevel::ePrimary,
                                             .commandBufferCount = 1 };
    vk::CommandBuffer cmdBuf = ctx.device.allocateCommandBuffers(allocInfo).front();
    cmdBuf.begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
    VkMemoryPropertyFlags memPropFlags;
    vmaGetAllocationMemoryProperties(ctx.vmaAllocator, image.allocation, &memPropFlags);

    vk::BufferCreateInfo stagingInfo{
        .size = dataSize,
        .usage = vk::BufferUsageFlagBits::eTransferSrc,
        .sharingMode = ctx.queueIndicesArr.size() > 1 ?  vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive,
        .queueFamilyIndexCount = static_cast<uint32_t>(ctx.queueIndicesArr.size()),
        .pQueueFamilyIndices = ctx.queueIndicesArr.data()
    };
    VulkanBuffer staging = Buffers::allocateBuffer(ctx, stagingInfo);
    vmaCopyMemoryToAllocation(ctx.vmaAllocator, data,
        staging.allocation, 0, dataSize);

    transitionImageLayout(
        cmdBuf,
        image.image,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eTransferDstOptimal,
        {},
        vk::AccessFlagBits2::eMemoryWrite,
        {},
        vk::PipelineStageFlagBits2::eTransfer
        );

    vk::BufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = image.extent;

    cmdBuf.copyBufferToImage(staging.buffer, image.image,vk::ImageLayout::eTransferDstOptimal, {region});

    if (ctx.bUnifiedGraphicsAndTransferQueues) {
        transitionImageLayout(
        cmdBuf,
        image.image,
        vk::ImageLayout::eTransferDstOptimal,
        vk::ImageLayout::eReadOnlyOptimal,
        vk::AccessFlagBits2::eTransferWrite,
        vk::AccessFlagBits2::eShaderRead,
        vk::PipelineStageFlagBits2::eTransfer,
        vk::PipelineStageFlagBits2::eFragmentShader
        );
        cmdBuf.end();
        ctx.transferQueue.submit(vk::SubmitInfo{ .commandBufferCount = 1, .pCommandBuffers = &cmdBuf },
            nullptr);
        ctx.transferQueue.waitIdle();
        Buffers::destroyBuffer(ctx, staging);
    }
    else {
        transitionImageLayout(
        cmdBuf,
        image.image,
        vk::ImageLayout::eTransferDstOptimal,
        vk::ImageLayout::eReadOnlyOptimal,
        vk::AccessFlagBits2::eTransferWrite,
        {},
        vk::PipelineStageFlagBits2::eTransfer,
        {},
        ctx.queueFamilyIndices.transferFamily.value(),
        ctx.queueFamilyIndices.graphicsFamily.value()
        );
        cmdBuf.end();
        ctx.transferQueue.submit(vk::SubmitInfo{ .commandBufferCount = 1, .pCommandBuffers = &cmdBuf },
            nullptr);

        vk::CommandBufferAllocateInfo bufInfo{
            .commandPool = ctx.globalGraphicsCmdPool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = 1};
        vk::CommandBuffer graphicsCmdBuf = ctx.device.allocateCommandBuffers(bufInfo).front();
        graphicsCmdBuf.begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
        transitionImageLayout(
                graphicsCmdBuf,
                image.image,
                vk::ImageLayout::eTransferDstOptimal,
                vk::ImageLayout::eReadOnlyOptimal,
                {},
                vk::AccessFlagBits2::eShaderRead,
                {},
                vk::PipelineStageFlagBits2::eFragmentShader,
                ctx.queueFamilyIndices.transferFamily.value(),
                ctx.queueFamilyIndices.graphicsFamily.value()
                );
        graphicsCmdBuf.end();
        ctx.graphicsQueue.submit(vk::SubmitInfo{ .commandBufferCount = 1, .pCommandBuffers = &cmdBuf },
            nullptr);


        Buffers::destroyBuffer(ctx, staging);
    }


}

vk::Format LE::Images::findSuitableDepthFormat(VulkanContext& ctx) {
    return Utils::findSupportedFormat(ctx.primaryPhysicalDevice,
    {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
        vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment
    );
}

LE::DepthImage LE::Images::CreateDepthBuffer(VulkanContext& ctx, vk::Extent2D swapChainExtent) {

    DepthImage depthImg{};
    vk::Format depthImageFormat = findSuitableDepthFormat(ctx);

    vk::ImageCreateInfo depthImageInfo{};
    depthImageInfo.imageType = vk::ImageType::e2D;
    depthImageInfo.extent.width = swapChainExtent.width;
    depthImageInfo.extent.height = swapChainExtent.height;
    depthImageInfo.extent.depth = 1;
    depthImageInfo.mipLevels = 1;
    depthImageInfo.arrayLayers = 1;
    depthImageInfo.format = depthImageFormat;
    depthImageInfo.tiling = vk::ImageTiling::eOptimal;
    depthImageInfo.initialLayout = vk::ImageLayout::eUndefined;
    depthImageInfo.samples = vk::SampleCountFlagBits::e1;
    depthImageInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
    depthImageInfo.sharingMode = vk::SharingMode::eExclusive;
    depthImageInfo.queueFamilyIndexCount = static_cast<uint32_t>(ctx.queueIndicesArr.size());
    depthImageInfo.pQueueFamilyIndices = ctx.queueIndicesArr.data();

    depthImg.image = allocateImage(ctx, depthImageInfo, VMA_MEMORY_USAGE_GPU_ONLY, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);

    createImageView(ctx, depthImg.image, vk::ImageAspectFlagBits::eDepth);
    depthImg.bHasStencilComponent = (depthImageFormat == vk::Format::eD32SfloatS8Uint || depthImageFormat == vk::Format::eD24UnormS8Uint);

    return depthImg;
}
