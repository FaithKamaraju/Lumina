//
// Created by Faith Kamaraju on 2026-01-18.
//

#pragma once
#include "Rendering/Image.h"
#include <vulkan/vulkan.hpp>
#include <VmaUsage.h>
#include "VulkanContext.h"
#include "Core/LE_Types.h"

namespace LE {

    struct VulkanImage {

        VkImageView imageView = nullptr;
        VkImage image = nullptr;
        VmaAllocation allocation = nullptr;
        VmaAllocationInfo info{};
        vk::Extent3D extent{};
        vk::Format format = vk::Format::eUndefined ;

    };


    inline vk::Format ToVkFormat(ImageFormat fmt) {

        switch (fmt) {
            // ---- 8-bit formats ----
            case LE::ImageFormat::R8_UNorm:  return vk::Format::eR8Unorm;
            case LE::ImageFormat::R8_SNorm:  return vk::Format::eR8Snorm;
            case LE::ImageFormat::R8_UInt:  return vk::Format::eR8Uint;
            case LE::ImageFormat::R8_SInt:  return vk::Format::eR8Sint;
            case LE::ImageFormat::R8_Srgb:  return vk::Format::eR8Srgb;

            // ---- 16-bit formats ----

            case LE::ImageFormat::R16_UNorm:  return vk::Format::eR16Unorm ;
            case LE::ImageFormat::R16_SNorm:  return vk::Format::eR16Snorm ;
            case LE::ImageFormat::R16_UInt:  return vk::Format::eR16Uint ;
            case LE::ImageFormat::R16_SInt:  return vk::Format::eR16Sint ;
            case LE::ImageFormat::R16_Float:  return vk::Format::eR16Sfloat ;
            case LE::ImageFormat::R8G8_UNorm:  return vk::Format::eR8G8Unorm;
            case LE::ImageFormat::R8G8_SNorm:  return vk::Format::eR8G8Snorm;
            case LE::ImageFormat::R8G8_UInt:  return vk::Format::eR8G8Uint;
            case LE::ImageFormat::R8G8_SInt:  return vk::Format::eR8G8Sint;

            // ---- 24-bit formats ----

            case LE::ImageFormat::R8G8B8_UNorm:  return vk::Format::eR8G8B8Unorm;
            case LE::ImageFormat::R8G8B8_SNorm:  return vk::Format::eR8G8B8Snorm;
            case LE::ImageFormat::R8G8B8_UInt:  return vk::Format::eR8G8B8Uint;
            case LE::ImageFormat::R8G8B8_SInt:  return vk::Format::eR8G8B8Sint;
            case LE::ImageFormat::R8G8B8_SRGB:  return vk::Format::eR8G8B8Srgb;
            case LE::ImageFormat::B8G8R8_UNorm:  return vk::Format::eB8G8R8Unorm;
            case LE::ImageFormat::B8G8R8_SNorm:  return vk::Format::eB8G8R8Snorm;
            case LE::ImageFormat::B8G8R8_UInt:  return vk::Format::eB8G8R8Uint;
            case LE::ImageFormat::B8G8R8_SInt:  return vk::Format::eB8G8R8Sint;
            case LE::ImageFormat::B8G8R8_SRGB:  return vk::Format::eB8G8R8Srgb;

            // ---- 32-bit formats ----
            case LE::ImageFormat::R32_UInt:  return vk::Format::eR32Uint;
            case LE::ImageFormat::R32_SInt:  return vk::Format::eR32Sint;
            case LE::ImageFormat::R32_Float:  return vk::Format::eR32Sfloat;
            case LE::ImageFormat::R16G16_UNorm: return vk::Format::eR16G16Unorm;
            case LE::ImageFormat::R16G16_SNorm: return vk::Format::eR16G16Snorm;
            case LE::ImageFormat::R16G16_UInt: return vk::Format::eR16G16Uint;
            case LE::ImageFormat::R16G16_SInt: return vk::Format::eR16G16Sint;
            case LE::ImageFormat::R16G16_Float: return vk::Format::eR16G16Sfloat;
            case LE::ImageFormat::R8G8B8A8_UNorm: return vk::Format::eR8G8B8A8Unorm;
            case LE::ImageFormat::R8G8B8A8_SNorm: return vk::Format::eR8G8B8A8Snorm;
            case LE::ImageFormat::R8G8B8A8_UInt: return vk::Format::eR8G8B8A8Uint;
            case LE::ImageFormat::R8G8B8A8_SInt: return vk::Format::eR8G8B8A8Sint;
            case LE::ImageFormat::R8G8B8A8_SRGB: return vk::Format::eR8G8B8A8Srgb;
            case LE::ImageFormat::B8G8R8A8_UNorm: return vk::Format::eB8G8R8A8Unorm;
            case LE::ImageFormat::B8G8R8A8_SNorm: return vk::Format::eB8G8R8A8Snorm;
            case LE::ImageFormat::B8G8R8A8_UInt: return vk::Format::eB8G8R8A8Uint;
            case LE::ImageFormat::B8G8R8A8_SInt: return vk::Format::eB8G8R8A8Sint;
            case LE::ImageFormat::B8G8R8A8_SRGB: return vk::Format::eB8G8R8A8Srgb;

            // ---- high precision formats ----
            case LE::ImageFormat::R32G32B32A32_Float: return vk::Format::eR32G32B32A32Sfloat;
            case LE::ImageFormat::R16G16B16A16_Float: return vk::Format::eR16G16B16A16Sfloat;

            // ---- Depth/Stencil ----
            case LE::ImageFormat::D16_UNorm: return vk::Format::eD16Unorm;
            case LE::ImageFormat::D16_UNorm_S8_UInt: return vk::Format::eD16UnormS8Uint;
            case LE::ImageFormat::D24_UNorm_S8_UInt: return vk::Format::eD24UnormS8Uint;
            case LE::ImageFormat::D32_Float: return vk::Format::eD32Sfloat;
            case LE::ImageFormat::D32_Float_S8_UInt: return vk::Format::eD32SfloatS8Uint;

            default: return vk::Format::eUndefined;
        }
    }

    inline vk::ImageUsageFlags ToVkUsage(ImageUsageFlags usage) {

        vk::ImageUsageFlags flags{};

        if (Any(usage & ImageUsageFlags::TransferSrc))  flags |= vk::ImageUsageFlagBits::eTransferSrc;
        if (Any(usage & ImageUsageFlags::TransferDst))  flags |= vk::ImageUsageFlagBits::eTransferDst;

        if (Any(usage & ImageUsageFlags::ShaderRead))   flags |= vk::ImageUsageFlagBits::eSampled;
        if (Any(usage & ImageUsageFlags::ShaderWrite))  flags |= vk::ImageUsageFlagBits::eStorage;

        if (Any(usage & ImageUsageFlags::ColorAttachment)) flags |= vk::ImageUsageFlagBits::eColorAttachment;
        if (Any(usage & ImageUsageFlags::DepthStencilAttachment)) flags |= vk::ImageUsageFlagBits::eDepthStencilAttachment;

        if (Any(usage & ImageUsageFlags::Storage))       flags |= vk::ImageUsageFlagBits::eStorage;

        return flags;
    }

    inline vk::Extent3D ToVkExtent3D(ImageExtent3D extent) {
        return {extent.width, extent.height, extent.depth};
    }
    inline vk::Extent2D ToVkExtent2D(ImageExtent3D extent) {
        return {extent.width, extent.height};
    }

    struct DepthImage {
        VulkanImage image;
        bool bHasStencilComponent = false;
    };

    namespace Images {
        VulkanImage allocateImage(
            VulkanContext& ctx,
            const vk::ImageCreateInfo& imageInfo,
            VmaMemoryUsage vmaMemoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
            VmaAllocationCreateFlags vmaFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT);

        Scope<VulkanImage> allocateImageScoped(
            VulkanContext& ctx,
            const vk::ImageCreateInfo& imageInfo,
            VmaMemoryUsage vmaMemoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
            VmaAllocationCreateFlags vmaFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT);

        void copyData(VulkanContext& ctx, VulkanImage& image, void* data, size_t dataSize);
        void createImageView(VulkanContext& ctx, VulkanImage& image, vk::ImageAspectFlags imageAspectMask);
        void destroyImage(VulkanContext& ctx, VulkanImage& image);
        void transitionImageLayout(
                vk::CommandBuffer buffer,
                vk::Image image,
                vk::ImageLayout oldLayout,
                vk::ImageLayout newLayout,
                vk::AccessFlags2 srcAccessMask,
                vk::AccessFlags2 dstAccessMask,
                vk::PipelineStageFlags2 srcStageMask,
                vk::PipelineStageFlags2 dstStageMask,
                uint32_t srcQueueIndex = VK_QUEUE_FAMILY_IGNORED,
                uint32_t dstQueueIndex = VK_QUEUE_FAMILY_IGNORED);


        DepthImage CreateDepthBuffer(VulkanContext& ctx, vk::Extent2D swapChainExtent);
        vk::Format findSuitableDepthFormat(VulkanContext& ctx);

    }

}
