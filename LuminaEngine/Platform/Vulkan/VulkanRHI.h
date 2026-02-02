//
// Created by Faith Kamaraju on 2026-01-18.
//

#pragma once

#include "VulkanBuffer.h"
#include "Rendering/RHI.h"
#include "VulkanContext.h"
#include "VulkanSwapChain.h"
#include "FrameData.h"
#include "VulkanPipeline.h"


struct GLFWwindow;

namespace LE {

    struct VulkanImageSlot {
        VulkanImage image{};
        size_t hashName{};
        uint32_t generation{};
    };

    struct VulkanBufferSlot {
        VulkanBuffer buffer{};
        uint32_t generation{};
    };

    struct VulkanSamplerSlot {
        VkSampler sampler = VK_NULL_HANDLE;
        size_t hashID{};
        uint32_t generation{};
    };

    struct VulkanPipelineSlot {
        VulkanPipeline pipeline{};
        uint32_t generation{};
    };

    struct PendingFree{
        uint32_t index{};
        uint32_t generation{};
        VkFence fence = VK_NULL_HANDLE;   // fence that must be signaled before free
    };

    class VulkanRHI : public RHI {
    public:
        VulkanRHI();
        ~VulkanRHI() override;

        LEBool InitDevice(GLFWwindow* windowHandle) override;
        LEBool InitImgui() override;

        // API
        BufferHandle AllocateAndCopyBufferFromBytes(const std::byte* dataSource, size_t dataSize, BufferUsageFlags usage) override;
        BufferHandle AllocateAndCopyVertexBuffer(const std::vector<Vertex>& vertices) override;
        BufferHandle AllocateAndCopyIndexBuffer(const std::vector<uint32_t>& indices) override;
        BufferHandle AllocateAndCopyUniformBuffer(size_t uniformBufferSize) override;
        void CopyUniformData(BufferHandle ub, void* uniformData, size_t dataSize) override;
        void DeallocateBuffer(BufferHandle handle) override;
        ImageHandle CreateImage(size_t hashedName , unsigned char* data, uint32_t nrChannels, ImageExtent3D imagesize, ImageFormat format, ImageUsageFlags imageUsage, bool mipmapped) override;\
        void DestroyImage(ImageHandle handle) override;
        SamplerHandle CreateImageSampler(SamplerKey samplerInfo) override;
        void DestroyImageSampler(SamplerHandle samplerHandle) override;
        PipelineHandle CreateGraphicsPipeline(GraphicsPipelineDesc desc) override;
        PipelineHandle CreateComputePipeline(ComputePipelineDesc desc) override;


        void DrawFrame(const std::vector<Renderable>& renderables, float timestep) override;
        void ProcessDeferredFrees() override;

    private:
        void initSubmitSemaphores();
        void createPerFrameData();

        void recreateSwapChainAndPerFrameData(GLFWwindow *windowRef);
        void recordCommands(const std::vector<Renderable>& renderables, uint32_t frameIndex, uint32_t imageIndex, float timestep);


        // Descriptor handling
            // Descriptor Allocation
        vk::DescriptorSet allocateDescriptor(vk::DescriptorSetLayout layout);
        vk::DescriptorSet allocateVariableCountDescriptor(uint32_t descriptorCount, vk::DescriptorSetLayout layout);

            // Descriptor Writing code
        void writeBindlessImage(ImageHandle imageHandle, vk::ImageLayout imageLayout) const;
        void writeBindlessSampler(SamplerHandle samplerHandle) const;
        void writeUniformBuffer(BufferHandle bufferHandle, size_t ub_size, vk::DescriptorSet desc_set) const;


        // Image handling
        ImageHandle bindImage(const VulkanImage& image, size_t hashName);
        void unbindImage(ImageHandle handle, VkFence fenceForCurrentFrame);
        void processDeferredFrees_Images();
        uint32_t allocImageIndex(const VulkanImage& image, size_t hashName);
        bool isAlive(ImageHandle handle) const;
        void freeImageIndex(uint32_t idx);

        // Buffer handling
        BufferHandle bindBuffer(const VulkanBuffer& buf);
        void unbindBuffer(BufferHandle handle, VkFence fenceForCurrentFrame);
        void processDeferredFrees_Buffer();
        uint32_t allocBufferIndex(const VulkanBuffer &buf);
        bool isAlive(BufferHandle handle) const;
        void freeBufferIndex(uint32_t idx);

        // Sampler handling
        SamplerHandle bindSampler(const vk::Sampler& sampler, size_t hashID);
        void unbindSampler(SamplerHandle handle, VkFence fenceForCurrentFrame);
        void processDeferredFrees_Sampler();
        uint32_t allocSamplerIndex(const vk::Sampler &sampler, size_t hashID);
        bool isAlive(SamplerHandle handle) const;
        void freeSamplerIndex(uint32_t idx);


        GLFWwindow* m_WindowRef = nullptr;
        // ImguiVulkan* m_ImguiVulkanImpl = nullptr;
        VulkanContext vkCtx;
        SwapChain swapChain;

        uint8_t currentFrame = 0;
        std::array<FrameData, MAX_FRAMES_IN_FLIGHT> perFrameData;
        std::array<vk::Semaphore, 3> submitSemaphores{nullptr, nullptr, nullptr};

        std::vector<VulkanImageSlot> m_Images;
        std::vector<VulkanBufferSlot> m_Buffers;
        std::vector<VulkanSamplerSlot> m_Samplers;

        std::vector<uint32_t> m_ImagesFreeList;
        std::vector<PendingFree> m_ImagesPendingFrees;

        std::vector<uint32_t> m_BuffersFreeList;
        std::vector<PendingFree> m_BuffersPendingFrees;

        std::vector<uint32_t> m_SamplersFreeList;
        std::vector<PendingFree> m_SamplersPendingFrees;


    };
}