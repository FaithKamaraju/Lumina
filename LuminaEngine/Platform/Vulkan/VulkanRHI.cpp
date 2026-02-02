//
// Created by Faith Kamaraju on 2026-01-18.
//

#include "VulkanRHI.h"
#include <glm/glm.hpp>
#include "Platform/Vulkan/VulkanUtils.h"
#include "Core/Events/EventManager.h"
#include "Core/Events/WindowEvent.h"
#include "VulkanBuffer.h"
#include "VulkanSampler.h"
#include "Rendering/RenderingConstants.h"


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
}

LE::VulkanRHI::~VulkanRHI() {
}

LEBool LE::VulkanRHI::InitDevice(GLFWwindow* windowHandle) {

    m_WindowRef = windowHandle;
    vkCtx.InitVulkanInstance();
    vkCtx.AcquireSurface(windowHandle);
    vkCtx.PickPhysicalDevice();

    std::set queueIndices = {vkCtx.queueFamilyIndices.graphicsFamily.value(), vkCtx.queueFamilyIndices.transferFamily.value()};
    if (queueIndices.size() > 1) {
        vkCtx.queueIndicesArr.emplace_back(vkCtx.queueFamilyIndices.transferFamily.value());
        vkCtx.queueIndicesArr.emplace_back(vkCtx.queueFamilyIndices.graphicsFamily.value());
    }
    else if (queueIndices.size() == 1) {
        vkCtx.queueIndicesArr.emplace_back(vkCtx.queueFamilyIndices.transferFamily.value());
    }

    vkCtx.CreateLogicalDevice();
    vkCtx.CreateVmaAllocator();

    swapChain.CreateSwapChain(&vkCtx, windowHandle);

    initSubmitSemaphores();
    vkCtx.InitPools();
    vkCtx.descriptorAllocator.InitPool(&vkCtx, FrameData::maxSets, FrameData::sizes, vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind);
    createPerFrameData();

    LE::Events::Subscribe<LE::WindowFramebufferResizeEvent>([this](const LE::WindowFramebufferResizeEvent &e) {
           swapChain.m_FramebufferResized = true;
           LE_CORE_INFO("new framebuffer size {0} x {1}", e.width, e.height);
       });

    return LE_SUCCESS;
}

LEBool LE::VulkanRHI::InitImgui() {

    return LE_SUCCESS;
}

LE::BufferHandle LE::VulkanRHI::AllocateAndCopyBufferFromBytes(const std::byte *dataSource, size_t dataSize, BufferUsageFlags usage) {

    vk::BufferCreateInfo bufferInfo{
        .size = dataSize,
        .usage = ToVkBufferUsageFlags(usage) | vk::BufferUsageFlagBits::eTransferDst,
        .sharingMode = vkCtx.queueIndicesArr.size() > 1 ?  vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive,
        .queueFamilyIndexCount = static_cast<uint32_t>(vkCtx.queueIndicesArr.size()),
        .pQueueFamilyIndices = vkCtx.queueIndicesArr.data()
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
        .sharingMode = vkCtx.queueIndicesArr.size() > 1 ?  vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive,
        .queueFamilyIndexCount = static_cast<uint32_t>(vkCtx.queueIndicesArr.size()),
        .pQueueFamilyIndices = vkCtx.queueIndicesArr.data()
    };
    VulkanBuffer buf = Buffers::allocateBuffer(vkCtx, bufferInfo, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
    Buffers::copyData(vkCtx, buf, (void*)vertices.data(), vertices.size()*sizeof(vertices[0]));

    return bindBuffer(buf);

}

LE::BufferHandle LE::VulkanRHI::AllocateAndCopyIndexBuffer(const std::vector<uint32_t> &indices) {

    vk::BufferCreateInfo bufferInfo{
        .size = indices.size() * sizeof(indices[0]),
        .usage = vk::BufferUsageFlagBits::eIndexBuffer |
            vk::BufferUsageFlagBits::eTransferDst,
        .sharingMode = vkCtx.queueIndicesArr.size() > 1 ?  vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive,
        .queueFamilyIndexCount = static_cast<uint32_t>(vkCtx.queueIndicesArr.size()),
        .pQueueFamilyIndices = vkCtx.queueIndicesArr.data()
    };

    VulkanBuffer buf = Buffers::allocateBuffer(vkCtx, bufferInfo, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
    Buffers::copyData(vkCtx, buf, (void*)indices.data(), indices.size() * sizeof(indices[0]));

    return bindBuffer(buf);
}

LE::BufferHandle LE::VulkanRHI::AllocateAndCopyUniformBuffer(size_t uniformBufferSize) {
    vk::BufferCreateInfo bufferInfo{
        .size = uniformBufferSize,
        .usage = vk::BufferUsageFlagBits::eUniformBuffer |
            vk::BufferUsageFlagBits::eTransferDst,
        .sharingMode = vkCtx.queueIndicesArr.size() > 1 ?  vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive,
        .queueFamilyIndexCount = static_cast<uint32_t>(vkCtx.queueIndicesArr.size()),
        .pQueueFamilyIndices = vkCtx.queueIndicesArr.data()
    };

    VulkanBuffer buf = Buffers::allocateBuffer(vkCtx, bufferInfo, VMA_MEMORY_USAGE_AUTO,VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
    VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT |
    VMA_ALLOCATION_CREATE_MAPPED_BIT);

    return bindBuffer(buf);
}

void LE::VulkanRHI::CopyUniformData(BufferHandle ub, void *uniformData, size_t dataSize) {
    vk::CommandBufferAllocateInfo allocInfo{ .commandPool = vkCtx.transferCommandPool,
                                             .level = vk::CommandBufferLevel::ePrimary,
                                             .commandBufferCount = 1 };
    vk::CommandBuffer cmdBuf = vkCtx.device.allocateCommandBuffers(allocInfo).front();
    cmdBuf.begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

    VkMemoryPropertyFlags memPropFlags;
    vmaGetAllocationMemoryProperties(vkCtx.vmaAllocator, m_Buffers[ub.id].buffer.allocation, &memPropFlags);

    if(memPropFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
    {
        vmaCopyMemoryToAllocation(vkCtx.vmaAllocator,uniformData, m_Buffers[ub.id].buffer.allocation,
            0, dataSize );
        Buffers::placeBufferMemoryBarrier(
                   cmdBuf,
                   vk::AccessFlagBits2::eHostWrite,
                   vk::AccessFlagBits2::eUniformRead,
                   vk::PipelineStageFlagBits2::eHost,
                   vk::PipelineStageFlagBits2::eVertexShader|vk::PipelineStageFlagBits2::eFragmentShader,
                   m_Buffers[ub.id].buffer.buffer,
                   0,
                   VK_WHOLE_SIZE);
        cmdBuf.end();
        vkCtx.transferQueue.submit(vk::SubmitInfo{ .commandBufferCount = 1, .pCommandBuffers = &cmdBuf },
            nullptr);
        vkCtx.transferQueue.waitIdle();
    }
    else
    {
        vk::BufferCreateInfo stagingInfo{
            .size = dataSize,
            .usage = vk::BufferUsageFlagBits::eTransferSrc,
            .sharingMode = vkCtx.queueIndicesArr.size() > 1 ?  vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive,
            .queueFamilyIndexCount = static_cast<uint32_t>(vkCtx.queueIndicesArr.size()),
            .pQueueFamilyIndices = vkCtx.queueIndicesArr.data()
        };
        VulkanBuffer staging = Buffers::allocateBuffer(vkCtx, stagingInfo);
        vmaCopyMemoryToAllocation(vkCtx.vmaAllocator, uniformData,
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
        cmdBuf.copyBuffer(staging.buffer, m_Buffers[ub.id].buffer.buffer, vk::BufferCopy(0, 0,
            dataSize));
        Buffers::placeBufferMemoryBarrier(
               cmdBuf,
               vk::AccessFlagBits2::eTransferWrite,
               vk::AccessFlagBits2::eUniformRead,
               vk::PipelineStageFlagBits2::eHost,
               vk::PipelineStageFlagBits2::eVertexShader | vk::PipelineStageFlagBits2::eFragmentShader,
               m_Buffers[ub.id].buffer.buffer,
               0,
               VK_WHOLE_SIZE);
         cmdBuf.end();
         vkCtx.transferQueue.submit(vk::SubmitInfo{ .commandBufferCount = 1, .pCommandBuffers = &cmdBuf },
             nullptr);
         vkCtx.transferQueue.waitIdle();

        vmaDestroyBuffer(vkCtx.vmaAllocator, staging.buffer, staging.allocation);

    }
}

LE::ImageHandle LE::VulkanRHI::CreateImage(size_t hashedName, unsigned char *data, uint32_t nrChannels, ImageExtent3D imagesize, ImageFormat format,
                                           ImageUsageFlags imageUsage, bool mipmapped) {

    for (uint32_t i = 0; i < m_Images.size(); i++) {
        if (hashedName == m_Images[i].hashName) {
            return {.id = i, .generation = m_Images[i].generation};
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
    imageInfo.sharingMode = vkCtx.queueIndicesArr.size() > 1 ?  vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive;
    imageInfo.queueFamilyIndexCount = static_cast<uint32_t>(vkCtx.queueIndicesArr.size());
    imageInfo.pQueueFamilyIndices = vkCtx.queueIndicesArr.data();

    texture = Images::allocateImage(vkCtx, imageInfo);
    Images::copyData(vkCtx, texture, data, imagesize.width * imagesize.height * nrChannels);
    Images::createImageView(vkCtx, texture, vk::ImageAspectFlagBits::eColor);

    return bindImage(texture, hashedName);
}

LE::SamplerHandle LE::VulkanRHI::CreateImageSampler(SamplerKey samplerInfo) {

    for (uint32_t i = 0; i < m_Samplers.size(); i++) {
        if (samplerInfo.hashID == m_Samplers[i].hashID) {
            return {.id = i, .generation = m_Samplers[i].generation};
        }
    }
    vk::PhysicalDeviceProperties properties = vkCtx.primaryPhysicalDevice.getProperties();
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    vk::SamplerCreateInfo info = ToVulkanSamplerCreateInfo(samplerInfo);
    vk::Sampler samp = vkCtx.device.createSampler(info);

    return bindSampler(samp, samplerInfo.hashID);
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

LE::PipelineHandle LE::VulkanRHI::CreateGraphicsPipeline(GraphicsPipelineDesc desc) {
    return {};

}

LE::PipelineHandle LE::VulkanRHI::CreateComputePipeline(ComputePipelineDesc desc) {
    return {};
}


void LE::VulkanRHI::DrawFrame(const std::vector<Renderable>& renderables, float timestep) {

    auto _ = vkCtx.device.waitForFences(perFrameData[currentFrame].inFlightFence, vk::True, UINT64_MAX);

    auto result = vkCtx.device.acquireNextImageKHR(swapChain.m_SwapChain, UINT64_MAX, perFrameData[currentFrame].acquireSemaphore,
        VK_NULL_HANDLE );
    if (result.result == vk::Result::eErrorOutOfDateKHR || result.result == vk::Result::eSuboptimalKHR || swapChain.m_FramebufferResized)
    {
        swapChain.m_FramebufferResized = false;
        recreateSwapChainAndPerFrameData(m_WindowRef);
        return;
    }
    else if (result.result != vk::Result::eSuccess) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    vkCtx.device.resetFences(perFrameData[currentFrame].inFlightFence);
    perFrameData[currentFrame].graphicsCommandBuffer.reset({});
    recordCommands(renderables, currentFrame, result.value, timestep);


    // updateUniformsBuffers(currentFrame, timestep);

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

    vkCtx.graphicsQueue.submit(submitInfo, perFrameData[currentFrame].inFlightFence);

    vk::PresentInfoKHR presentInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &submitSemaphores[result.value],
        .swapchainCount = 1,
        .pSwapchains = &swapChain.m_SwapChain,
        .pImageIndices = &result.value,
        .pResults = nullptr,
    };


    vk::Result presentResult = vkCtx.presentQueue.presentKHR(presentInfo);
    if (presentResult == vk::Result::eErrorOutOfDateKHR || presentResult == vk::Result::eSuboptimalKHR  || swapChain.m_FramebufferResized)
    {
        swapChain.m_FramebufferResized = false;
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
}



// private

void LE::VulkanRHI::initSubmitSemaphores() {
    for (int i = 0; i < swapChain.m_SwapChainImages.size(); i++) {
        vk::SemaphoreCreateInfo semaphoreInfo{};
        submitSemaphores[i] = vkCtx.device.createSemaphore(semaphoreInfo);
    }
}

void LE::VulkanRHI::createPerFrameData() {

    FrameData::constructDescriptorSetLayouts(vkCtx);
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {

        perFrameData[i] = {};
        perFrameData[i].initSyncObjects(vkCtx);

        perFrameData[i].uniformBuffer = AllocateAndCopyUniformBuffer(sizeof(SceneUniformBuffer));
        perFrameData[i].sceneDescriptorSet = allocateDescriptor(FrameData::sceneDescriptorLayout);
        writeUniformBuffer(perFrameData[i].uniformBuffer, sizeof(SceneUniformBuffer),
            perFrameData[i].sceneDescriptorSet);

        perFrameData[i].bindlessTexturesDescriptorSet = allocateVariableCountDescriptor(MAX_BINDLESS_TEXTURES,
            FrameData::bindlessTexturesLayout);
        perFrameData[i].bindlessSamplersDescriptorSet = allocateVariableCountDescriptor(MAX_BINDLESS_SAMPLERS,
            FrameData::bindlessSamplersLayout);

    }
}


void LE::VulkanRHI::recreateSwapChainAndPerFrameData(GLFWwindow *windowRef) {

    vkCtx.device.waitIdle();

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkCtx.device.destroySemaphore(perFrameData[i].acquireSemaphore, nullptr);
        vkCtx.device.destroyFence(perFrameData[i].inFlightFence, nullptr);
        vkCtx.device.destroyCommandPool(perFrameData[i].graphicsCommandPool, nullptr);
    }
    for (int i = 0; i < swapChain.m_SwapChainImageViews.size(); i++) {
        vkCtx.device.destroySemaphore(submitSemaphores[i], nullptr);
    }

    vkCtx.device.destroyCommandPool(vkCtx.transferCommandPool);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        perFrameData[i].initSyncObjects(vkCtx);

    }
    swapChain.RecreateSwapChain(windowRef);
    initSubmitSemaphores();
    vkCtx.InitPools();
}

void LE::VulkanRHI::recordCommands(const std::vector<Renderable>& renderables, uint32_t frameIndex, uint32_t imageIndex, float timestep) {
    vk::CommandBufferBeginInfo beginInfo{};
    vk::Result result = perFrameData[frameIndex].graphicsCommandBuffer.begin(&beginInfo);

    Images::transitionImageLayout(
        perFrameData[frameIndex].graphicsCommandBuffer,
        swapChain.m_SwapChainImages[imageIndex],
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        {},
        vk::AccessFlagBits2::eColorAttachmentWrite,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput
    );

    vk::ClearValue clearColor = {{0.0f, 0.0f, 0.0f, 1.0f}};
    vk::ClearDepthStencilValue clearDepthStencil = {1.f, 0};
    vk::RenderingAttachmentInfo attachmentInfo = {
        .imageView = swapChain.m_SwapChainImageViews[imageIndex],
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = clearColor
    };
    vk::RenderingAttachmentInfo depthAttachInfo = {
        .imageView = swapChain.m_DepthBuffer.image.imageView,
        .imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eDontCare,
        .clearValue = clearDepthStencil
    };

    // Set up the rendering info
    vk::RenderingInfo renderingInfo = {
        .renderArea = { .offset = { 0, 0 }, .extent = swapChain.m_SwapChainExtent },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachmentInfo,
        .pDepthAttachment = &depthAttachInfo,
    };

    perFrameData[frameIndex].graphicsCommandBuffer.beginRendering(renderingInfo);

    // perFrameData[frameIndex].graphicsCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_Materials[0]->pipeline);

    // vk::Buffer vertexBuffers[] = {testObj->mesh.vertexBuffer.buffer};
    // vk::DeviceSize offsets[] = {0};
    // m_perFrameData[frameIndex].graphicsCommandBuffer.bindVertexBuffers(0, vertexBuffers, offsets);
    // perFrameData[frameIndex].graphicsCommandBuffer.bindIndexBuffer(testObj->mesh.indexBuffer.buffer, 0, vk::IndexType::eUint32);

    vk::Viewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChain.m_SwapChainExtent.width);
    viewport.height = static_cast<float>(swapChain.m_SwapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    perFrameData[frameIndex].graphicsCommandBuffer.setViewport(0, viewport);

    vk::Rect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChain.m_SwapChainExtent;
    perFrameData[frameIndex].graphicsCommandBuffer.setScissor(0, scissor);

    // FrameData::PerObjectConstants object{};
    // object.bufferAddress = testObj->mesh.vertexBufAdd;
    // object.albedoIndex = testObj->m_MaterialInstance->textureIndex.index;
    // object.model = glm::mat4(1.0f);
    // object.model = glm::rotate(object.model, timestep * glm::radians(60.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    //  perFrameData[frameIndex].graphicsCommandBuffer.pushConstants(m_Materials[0]->pipelineLayout,
    //      vk::ShaderStageFlagBits::eFragment|vk::ShaderStageFlagBits::eVertex, 0, sizeof(FrameData::PerObjectConstants),&object);
    //
    // std::array descSets = {
    //     perFrameData[frameIndex].sceneDescriptorSet, perFrameData[frameIndex].bindlessTexturesDescriptorSet
    // };
    // perFrameData[frameIndex].graphicsCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_Materials[0]->pipelineLayout, 0,
    //     2, descSets.data(), 0, nullptr);
    //
    //
    // perFrameData[frameIndex].graphicsCommandBuffer.drawIndexed(static_cast<uint32_t>(indices.size()),
    //     1, 0, 0, 0);

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
            swapChain.m_SwapChainImages[imageIndex],
            vk::ImageLayout::eColorAttachmentOptimal,
            vk::ImageLayout::ePresentSrcKHR,
            vk::AccessFlagBits2::eColorAttachmentWrite,                // srcAccessMask
            {},                                                        // dstAccessMask
            vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // srcStage
            vk::PipelineStageFlagBits2::eBottomOfPipe                  // dstStage
        );

    perFrameData[frameIndex].graphicsCommandBuffer.end();

}




// Descriptor Handling section

    // Descriptor Allocation code
vk::DescriptorSet LE::VulkanRHI::allocateDescriptor(vk::DescriptorSetLayout layout) {
    return vkCtx.descriptorAllocator.Allocate(&vkCtx,layout, nullptr);
}

vk::DescriptorSet LE::VulkanRHI::allocateVariableCountDescriptor(uint32_t descriptorCount, vk::DescriptorSetLayout layout) {

    vk::DescriptorSetVariableDescriptorCountAllocateInfoEXT variableDescriptorCountAllocInfo = {};
    variableDescriptorCountAllocInfo.descriptorSetCount = 1;
    variableDescriptorCountAllocInfo.pDescriptorCounts  = &descriptorCount;

    return vkCtx.descriptorAllocator.Allocate(&vkCtx, layout, &variableDescriptorCountAllocInfo);
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
        vkCtx.device.updateDescriptorSets(imageDescriptorWrite, nullptr);
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
        vkCtx.device.updateDescriptorSets(samplerDescriptorWrite, nullptr);
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
    vkCtx.device.updateDescriptorSets(uboDescriptorWrite,nullptr);
}


// Image Handling section

LE::ImageHandle LE::VulkanRHI::bindImage(const VulkanImage &image, size_t hashName) {

    uint32_t idx = allocImageIndex(image, hashName);
    m_Images[idx].generation++;

    ImageHandle handle{.id = idx, .generation = m_Images[idx].generation};
    writeBindlessImage(handle, vk::ImageLayout::eShaderReadOnlyOptimal);
    return handle;
}

void LE::VulkanRHI::unbindImage(ImageHandle handle, VkFence fenceForCurrentFrame) {
    if (!isAlive(handle)) return;
    m_ImagesPendingFrees.push_back({handle.id, handle.generation,fenceForCurrentFrame});

}

uint32_t LE::VulkanRHI::allocImageIndex(const VulkanImage& image, size_t hashName) {
    uint32_t idx{};
    if (!m_ImagesFreeList.empty())
    {
        idx = m_ImagesFreeList.back();
        m_ImagesFreeList.pop_back();
        m_Images[idx] = {image, hashName};
    }
    else {
        idx = static_cast<uint32_t>(m_Images.size());
        m_Images.emplace_back(image, hashName);
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
        VkResult status = vkGetFenceStatus(vkCtx.device, pf.fence);
        if (status == VK_SUCCESS)
        {
            Images::destroyImage(vkCtx, m_Images[pf.index].image);
            m_Images[pf.index].image = VulkanImage{};
            m_Images[pf.index].hashName= 0;

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

    return {.id = idx, .generation = m_Buffers[idx].generation};
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
        m_Buffers.emplace_back(buf);
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
        VkResult status = vkGetFenceStatus(vkCtx.device, pf.fence);
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

    SamplerHandle handle{.id = idx, .generation = m_Samplers[idx].generation};
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
        m_Samplers.emplace_back(sampler, hashID);
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
        VkResult status = vkGetFenceStatus(vkCtx.device, pf.fence);
        if (status == VK_SUCCESS)
        {
            vkCtx.device.destroySampler(m_Samplers[pf.index].sampler);
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

