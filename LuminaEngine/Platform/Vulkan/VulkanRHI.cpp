//
// Created by Faith Kamaraju on 2026-01-18.
//

#include "VulkanRHI.h"
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include "Platform/Vulkan/VulkanUtils.h"
#include "Core/Events/EventManager.h"
#include "Core/Events/WindowEvent.h"
#include "VulkanBuffer.h"
#include "VulkanSampler.h"
#include "Rendering/RenderingConstants.h"
#include "PerRenderableConstants.h"
#include "Rendering/GPUMaterialData.h"
#include "Rendering/SceneData.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE


LE::VulkanRHI::VulkanRHI() {

    //Init image vectors
    m_Images.reserve(MAX_BINDLESS_TEXTURES);
    m_ImagesFreeList.reserve(MAX_BINDLESS_TEXTURES);
    // Reserve index 0 for a default white texture
    // Reserve index 1 for a default grey texture
    // Reserve index 2 for a default black texture
    // Reserve index 3 for a default error checkerboard texture
    // Reserve index 4 for a default Imgui Font texture

    m_Buffers.reserve(MIN_BUFFER_VECTOR_SIZE);
    m_BuffersFreeList.reserve(MIN_BUFFER_VECTOR_SIZE);

    m_Samplers.reserve(MAX_BINDLESS_SAMPLERS);
    m_SamplersFreeList.reserve(MAX_BINDLESS_SAMPLERS);

    m_ShaderModules.reserve(256);
    m_ShaderModulesFreeList.reserve(256);
}

LE::VulkanRHI::~VulkanRHI() {

    vkCtx->device.waitIdle();
    for (auto& image : m_Images) {
        Images::destroyImage(vkCtx,image.image);
    }
    for (auto& buffer : m_Buffers) {
        Buffers::destroyBuffer(vkCtx, buffer.buffer);
    }
    for (auto& sampler : m_Samplers) {
        vkCtx->device.destroySampler(sampler.sampler);
    }
    for (auto& module : m_ShaderModules) {
        vkCtx->device.destroyShaderModule(module.shaderModule);
    }
    for (auto& pipeline : m_Pipelines) {
        vkCtx->device.destroyPipeline(pipeline.pipeline);
    }

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        perFrameData[i].cleanup(vkCtx);
    }
    vkCtx->device.destroyDescriptorSetLayout(sceneDescriptorLayout, nullptr);
    vkCtx->device.destroyDescriptorSetLayout(bindlessTexturesLayout, nullptr);
    vkCtx->device.destroyDescriptorSetLayout(bindlessSamplersLayout, nullptr);
    vkCtx->device.destroyDescriptorSetLayout(materialDataDescriptorLayout, nullptr);
    vkCtx->device.destroyPipelineLayout(m_PipelineLayout);
    vkCtx->descriptorAllocator.DestroyPool(vkCtx);
    vkCtx->device.destroyCommandPool(vkCtx->transferCommandPool);
    vkCtx->device.destroyCommandPool(vkCtx->globalGraphicsCmdPool);

    for (int i = 0; i < swapChain->m_SwapChainImageViews.size(); i++) {
        vkCtx->device.destroySemaphore(submitSemaphores[i], nullptr);
    }
    swapChain->destroySwapChain();
    delete swapChain;
    vmaDestroyAllocator(vkCtx->vmaAllocator);
    vkCtx->device.destroy(nullptr);
    vkCtx->instance.destroySurfaceKHR(vkCtx->surface, nullptr);
    if (Utils::enableValidationLayers) {
        vkCtx->instance.destroyDebugUtilsMessengerEXT(vkCtx->debugMessenger, nullptr);
    }
    vkCtx->instance.destroy(nullptr);
    delete vkCtx;
}

LEBool LE::VulkanRHI::InitDevice(GLFWwindow* windowHandle) {

    m_WindowRef = windowHandle;
    vkCtx = new VulkanContext();
    vkCtx->InitVulkanInstance();
    vkCtx->AcquireSurface(windowHandle);
    vkCtx->PickPhysicalDevice();

    std::set queueIndices = {vkCtx->queueFamilyIndices.graphicsFamily.value(), vkCtx->queueFamilyIndices.transferFamily.value()};
    if (queueIndices.size() > 1) {
        vkCtx->queueIndicesArr.emplace_back(vkCtx->queueFamilyIndices.transferFamily.value());
        vkCtx->queueIndicesArr.emplace_back(vkCtx->queueFamilyIndices.graphicsFamily.value());
    }
    else if (queueIndices.size() == 1) {
        vkCtx->queueIndicesArr.emplace_back(vkCtx->queueFamilyIndices.transferFamily.value());
    }

    vkCtx->CreateLogicalDevice();
    vkCtx->CreateVmaAllocator();

    swapChain = new SwapChain();
    swapChain->CreateSwapChain(vkCtx, windowHandle);

    initSubmitSemaphores();
    vkCtx->InitPools();
    vkCtx->descriptorAllocator.InitPool(vkCtx, maxSets, sizes, vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind);
    createPerFrameData();
    constructPipelineLayout();

    LE::Events::Subscribe<LE::WindowFramebufferResizeEvent>([this](const LE::WindowFramebufferResizeEvent &e) {
           swapChain->m_FramebufferResized = true;
           LE_CORE_INFO("new framebuffer size {0} x {1}", e.width, e.height);
       });

    UpdateCameraAndSceneData();

    return LE_SUCCESS;
}

LEBool LE::VulkanRHI::InitImgui() {

    return LE_SUCCESS;
}


void LE::VulkanRHI::UpdateMaterialDataSSBOs(PBRMaterialGPUData data, size_t index) {

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        UpdateStorageBufferData(perFrameData[i].materialBufferHandle, &data,
            sizeof(PBRMaterialGPUData), sizeof(PBRMaterialGPUData) * index);
    }

}

void LE::VulkanRHI::UpdateCameraAndSceneData() {
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        CameraAndSceneData ubo{};
        ubo.view = glm::lookAt(glm::vec3(0.0f, 0.f, 1.3f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f),
            swapChain->m_SwapChainExtent.width / (float) swapChain->m_SwapChainExtent.height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;
        // ubo.time = timestep;
        UpdateUniformBufferData(perFrameData[i].cameraAndSceneBufferHandle, &ubo, sizeof(ubo), 0);
    }

}


LE::BufferHandle LE::VulkanRHI::AllocateAndCopyBufferFromBytes(const std::byte *dataSource, size_t dataSize, BufferUsageFlags usage) {

    vk::BufferCreateInfo bufferInfo{
        .size = dataSize,
        .usage = ToVkBufferUsageFlags(usage) | vk::BufferUsageFlagBits::eTransferDst,
        .sharingMode = vkCtx->queueIndicesArr.size() > 1 ?  vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive,
        .queueFamilyIndexCount = static_cast<uint32_t>(vkCtx->queueIndicesArr.size()),
        .pQueueFamilyIndices = vkCtx->queueIndicesArr.data()
    };
    VulkanBuffer buf = Buffers::allocateBuffer(vkCtx, bufferInfo, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
    Buffers::copyData(vkCtx, buf, (void*)dataSource, dataSize);

    return bindBuffer(buf);
}

LE::BufferHandle LE::VulkanRHI::AllocateAndCopyVertexBuffer(const std::vector<Vertex> &vertices) {

    vk::BufferCreateInfo bufferInfo{
        .size = vertices.size() * sizeof(vertices[0]),
        .usage = vk::BufferUsageFlagBits::eVertexBuffer |
            vk::BufferUsageFlagBits::eShaderDeviceAddress |
            vk::BufferUsageFlagBits::eTransferDst,
        .sharingMode = vkCtx->queueIndicesArr.size() > 1 ?  vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive,
        .queueFamilyIndexCount = static_cast<uint32_t>(vkCtx->queueIndicesArr.size()),
        .pQueueFamilyIndices = vkCtx->queueIndicesArr.data()
    };
    VulkanBuffer buf = Buffers::allocateBuffer(vkCtx, bufferInfo, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
    Buffers::copyData(vkCtx, buf, (void*)vertices.data(), vertices.size()*sizeof(vertices[0]));

    vk::BufferDeviceAddressInfo deviceAdressInfo{.buffer = buf.buffer };
    buf.bufAddress = vkCtx->device.getBufferAddress(&deviceAdressInfo);

    return bindBuffer(buf);

}

LE::BufferHandle LE::VulkanRHI::AllocateAndCopyIndexBuffer(const std::vector<uint32_t> &indices) {

    vk::BufferCreateInfo bufferInfo{
        .size = indices.size() * sizeof(indices[0]),
        .usage = vk::BufferUsageFlagBits::eIndexBuffer |
            vk::BufferUsageFlagBits::eTransferDst,
        .sharingMode = vkCtx->queueIndicesArr.size() > 1 ?  vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive,
        .queueFamilyIndexCount = static_cast<uint32_t>(vkCtx->queueIndicesArr.size()),
        .pQueueFamilyIndices = vkCtx->queueIndicesArr.data()
    };

    VulkanBuffer buf = Buffers::allocateBuffer(vkCtx, bufferInfo, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
    Buffers::copyData(vkCtx, buf, (void*)indices.data(), indices.size() * sizeof(indices[0]));

    return bindBuffer(buf);
}

LE::BufferHandle LE::VulkanRHI::AllocateUniformBuffer(size_t uniformBufferSize) {
    vk::BufferCreateInfo bufferInfo{
        .size = uniformBufferSize,
        .usage = vk::BufferUsageFlagBits::eUniformBuffer |
            vk::BufferUsageFlagBits::eTransferDst,
        .sharingMode = vkCtx->queueIndicesArr.size() > 1 ?  vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive,
        .queueFamilyIndexCount = static_cast<uint32_t>(vkCtx->queueIndicesArr.size()),
        .pQueueFamilyIndices = vkCtx->queueIndicesArr.data()
    };

    VulkanBuffer buf = Buffers::allocateBuffer(vkCtx, bufferInfo, VMA_MEMORY_USAGE_AUTO,VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
    VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT |
    VMA_ALLOCATION_CREATE_MAPPED_BIT);

    return bindBuffer(buf);
}

LE::BufferHandle LE::VulkanRHI::AllocateStorageBuffer(size_t storageBufferSize) {
    vk::BufferCreateInfo bufferInfo{
        .size = storageBufferSize,
        .usage = vk::BufferUsageFlagBits::eStorageBuffer |
            vk::BufferUsageFlagBits::eTransferDst,
        .sharingMode = vkCtx->queueIndicesArr.size() > 1 ?  vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive,
        .queueFamilyIndexCount = static_cast<uint32_t>(vkCtx->queueIndicesArr.size()),
        .pQueueFamilyIndices = vkCtx->queueIndicesArr.data()
    };

    VulkanBuffer buf = Buffers::allocateBuffer(vkCtx, bufferInfo, VMA_MEMORY_USAGE_AUTO,VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
    VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT |
    VMA_ALLOCATION_CREATE_MAPPED_BIT);

    return bindBuffer(buf);
}

void LE::VulkanRHI::UpdateUniformBufferData(BufferHandle ub, void *uniformData, size_t dataSize, size_t offset) {
    vk::CommandBufferAllocateInfo allocInfo{ .commandPool = vkCtx->transferCommandPool,
                                             .level = vk::CommandBufferLevel::ePrimary,
                                             .commandBufferCount = 1 };
    vk::CommandBuffer cmdBuf = vkCtx->device.allocateCommandBuffers(allocInfo).front();
    cmdBuf.begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

    VkMemoryPropertyFlags memPropFlags;
    vmaGetAllocationMemoryProperties(vkCtx->vmaAllocator, m_Buffers[ub.id].buffer.allocation, &memPropFlags);

    if(memPropFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
    {
        vmaCopyMemoryToAllocation(vkCtx->vmaAllocator,uniformData, m_Buffers[ub.id].buffer.allocation,
            offset, dataSize );
        Buffers::placeBufferMemoryBarrier(
                   cmdBuf,
                   vk::AccessFlagBits2::eHostWrite,
                   vk::AccessFlagBits2::eUniformRead,
                   vk::PipelineStageFlagBits2::eHost,
                   vk::PipelineStageFlagBits2::eVertexShader|vk::PipelineStageFlagBits2::eFragmentShader,
                   m_Buffers[ub.id].buffer.buffer,
                   offset,
                   dataSize);
        cmdBuf.end();
        vkCtx->transferQueue.submit(vk::SubmitInfo{ .commandBufferCount = 1, .pCommandBuffers = &cmdBuf },
            nullptr);
        vkCtx->transferQueue.waitIdle();
    }
    else
    {
        vk::BufferCreateInfo stagingInfo{
            .size = dataSize,
            .usage = vk::BufferUsageFlagBits::eTransferSrc,
            .sharingMode = vkCtx->queueIndicesArr.size() > 1 ?  vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive,
            .queueFamilyIndexCount = static_cast<uint32_t>(vkCtx->queueIndicesArr.size()),
            .pQueueFamilyIndices = vkCtx->queueIndicesArr.data()
        };
        VulkanBuffer staging = Buffers::allocateBuffer(vkCtx, stagingInfo);
        vmaCopyMemoryToAllocation(vkCtx->vmaAllocator, uniformData,
            staging.allocation, 0, dataSize);
        Buffers::placeBufferMemoryBarrier(
                           cmdBuf,
                           vk::AccessFlagBits2::eHostWrite,
                           vk::AccessFlagBits2::eTransferRead,
                           vk::PipelineStageFlagBits2::eHost,
                           vk::PipelineStageFlagBits2::eTransfer,
                           staging.buffer,
                           0,
                           VK_WHOLE_SIZE);
        cmdBuf.copyBuffer(staging.buffer, m_Buffers[ub.id].buffer.buffer, vk::BufferCopy(0, offset,
            dataSize));
        Buffers::placeBufferMemoryBarrier(
               cmdBuf,
               vk::AccessFlagBits2::eTransferWrite,
               vk::AccessFlagBits2::eUniformRead,
               vk::PipelineStageFlagBits2::eHost,
               vk::PipelineStageFlagBits2::eVertexShader | vk::PipelineStageFlagBits2::eFragmentShader,
               m_Buffers[ub.id].buffer.buffer,
               offset,
               dataSize);
         cmdBuf.end();
         vkCtx->transferQueue.submit(vk::SubmitInfo{ .commandBufferCount = 1, .pCommandBuffers = &cmdBuf },
             nullptr);
         vkCtx->transferQueue.waitIdle();

        vmaDestroyBuffer(vkCtx->vmaAllocator, staging.buffer, staging.allocation);

    }
}

void LE::VulkanRHI::UpdateStorageBufferData(BufferHandle ub, void *storageData, size_t dataSize, size_t offset) {

    vk::CommandBufferAllocateInfo allocInfo{ .commandPool = vkCtx->transferCommandPool,
                                             .level = vk::CommandBufferLevel::ePrimary,
                                             .commandBufferCount = 1 };
    vk::CommandBuffer cmdBuf = vkCtx->device.allocateCommandBuffers(allocInfo).front();
    cmdBuf.begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

    VkMemoryPropertyFlags memPropFlags;
    vmaGetAllocationMemoryProperties(vkCtx->vmaAllocator, m_Buffers[ub.id].buffer.allocation, &memPropFlags);

    if(memPropFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
    {
        vmaCopyMemoryToAllocation(vkCtx->vmaAllocator,storageData, m_Buffers[ub.id].buffer.allocation,
            offset, dataSize );
        Buffers::placeBufferMemoryBarrier(
                   cmdBuf,
                   vk::AccessFlagBits2::eHostWrite,
                   vk::AccessFlagBits2::eShaderStorageRead,
                   vk::PipelineStageFlagBits2::eHost,
                   vk::PipelineStageFlagBits2::eVertexShader|vk::PipelineStageFlagBits2::eFragmentShader,
                   m_Buffers[ub.id].buffer.buffer,
                   offset,
                   dataSize);
        cmdBuf.end();
        vkCtx->transferQueue.submit(vk::SubmitInfo{ .commandBufferCount = 1, .pCommandBuffers = &cmdBuf },
            nullptr);
        vkCtx->transferQueue.waitIdle();
    }
    else
    {
        vk::BufferCreateInfo stagingInfo{
            .size = dataSize,
            .usage = vk::BufferUsageFlagBits::eTransferSrc,
            .sharingMode = vkCtx->queueIndicesArr.size() > 1 ?  vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive,
            .queueFamilyIndexCount = static_cast<uint32_t>(vkCtx->queueIndicesArr.size()),
            .pQueueFamilyIndices = vkCtx->queueIndicesArr.data()
        };
        VulkanBuffer staging = Buffers::allocateBuffer(vkCtx, stagingInfo);
        vmaCopyMemoryToAllocation(vkCtx->vmaAllocator, storageData,
            staging.allocation, 0, dataSize);
        Buffers::placeBufferMemoryBarrier(
                           cmdBuf,
                           vk::AccessFlagBits2::eHostWrite,
                           vk::AccessFlagBits2::eTransferRead,
                           vk::PipelineStageFlagBits2::eHost,
                           vk::PipelineStageFlagBits2::eTransfer,
                           staging.buffer,
                           0,
                           VK_WHOLE_SIZE);
        cmdBuf.copyBuffer(staging.buffer, m_Buffers[ub.id].buffer.buffer,
            vk::BufferCopy(0, offset, dataSize));
        Buffers::placeBufferMemoryBarrier(
               cmdBuf,
               vk::AccessFlagBits2::eTransferWrite,
               vk::AccessFlagBits2::eShaderStorageRead,
               vk::PipelineStageFlagBits2::eHost,
               vk::PipelineStageFlagBits2::eVertexShader | vk::PipelineStageFlagBits2::eFragmentShader,
               m_Buffers[ub.id].buffer.buffer,
               offset,
               dataSize);
         cmdBuf.end();
         vkCtx->transferQueue.submit(vk::SubmitInfo{ .commandBufferCount = 1, .pCommandBuffers = &cmdBuf },
             nullptr);
         vkCtx->transferQueue.waitIdle();

        vmaDestroyBuffer(vkCtx->vmaAllocator, staging.buffer, staging.allocation);

    }

}

LE::ImageHandle LE::VulkanRHI::CreateImage(uint64_t hashID, unsigned char *data, uint32_t nrChannels, ImageExtent3D imagesize, ImageFormat format,
                                           ImageUsageFlags imageUsage, bool mipmapped) {

    for (uint32_t i = 0; i < m_Images.size(); i++) {
        if (hashID == m_Images[i].hashID) {
            return {.id = static_cast<int32_t>(i), .generation = m_Images[i].generation};
        }
    }

    VulkanImage texture{};
    vk::ImageCreateInfo imageInfo{};
    imageInfo.imageType = vk::ImageType::e2D;
    imageInfo.extent = ToVkExtent3D(imagesize);
    imageInfo.mipLevels =
        mipmapped ? static_cast<uint32_t>(std::floor(std::log2(std::max(imagesize.width, imagesize.height)))) + 1 : 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = ToVkFormat(format);
    imageInfo.tiling = vk::ImageTiling::eOptimal;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;
    imageInfo.samples = vk::SampleCountFlagBits::e1;
    imageInfo.usage = vk::ImageUsageFlagBits::eTransferDst | ToVkImageUsage(imageUsage);
    imageInfo.sharingMode = vkCtx->queueIndicesArr.size() > 1 ?  vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive;
    imageInfo.queueFamilyIndexCount = static_cast<uint32_t>(vkCtx->queueIndicesArr.size());
    imageInfo.pQueueFamilyIndices = vkCtx->queueIndicesArr.data();

    texture = Images::allocateImage(vkCtx, imageInfo);
    Images::copyData(vkCtx, texture, data, imagesize.width * imagesize.height * nrChannels);
    Images::createImageView(vkCtx, texture, vk::ImageAspectFlagBits::eColor);

    return bindImage(texture, hashID);
}

void LE::VulkanRHI::IncrementImageRef(ImageHandle handle) {
    m_Images[handle.id].numTextureRefs += 1;
}

void LE::VulkanRHI::DecrementImageRef(ImageHandle handle) {
    m_Images[handle.id].numTextureRefs -= 1;
    if (m_Images[handle.id].numTextureRefs <= 0) {
        DestroyImage(handle);
    }
}

void LE::VulkanRHI::IncrementSamplerRef(SamplerHandle handle) {
    m_Samplers[handle.id].numTextureRefs += 1;
}

void LE::VulkanRHI::DecrementSamplerRef(SamplerHandle handle) {
    m_Samplers[handle.id].numTextureRefs -= 1;
    if (m_Samplers[handle.id].numTextureRefs <= 0) {
        DestroyImageSampler(handle);
    }
}

void LE::VulkanRHI::IncrementShaderModuleRef(ShaderHandle handle) {
    m_ShaderModules[handle.id].numMaterialRefs += 1;
}

void LE::VulkanRHI::DecrementShaderModuleRef(ShaderHandle handle) {
    m_ShaderModules[handle.id].numMaterialRefs -= 1;
    if (m_ShaderModules[handle.id].numMaterialRefs <= 0) {
        // DestroyShaderModule(handle);
        m_ShaderModules[handle.id].numMaterialRefs = 0;
    }
}

LE::SamplerHandle LE::VulkanRHI::CreateImageSampler(SamplerKey samplerInfo) {

    for (uint32_t i = 0; i < m_Samplers.size(); i++) {
        if (samplerInfo.hashID == m_Samplers[i].hashID) {
            return {.id = (int32_t)i, .generation = m_Samplers[i].generation};
        }
    }
    vk::PhysicalDeviceProperties properties = vkCtx->primaryPhysicalDevice.getProperties();
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    vk::SamplerCreateInfo info = ToVulkanSamplerCreateInfo(samplerInfo);
    vk::Sampler samp = vkCtx->device.createSampler(info);

    return bindSampler(samp, samplerInfo.hashID);
}

LE::ShaderHandle LE::VulkanRHI::CreateShaderModule(uint64_t hashID, const std::vector<uint32_t> &spvBinary) {

    for (uint32_t i = 0; i < m_ShaderModules.size(); i++) {
        if (hashID == m_ShaderModules[i].hashID) {
            return {.id = static_cast<int32_t>(i), .generation = m_ShaderModules[i].generation};
        }
    }
    vk::ShaderModuleCreateInfo createInfo{};
    createInfo.codeSize = spvBinary.size() * sizeof(uint32_t);
    createInfo.pCode = spvBinary.data();

    vk::ShaderModule module = vkCtx->device.createShaderModule(createInfo);
    return bindShaderModule(module, hashID);
}



void LE::VulkanRHI::DeallocateBuffer(BufferHandle handle) {
    unbindBuffer(handle, perFrameData[currentFrame].inFlightFence);
}

void LE::VulkanRHI::DestroyImage(ImageHandle handle) {
    unbindImage(handle, perFrameData[currentFrame].inFlightFence);
}

void LE::VulkanRHI::DestroyImageSampler(SamplerHandle samplerHandle) {
    unbindSampler(samplerHandle, perFrameData[currentFrame].inFlightFence);
}

void LE::VulkanRHI::DestroyShaderModule(ShaderHandle shaderHandle) {
    unbindShaderModule(shaderHandle, perFrameData[currentFrame].inFlightFence);
}

LE::PipelineHandle LE::VulkanRHI::CreateGraphicsPipeline(const GraphicsPipelineDesc &desc) {

    vk::PipelineShaderStageCreateInfo vertShaderStageInfo{
        .stage = vk::ShaderStageFlagBits::eVertex,
        .module = m_ShaderModules[desc.vertexHandle.id].shaderModule,
        .pName = "main",
    };
    vk::PipelineShaderStageCreateInfo fragShaderStageInfo{
        .stage = vk::ShaderStageFlagBits::eFragment ,
        .module = m_ShaderModules[desc.fragmentHandle.id].shaderModule,
        .pName = "main",
    };
    vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo{
        .dynamicStateCount = static_cast<uint32_t>(LE::DYNAMIC_STATES.size()),
        .pDynamicStates = LE::DYNAMIC_STATES.data(),
    };

    vk::PipelineVertexInputStateCreateInfo vertexInputCreateInfo{
        .vertexBindingDescriptionCount = 0,
        .pVertexBindingDescriptions = nullptr,
        .vertexAttributeDescriptionCount = 0,
        .pVertexAttributeDescriptions = nullptr,
    };
    vk::PipelineInputAssemblyStateCreateInfo inputAsmCreateInfo{
        .pNext = nullptr,
        .topology = GetVKTopology(desc.topology),
        .primitiveRestartEnable = VK_FALSE,
    };

    vk::PipelineViewportStateCreateInfo viewportState{
        .viewportCount = 1,
        .scissorCount = 1,
    };
    vk::PipelineRasterizationStateCreateInfo rasterizer{
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = vk::PolygonMode::eFill,
        .cullMode = vk::CullModeFlagBits::eNone,
        // .cullMode = GetVKCullMode(desc.cullMode),
        .frontFace = vk::FrontFace::eClockwise,
        .depthBiasEnable = VK_FALSE,
        .lineWidth = 1.0f,
    };
    vk::PipelineMultisampleStateCreateInfo multisampling{
        .rasterizationSamples = vk::SampleCountFlagBits::e1,
        .sampleShadingEnable = VK_FALSE,
    };
    vk::PipelineDepthStencilStateCreateInfo depthStencil{
        .depthTestEnable       = vk::True,
        .depthWriteEnable      = vk::True,
        .depthCompareOp        = vk::CompareOp::eLess,
        .depthBoundsTestEnable = vk::False,
        .stencilTestEnable     = vk::False}
    ;
    vk::PipelineColorBlendAttachmentState colorBlendAttachment{
        .blendEnable = vk::True,
        .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
        .dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
        .colorBlendOp = vk::BlendOp::eAdd,
        .srcAlphaBlendFactor = vk::BlendFactor::eOne,
        .dstAlphaBlendFactor = vk::BlendFactor::eZero,
        .alphaBlendOp = vk::BlendOp::eAdd,
        .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,
    };
    vk::PipelineColorBlendStateCreateInfo colorBlending{
        .logicOpEnable = VK_FALSE,
        .logicOp = vk::LogicOp::eCopy,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
    };
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo{
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &swapChain->m_SwapChainImageFormat,
        .depthAttachmentFormat = swapChain->m_DepthBuffer.image.format,
        };

    vk::GraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputCreateInfo;
    pipelineInfo.pInputAssemblyState = &inputAsmCreateInfo;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.layout = m_PipelineLayout;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicStateCreateInfo;
    pipelineInfo.pNext = &pipelineRenderingCreateInfo;

    auto pipeline = vkCtx->device.createGraphicsPipeline(nullptr, pipelineInfo, nullptr).value;

    return bindPipeline(pipeline);

}

LE::PipelineHandle LE::VulkanRHI::CreateComputePipeline(const ComputePipelineDesc &desc) {
    return {};
}


void LE::VulkanRHI::DrawFrame(const std::vector<Renderable>& renderables, float timestep) {

    auto _ = vkCtx->device.waitForFences(perFrameData[currentFrame].inFlightFence, vk::True, UINT64_MAX);

    auto result = vkCtx->device.acquireNextImageKHR(swapChain->m_SwapChain, UINT64_MAX, perFrameData[currentFrame].acquireSemaphore,
        VK_NULL_HANDLE );
    if (result.result == vk::Result::eErrorOutOfDateKHR || result.result == vk::Result::eSuboptimalKHR || swapChain->m_FramebufferResized)
    {
        swapChain->m_FramebufferResized = false;
        recreateSwapChainAndPerFrameData(m_WindowRef);
        return;
    }
    if (result.result != vk::Result::eSuccess) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    vkCtx->device.resetFences(perFrameData[currentFrame].inFlightFence);
    perFrameData[currentFrame].graphicsCommandBuffer.reset({});
    recordCommands(renderables, currentFrame, result.value, timestep);


    // updateSceneData(currentFrame, timestep);

    vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput};
    vk::SubmitInfo submitInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &perFrameData[currentFrame].acquireSemaphore,
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &perFrameData[currentFrame].graphicsCommandBuffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &submitSemaphores[result.value],
    };

    vkCtx->graphicsQueue.submit(submitInfo, perFrameData[currentFrame].inFlightFence);

    vk::PresentInfoKHR presentInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &submitSemaphores[result.value],
        .swapchainCount = 1,
        .pSwapchains = &swapChain->m_SwapChain,
        .pImageIndices = &result.value,
        .pResults = nullptr,
    };


    vk::Result presentResult = vkCtx->presentQueue.presentKHR(presentInfo);
    if (presentResult == vk::Result::eErrorOutOfDateKHR || presentResult == vk::Result::eSuboptimalKHR  || swapChain->m_FramebufferResized)
    {
        swapChain->m_FramebufferResized = false;
        recreateSwapChainAndPerFrameData(m_WindowRef);
    }
    else if (presentResult != vk::Result::eSuccess) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    currentFrame  = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

}


void LE::VulkanRHI::ProcessDeferredFrees() {
    processDeferredFrees_Images();
    processDeferredFrees_Buffer();
    processDeferredFrees_Sampler();
    processDeferredFrees_ShaderModules();
}



// private

void LE::VulkanRHI::initSubmitSemaphores() {
    for (int i = 0; i < swapChain->m_SwapChainImages.size(); i++) {
        vk::SemaphoreCreateInfo semaphoreInfo{};
        submitSemaphores[i] = vkCtx->device.createSemaphore(semaphoreInfo);
    }
}

void LE::VulkanRHI::constructDescriptorSetLayouts() {
    {
        DescriptorBuilder builder{};
        builder.AddBinding(0, vk::DescriptorType::eUniformBuffer, 1,
            vk::ShaderStageFlagBits::eVertex, nullptr );
        sceneDescriptorLayout =  builder.Build(vkCtx,{},nullptr);
    }

    {
        DescriptorBuilder builder{};
        builder.AddBinding(0, vk::DescriptorType::eSampledImage, MAX_BINDLESS_TEXTURES ,vk::ShaderStageFlagBits::eFragment,
            nullptr );
        vk::DescriptorBindingFlags bindingFlags = vk::DescriptorBindingFlagBits::eUpdateAfterBind |
                                                vk::DescriptorBindingFlagBits::eUpdateUnusedWhilePending |
                                                vk::DescriptorBindingFlagBits::eVariableDescriptorCount |
                                                vk::DescriptorBindingFlagBits::ePartiallyBound;
        bindlessTexturesLayout = builder.Build(vkCtx,vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool, &bindingFlags);
    }

    {
        DescriptorBuilder builder{};
        builder.AddBinding(0, vk::DescriptorType::eSampler, MAX_BINDLESS_SAMPLERS ,vk::ShaderStageFlagBits::eFragment,
            nullptr );
        vk::DescriptorBindingFlags bindingFlags = vk::DescriptorBindingFlagBits::eUpdateAfterBind |
                                                vk::DescriptorBindingFlagBits::eUpdateUnusedWhilePending |
                                                vk::DescriptorBindingFlagBits::eVariableDescriptorCount |
                                                vk::DescriptorBindingFlagBits::ePartiallyBound;
        bindlessSamplersLayout = builder.Build(vkCtx,vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool, &bindingFlags);
    }

    {
        DescriptorBuilder builder{};
        builder.AddBinding(0, vk::DescriptorType::eStorageBuffer, 1 ,
            vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
            nullptr );
        materialDataDescriptorLayout = builder.Build(vkCtx,{}, nullptr);
    }
}

void LE::VulkanRHI::createPerFrameData() {

    constructDescriptorSetLayouts();
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {

        perFrameData[i] = {};
        perFrameData[i].initSyncObjects(vkCtx);

        perFrameData[i].cameraAndSceneBufferHandle = AllocateUniformBuffer(sizeof(CameraAndSceneData));
        perFrameData[i].materialBufferHandle = AllocateStorageBuffer(sizeof(PBRMaterialGPUData) * MAX_PBR_MATERIAL_INSTANCES);

        perFrameData[i].sceneDescriptorSet = allocateDescriptor(sceneDescriptorLayout);
        writeUniformBuffer(perFrameData[i].cameraAndSceneBufferHandle, sizeof(CameraAndSceneData),
            perFrameData[i].sceneDescriptorSet);

        perFrameData[i].bindlessTexturesDescriptorSet = allocateVariableCountDescriptor(MAX_BINDLESS_TEXTURES,
            bindlessTexturesLayout);

        perFrameData[i].bindlessSamplersDescriptorSet = allocateVariableCountDescriptor(MAX_BINDLESS_SAMPLERS,
            bindlessSamplersLayout);

        perFrameData[i].materialDataDescriptorSet = allocateDescriptor(materialDataDescriptorLayout);
        writeStorageBuffer(perFrameData[i].materialBufferHandle,
            sizeof(PBRMaterialGPUData) * MAX_PBR_MATERIAL_INSTANCES,
            perFrameData[i].materialDataDescriptorSet);

    }
}

void LE::VulkanRHI::constructPipelineLayout() {

    vk::PushConstantRange pushConstant{};
    pushConstant.offset = 0;
    pushConstant.size = sizeof(PerRenderableConstants);
    pushConstant.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;


    std::array setLayouts = {sceneDescriptorLayout, bindlessTexturesLayout,
        bindlessSamplersLayout, materialDataDescriptorLayout};
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
        .setLayoutCount = setLayouts.size(),
        .pSetLayouts = setLayouts.data(),
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &pushConstant,
    };
    if (!m_PipelineLayout) m_PipelineLayout = vkCtx->device.createPipelineLayout(pipelineLayoutInfo,nullptr);

}


void LE::VulkanRHI::recreateSwapChainAndPerFrameData(GLFWwindow *windowRef) {

    vkCtx->device.waitIdle();

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        perFrameData[i].cleanup(vkCtx);
    }
    for (int i = 0; i < swapChain->m_SwapChainImageViews.size(); i++) {
        vkCtx->device.destroySemaphore(submitSemaphores[i], nullptr);
    }

    vkCtx->device.destroyCommandPool(vkCtx->transferCommandPool);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        perFrameData[i].initSyncObjects(vkCtx);

    }
    swapChain->RecreateSwapChain(windowRef);
    initSubmitSemaphores();
    vkCtx->InitPools();
}

void LE::VulkanRHI::recordCommands(const std::vector<Renderable>& renderables, uint32_t frameIndex, uint32_t imageIndex, float timestep) const {
    vk::CommandBufferBeginInfo beginInfo{};
    vk::Result result = perFrameData[frameIndex].graphicsCommandBuffer.begin(&beginInfo);
    assert((result == vk::Result::eSuccess) && "Command recording couldn't begin!");

    Images::transitionImageLayout(
        perFrameData[frameIndex].graphicsCommandBuffer,
        swapChain->m_SwapChainImages[imageIndex],
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        {},
        vk::AccessFlagBits2::eColorAttachmentWrite,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::ImageAspectFlagBits::eColor
    );

    Images::transitionImageLayout(
        perFrameData[frameIndex].graphicsCommandBuffer,
        swapChain->m_DepthBuffer.image.image,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eDepthAttachmentOptimal,
        vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
        vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
        vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
        vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
        vk::ImageAspectFlagBits::eDepth
    );

    vk::ClearValue clearColor = {{0.0f, 0.0f, 0.0f, 1.0f}};
    vk::ClearDepthStencilValue clearDepthStencil = {1.f, 0};
    vk::RenderingAttachmentInfo attachmentInfo = {
        .imageView = swapChain->m_SwapChainImageViews[imageIndex],
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = clearColor
    };
    vk::RenderingAttachmentInfo depthAttachInfo = {
        .imageView = swapChain->m_DepthBuffer.image.imageView,
        .imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eDontCare,
        .clearValue = clearDepthStencil
    };

    vk::Viewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChain->m_SwapChainExtent.width);
    viewport.height = static_cast<float>(swapChain->m_SwapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    perFrameData[frameIndex].graphicsCommandBuffer.setViewport(0, viewport);

    vk::Rect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChain->m_SwapChainExtent;
    perFrameData[frameIndex].graphicsCommandBuffer.setScissor(0, scissor);

    // Set up the rendering info
    vk::RenderingInfo renderingInfo = {
        .renderArea = { .offset = { 0, 0 },
        .extent = swapChain->m_SwapChainExtent },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachmentInfo,
        .pDepthAttachment = &depthAttachInfo,
    };

    perFrameData[frameIndex].graphicsCommandBuffer.beginRendering(renderingInfo);



    std::array descSets = {
        perFrameData[frameIndex].sceneDescriptorSet,
        perFrameData[frameIndex].bindlessTexturesDescriptorSet,
        perFrameData[frameIndex].bindlessSamplersDescriptorSet,
        perFrameData[frameIndex].materialDataDescriptorSet
    };
    perFrameData[frameIndex].graphicsCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_PipelineLayout, 0,
        descSets.size(), descSets.data(), 0, nullptr);

    for (auto& renderable : renderables) {
        perFrameData[frameIndex].graphicsCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipelines[renderable.pipelineHandle.id].pipeline);
        perFrameData[frameIndex].graphicsCommandBuffer.bindIndexBuffer(m_Buffers[renderable.indexBufferHandle.id].buffer.buffer, 0, vk::IndexType::eUint32);
        PerRenderableConstants constants{};
        constants.modelMatrix = glm::mat4(1.0f);
        constants.modelMatrix = glm::translate(constants.modelMatrix, {0, 0, -5.f});
        constants.modelMatrix = glm::rotate(constants.modelMatrix, glm::radians(180.f), glm::vec3(0.0f, 0.f,1.f));
        constants.modelMatrix = glm::rotate(constants.modelMatrix, timestep * glm::radians(60.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        constants.materialIndex = renderable.materialIndex;
        constants.vertexBufferAddress = m_Buffers[renderable.vertexBufferHandle.id].buffer.bufAddress;
        perFrameData[frameIndex].graphicsCommandBuffer.pushConstants(m_PipelineLayout,
             vk::ShaderStageFlagBits::eFragment|vk::ShaderStageFlagBits::eVertex, 0, sizeof(PerRenderableConstants),&constants);

        perFrameData[frameIndex].graphicsCommandBuffer.drawIndexed(renderable.indexCount,
            1, renderable.indexOffset, 0, 0);
    }

    // ImDrawData* draw_data = ImGui::GetDrawData();
    // const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
    // if (!is_minimized)
    // {
    //     VkDescriptorSet descSetsImgui[2] = {
    //         perFrameData[frameIndex].sceneDescriptorSet, perFrameData[frameIndex].bindlessTexturesDescriptorSet
    //     };
    //     ImGui_ImplVulkan_RenderDrawData(draw_data, perFrameData[frameIndex].graphicsCommandBuffer,
    //     descSetsImgui, nullptr);
    // }

    perFrameData[frameIndex].graphicsCommandBuffer.endRendering();

    Images::transitionImageLayout(
            perFrameData[frameIndex].graphicsCommandBuffer,
            swapChain->m_SwapChainImages[imageIndex],
            vk::ImageLayout::eColorAttachmentOptimal,
            vk::ImageLayout::ePresentSrcKHR,
            vk::AccessFlagBits2::eColorAttachmentWrite,                // srcAccessMask
            {},                                                        // dstAccessMask
            vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // srcStage
            vk::PipelineStageFlagBits2::eBottomOfPipe,                  // dstStage
            vk::ImageAspectFlagBits::eColor
    );

    perFrameData[frameIndex].graphicsCommandBuffer.end();

}




// Descriptor Handling section

    // Descriptor Allocation code
vk::DescriptorSet LE::VulkanRHI::allocateDescriptor(vk::DescriptorSetLayout layout) {
    return vkCtx->descriptorAllocator.Allocate(vkCtx,layout, nullptr);
}

vk::DescriptorSet LE::VulkanRHI::allocateVariableCountDescriptor(uint32_t descriptorCount, vk::DescriptorSetLayout layout) {

    vk::DescriptorSetVariableDescriptorCountAllocateInfoEXT variableDescriptorCountAllocInfo = {};
    variableDescriptorCountAllocInfo.descriptorSetCount = 1;
    variableDescriptorCountAllocInfo.pDescriptorCounts  = &descriptorCount;

    return vkCtx->descriptorAllocator.Allocate(vkCtx, layout, &variableDescriptorCountAllocInfo);
}

    // Descriptor Writing Code
void LE::VulkanRHI::writeBindlessImage(ImageHandle imageHandle, vk::ImageLayout imageLayout) const {

    vk::DescriptorImageInfo imageInfo{
        .sampler = VK_NULL_HANDLE,
        .imageView = m_Images[imageHandle.id].image.imageView,
        .imageLayout = imageLayout};
    vk::WriteDescriptorSet imageDescriptorWrite{};
    imageDescriptorWrite.dstBinding = 0;
    imageDescriptorWrite.dstArrayElement = imageHandle.id;
    imageDescriptorWrite.descriptorType = vk::DescriptorType::eSampledImage;
    imageDescriptorWrite.descriptorCount = 1;
    imageDescriptorWrite.pImageInfo = &imageInfo;
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        imageDescriptorWrite.dstSet = perFrameData[i].bindlessTexturesDescriptorSet;
        vkCtx->device.updateDescriptorSets(imageDescriptorWrite, nullptr);
    }
}

void LE::VulkanRHI::writeBindlessSampler(const SamplerHandle samplerHandle) const {

    vk::DescriptorImageInfo info{
        .sampler = m_Samplers[samplerHandle.id].sampler,
        .imageView = VK_NULL_HANDLE,
        .imageLayout = vk::ImageLayout::eUndefined
    };
    vk::WriteDescriptorSet samplerDescriptorWrite{};
    samplerDescriptorWrite.dstBinding = 0;
    samplerDescriptorWrite.dstArrayElement = samplerHandle.id;
    samplerDescriptorWrite.descriptorType = vk::DescriptorType::eSampler;
    samplerDescriptorWrite.descriptorCount = 1;
    samplerDescriptorWrite.pImageInfo = &info;
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        samplerDescriptorWrite.dstSet = perFrameData[i].bindlessSamplersDescriptorSet;
        vkCtx->device.updateDescriptorSets(samplerDescriptorWrite, nullptr);
    }
}

void LE::VulkanRHI::writeUniformBuffer(BufferHandle bufferHandle, size_t ub_size, vk::DescriptorSet desc_set) const {

    vk::DescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = m_Buffers[bufferHandle.id].buffer.buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = ub_size;
    vk::WriteDescriptorSet uboDescriptorWrite{};
    uboDescriptorWrite.dstBinding = 0;
    uboDescriptorWrite.dstSet = desc_set;
    uboDescriptorWrite.dstArrayElement = 0;
    uboDescriptorWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
    uboDescriptorWrite.descriptorCount = 1;
    uboDescriptorWrite.pBufferInfo = &bufferInfo;
    vkCtx->device.updateDescriptorSets(uboDescriptorWrite,nullptr);
}

void LE::VulkanRHI::writeStorageBuffer(BufferHandle bufferHandle, size_t sb_size, vk::DescriptorSet desc_set) const {

    vk::DescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = m_Buffers[bufferHandle.id].buffer.buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sb_size;
    vk::WriteDescriptorSet ssboDescriptorWrite{};
    ssboDescriptorWrite.dstBinding = 0;
    ssboDescriptorWrite.dstSet = desc_set;
    ssboDescriptorWrite.dstArrayElement = 0;
    ssboDescriptorWrite.descriptorType = vk::DescriptorType::eStorageBuffer;
    ssboDescriptorWrite.descriptorCount = 1;
    ssboDescriptorWrite.pBufferInfo = &bufferInfo;
    vkCtx->device.updateDescriptorSets(ssboDescriptorWrite,nullptr);

}


// Image Handling section

LE::ImageHandle LE::VulkanRHI::bindImage(const VulkanImage &image, uint64_t hashID) {

    uint32_t idx = allocImageIndex(image, hashID);
    m_Images[idx].generation++;

    ImageHandle handle{.id = (int32_t)idx, .generation = m_Images[idx].generation};
    writeBindlessImage(handle, vk::ImageLayout::eShaderReadOnlyOptimal);
    return handle;
}

void LE::VulkanRHI::unbindImage(ImageHandle handle, VkFence fenceForCurrentFrame) {
    if (!isAlive(handle)) return;
    m_ImagesPendingFrees.push_back({handle.id, handle.generation,fenceForCurrentFrame});

}

uint32_t LE::VulkanRHI::allocImageIndex(const VulkanImage& image, uint64_t hashID) {
    uint32_t idx{};
    if (!m_ImagesFreeList.empty())
    {
        idx = m_ImagesFreeList.back();
        m_ImagesFreeList.pop_back();
        m_Images[idx] = {image, hashID};
    }
    else {
        assert(m_Images.size() <= MAX_BINDLESS_TEXTURES && "Max number of images reached! Cannot allocate anymore!");
        idx = static_cast<uint32_t>(m_Images.size());
        m_Images.emplace_back(image, hashID, 0, 0);
    }
    return idx;
}

bool LE::VulkanRHI::isAlive(ImageHandle handle) const {
    return (handle.generation == m_Images[handle.id].generation);
}

void LE::VulkanRHI::freeImageIndex(uint32_t idx) {
    m_ImagesFreeList.push_back(idx);
}

void LE::VulkanRHI::processDeferredFrees_Images() {
    for (size_t i = 0; i < m_ImagesPendingFrees.size();)
    {
        PendingFree& pf = m_ImagesPendingFrees[i];
        // Non-blocking check
        VkResult status = vkGetFenceStatus(vkCtx->device, pf.fence);
        if (status == VK_SUCCESS)
        {
            Images::destroyImage(vkCtx, m_Images[pf.index].image);
            m_Images[pf.index].image = VulkanImage{};
            m_Images[pf.index].hashID= 0;

            // Invalidate old handles
            m_Images[pf.index].generation++;
            // Clear descriptor (recommended)
            writeBindlessImage({},vk::ImageLayout::eShaderReadOnlyOptimal);
            freeImageIndex(pf.index);
            m_ImagesPendingFrees[i] = m_ImagesPendingFrees.back();
            m_ImagesPendingFrees.pop_back();
        }
        else
        {
            ++i;
        }
    }
}

//Start Region

LE::BufferHandle LE::VulkanRHI::bindBuffer(const VulkanBuffer &buf) {
    uint32_t idx = allocBufferIndex(buf);
    m_Buffers[idx].generation++;

    return {.id = (int32_t)idx, .generation = m_Buffers[idx].generation};
}

void LE::VulkanRHI::unbindBuffer(BufferHandle handle, VkFence fenceForCurrentFrame) {
    if (!isAlive(handle)) return;
    m_BuffersPendingFrees.push_back({handle.id, handle.generation,fenceForCurrentFrame});

}

uint32_t LE::VulkanRHI::allocBufferIndex(const VulkanBuffer &buf) {
    uint32_t idx{};
    if (!m_BuffersFreeList.empty())
    {
        idx = m_BuffersFreeList.back();
        m_BuffersFreeList.pop_back();
        m_Buffers[idx] = {buf};
    }
    else {
        idx = static_cast<uint32_t>(m_Buffers.size());
        m_Buffers.emplace_back(buf, 0);
    }
    return idx;
}

bool LE::VulkanRHI::isAlive(BufferHandle handle) const {
    return (handle.generation == m_Buffers[handle.id].generation);
}

void LE::VulkanRHI::freeBufferIndex(uint32_t idx) {
    m_BuffersFreeList.push_back(idx);
}

void LE::VulkanRHI::processDeferredFrees_Buffer() {
    for (size_t i = 0; i < m_BuffersPendingFrees.size();)
    {
        PendingFree& pf = m_BuffersPendingFrees[i];
        // Non-blocking check
        VkResult status = vkGetFenceStatus(vkCtx->device, pf.fence);
        if (status == VK_SUCCESS)
        {
            Buffers::destroyBuffer(vkCtx, m_Buffers[pf.index].buffer);
            m_Buffers[pf.index].buffer = VulkanBuffer{};
            // Invalidate old handles
            m_Buffers[pf.index].generation++;
            freeBufferIndex(pf.index);
            m_BuffersPendingFrees[i] = m_BuffersPendingFrees.back();
            m_BuffersPendingFrees.pop_back();
        }
        else{
            ++i;
        }
    }
}

// Samplers section

LE::SamplerHandle LE::VulkanRHI::bindSampler(const vk::Sampler &sampler, size_t hashID) {
    uint32_t idx = allocSamplerIndex(sampler, hashID);
    m_Samplers[idx].generation++;

    SamplerHandle handle{.id = (int32_t)idx, .generation = m_Samplers[idx].generation};
    writeBindlessSampler(handle);
    return handle;
}

void LE::VulkanRHI::unbindSampler(SamplerHandle handle, VkFence fenceForCurrentFrame) {
    if (!isAlive(handle)) return;
    m_SamplersPendingFrees.push_back({handle.id, handle.generation,fenceForCurrentFrame});
}


uint32_t LE::VulkanRHI::allocSamplerIndex(const vk::Sampler &sampler, size_t hashID) {
    uint32_t idx{};
    if (!m_SamplersFreeList.empty())
    {
        idx = m_SamplersFreeList.back();
        m_SamplersFreeList.pop_back();
        m_Samplers[idx] = {sampler, hashID};
    }
    else {
        idx = static_cast<uint32_t>(m_Samplers.size());
        m_Samplers.emplace_back(sampler, hashID, 0, 0);
    }
    return idx;
}

bool LE::VulkanRHI::isAlive(SamplerHandle handle) const {
    return (handle.generation == m_Samplers[handle.id].generation);
}

void LE::VulkanRHI::freeSamplerIndex(uint32_t idx) {
    m_SamplersFreeList.push_back(idx);
}

void LE::VulkanRHI::processDeferredFrees_Sampler() {
    for (size_t i = 0; i < m_SamplersPendingFrees.size();)
    {
        PendingFree& pf = m_SamplersPendingFrees[i];
        // Non-blocking check
        VkResult status = vkGetFenceStatus(vkCtx->device, pf.fence);
        if (status == VK_SUCCESS)
        {
            vkCtx->device.destroySampler(m_Samplers[pf.index].sampler);
            m_Samplers[pf.index].sampler = VK_NULL_HANDLE;
            m_Samplers[pf.index].hashID = 0;

            // Invalidate old handles
            m_Samplers[pf.index].generation++;
            freeSamplerIndex(pf.index);
            m_SamplersPendingFrees[i] = m_SamplersPendingFrees.back();
            m_SamplersPendingFrees.pop_back();
        }
        else{
            ++i;
        }
    }
}

// Shader module Section

LE::ShaderHandle LE::VulkanRHI::bindShaderModule(const vk::ShaderModule &module, uint64_t hashID) {
    uint32_t idx = allocShaderModuleIndex(module, hashID);
    m_ShaderModules[idx].generation++;

    ShaderHandle handle{.id = static_cast<int32_t>(idx), .generation = m_ShaderModules[idx].generation};
    return handle;
}

void LE::VulkanRHI::unbindShaderModule(ShaderHandle handle, VkFence fenceForCurrentFrame) {
    if (!isAlive(handle)) return;
    m_ShaderModulesPendingFrees.push_back({handle.id, handle.generation,fenceForCurrentFrame});
}

uint32_t LE::VulkanRHI::allocShaderModuleIndex(const vk::ShaderModule &module, uint64_t hashID) {
    uint32_t idx{};
    if (!m_ShaderModulesFreeList.empty())
    {
        idx = m_ShaderModulesFreeList.back();
        m_ShaderModulesFreeList.pop_back();
        m_ShaderModules[idx] = {module, hashID};
    }
    else {
        idx = static_cast<uint32_t>(m_ShaderModules.size());
        m_ShaderModules.emplace_back(module, hashID, 0, 0);
    }
    return idx;
}

bool LE::VulkanRHI::isAlive(ShaderHandle handle) const {
    return (handle.generation == m_ShaderModules[handle.id].generation);
}

void LE::VulkanRHI::freeShaderModuleIndex(uint32_t idx) {
    m_ShaderModulesFreeList.push_back(idx);
}

void LE::VulkanRHI::processDeferredFrees_ShaderModules() {
    for (size_t i = 0; i < m_ShaderModulesPendingFrees.size();)
    {
        PendingFree& pf = m_ShaderModulesPendingFrees[i];
        // Non-blocking check
        VkResult status = vkGetFenceStatus(vkCtx->device, pf.fence);
        if (status == VK_SUCCESS)
        {
            vkCtx->device.destroyShaderModule(m_ShaderModules[pf.index].shaderModule);
            m_ShaderModules[pf.index].shaderModule = VK_NULL_HANDLE;
            m_ShaderModules[pf.index].hashID= 0;

            // Invalidate old handles
            m_ShaderModules[pf.index].generation++;
            // Clear descriptor (recommended)
            freeShaderModuleIndex(pf.index);
            m_ShaderModulesPendingFrees[i] = m_ShaderModulesPendingFrees.back();
            m_ShaderModulesPendingFrees.pop_back();
        }
        else
        {
            ++i;
        }
    }
}

// Pipeline handling

LE::PipelineHandle LE::VulkanRHI::bindPipeline(const vk::Pipeline &pipeline) {
    uint32_t idx = allocPipelineIndex(pipeline);
    m_Pipelines[idx].generation++;

    return {.id = (int32_t)idx, .generation = m_Pipelines[idx].generation};
}

void LE::VulkanRHI::unbindPipeline(PipelineHandle handle, VkFence fenceForCurrentFrame) {
    if (!isAlive(handle)) return;
    m_PipelinesPendingFrees.push_back({handle.id, handle.generation,fenceForCurrentFrame});
}

uint32_t LE::VulkanRHI::allocPipelineIndex(const vk::Pipeline &pipeline) {
    uint32_t idx{};
    if (!m_PipelinesFreeList.empty())
    {
        idx = m_PipelinesFreeList.back();
        m_PipelinesFreeList.pop_back();
        m_Pipelines[idx] = {pipeline};
    }
    else {
        idx = static_cast<uint32_t>(m_Pipelines.size());
        m_Pipelines.emplace_back(pipeline, 0);
    }
    return idx;
}

bool LE::VulkanRHI::isAlive(PipelineHandle handle) const {
    return (handle.generation == m_Pipelines[handle.id].generation);
}

void LE::VulkanRHI::freePipelineIndex(uint32_t idx) {
    m_PipelinesFreeList.push_back(idx);
}

void LE::VulkanRHI::processDeferredFrees_Pipelines() {
    for (size_t i = 0; i < m_PipelinesPendingFrees.size();)
    {
        PendingFree& pf = m_PipelinesPendingFrees[i];
        // Non-blocking check
        VkResult status = vkGetFenceStatus(vkCtx->device, pf.fence);
        if (status == VK_SUCCESS)
        {
            vkCtx->device.destroyPipeline(m_Pipelines[pf.index].pipeline);
            m_Pipelines[pf.index].pipeline = VK_NULL_HANDLE;
            m_Pipelines[pf.index].generation++;

            freePipelineIndex(pf.index);
            m_PipelinesPendingFrees[i] = m_PipelinesPendingFrees.back();
            m_PipelinesPendingFrees.pop_back();
        }
        else
        {
            ++i;
        }
    }
}
