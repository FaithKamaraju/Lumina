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
#include "Rendering/GPUMaterialData.h"


struct GLFWwindow;

namespace LE {

    inline vk::DescriptorSetLayout sceneDescriptorLayout = nullptr;
    inline vk::DescriptorSetLayout bindlessTexturesLayout = nullptr;
    inline vk::DescriptorSetLayout bindlessSamplersLayout = nullptr;
    inline vk::DescriptorSetLayout materialDataDescriptorLayout = nullptr;
    inline uint32_t maxSets = 8;
    inline std::vector<PoolSize> sizes =
    {
        { vk::DescriptorType::eUniformBuffer, MAX_FRAMES_IN_FLIGHT },
        { vk::DescriptorType::eSampledImage, MAX_FRAMES_IN_FLIGHT * MAX_BINDLESS_TEXTURES },
        { vk::DescriptorType::eSampler, MAX_FRAMES_IN_FLIGHT * MAX_BINDLESS_SAMPLERS },
        { vk::DescriptorType::eStorageBuffer, MAX_FRAMES_IN_FLIGHT },
    };

    struct VulkanImageSlot {
        VulkanImage image{};
        uint64_t hashID{};
        uint32_t numTextureRefs{};
        uint32_t generation{};
    };

    struct VulkanBufferSlot {
        VulkanBuffer buffer{};
        uint32_t generation{};
    };

    struct VulkanSamplerSlot {
        VkSampler sampler = VK_NULL_HANDLE;
        size_t hashID{};
        uint32_t numTextureRefs{};
        uint32_t generation{};
    };

    struct VulkanShaderSlot {
        vk::ShaderModule shaderModule = VK_NULL_HANDLE;
        uint64_t hashID{};
        uint32_t numMaterialRefs{};
        uint32_t generation{};
    };

    struct VulkanPipelineSlot {
        vk::Pipeline pipeline = VK_NULL_HANDLE;
        uint32_t generation{};
    };


    struct PendingFree{
        int32_t index{};
        uint32_t generation{};
        VkFence fence = VK_NULL_HANDLE;   // fence that must be signaled before free
    };

    class VulkanRHI : public RHI {
    public:
        VulkanRHI();
        ~VulkanRHI() override;

        LEBool InitDevice(GLFWwindow* windowHandle) override;
        LEBool InitImgui() override;

        void UpdateMaterialDataSSBOs(PBRMaterialGPUData data, size_t index) override;
        void UpdateCameraAndSceneData() override;

        // API
        BufferHandle AllocateAndCopyBufferFromBytes(const std::byte* dataSource, size_t dataSize, BufferUsageFlags usage) override;
        BufferHandle AllocateAndCopyVertexBuffer(const std::vector<Vertex>& vertices) override;
        BufferHandle AllocateAndCopyIndexBuffer(const std::vector<uint32_t>& indices) override;
        BufferHandle AllocateUniformBuffer(size_t uniformBufferSize) override;
        BufferHandle AllocateStorageBuffer(size_t storageBufferSize) override;
        void UpdateUniformBufferData(BufferHandle ub, void* uniformData, size_t dataSize, size_t offset) override;
        void UpdateStorageBufferData(BufferHandle ub, void* storageData, size_t dataSize, size_t offset) override;
        void DeallocateBuffer(BufferHandle handle) override;

        ImageHandle CreateImage(uint64_t hashID , unsigned char* data, uint32_t nrChannels, ImageExtent3D imagesize,
            ImageFormat format, ImageUsageFlags imageUsage, bool mipmapped) override;
        void IncrementImageRef(ImageHandle handle) override;
        void DecrementImageRef(ImageHandle handle) override;
        void DestroyImage(ImageHandle handle) override;

        SamplerHandle CreateImageSampler(SamplerKey samplerInfo) override;
        void IncrementSamplerRef(SamplerHandle handle) override;
        void DecrementSamplerRef(SamplerHandle handle) override;
        void DestroyImageSampler(SamplerHandle samplerHandle) override;

        ShaderHandle CreateShaderModule(uint64_t hashID, const std::vector<uint32_t> &spvBinary) override;
        void IncrementShaderModuleRef(ShaderHandle handle) override;
        void DecrementShaderModuleRef(ShaderHandle handle) override;
        void DestroyShaderModule(ShaderHandle shaderHandle) override;

        PipelineHandle CreateGraphicsPipeline(const GraphicsPipelineDesc& desc) override;
        PipelineHandle CreateComputePipeline(const ComputePipelineDesc& desc) override;


        void DrawFrame(const std::vector<Renderable>& renderables, float timestep) override;
        void ProcessDeferredFrees() override;

    private:
        void initSubmitSemaphores();
        void constructDescriptorSetLayouts();
        void createPerFrameData();
        void constructPipelineLayout();

        void recreateSwapChainAndPerFrameData(GLFWwindow *windowRef);
        void recordCommands(const std::vector<Renderable>& renderables, uint32_t frameIndex, uint32_t imageIndex, float timestep) const;


        // Descriptor handling
            // Descriptor Allocation
        vk::DescriptorSet allocateDescriptor(vk::DescriptorSetLayout layout);
        vk::DescriptorSet allocateVariableCountDescriptor(uint32_t descriptorCount, vk::DescriptorSetLayout layout);

            // Descriptor Writing code
        void writeBindlessImage(ImageHandle imageHandle, vk::ImageLayout imageLayout) const;
        void writeBindlessSampler(SamplerHandle samplerHandle) const;
        void writeUniformBuffer(BufferHandle bufferHandle, size_t ub_size, vk::DescriptorSet desc_set) const;
        void writeStorageBuffer(BufferHandle bufferHandle, size_t sb_size, vk::DescriptorSet desc_set) const;


        // Image handling
        ImageHandle bindImage(const VulkanImage& image, uint64_t hashID);
        void unbindImage(ImageHandle handle, VkFence fenceForCurrentFrame);
        void processDeferredFrees_Images();
        uint32_t allocImageIndex(const VulkanImage& image, uint64_t hashID);
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

        // Shader Module handling
        ShaderHandle bindShaderModule(const vk::ShaderModule& module, uint64_t hashID);
        void unbindShaderModule(ShaderHandle handle, VkFence fenceForCurrentFrame);
        void processDeferredFrees_ShaderModules();
        uint32_t allocShaderModuleIndex(const vk::ShaderModule& module, uint64_t hashID);
        bool isAlive(ShaderHandle handle) const;
        void freeShaderModuleIndex(uint32_t idx);

        // Pipeline handling
        PipelineHandle bindPipeline(const vk::Pipeline& pipeline);
        void unbindPipeline(PipelineHandle handle, VkFence fenceForCurrentFrame);
        void processDeferredFrees_Pipelines();
        uint32_t allocPipelineIndex(const vk::Pipeline& pipeline);
        bool isAlive(PipelineHandle handle) const;
        void freePipelineIndex(uint32_t idx);



        GLFWwindow* m_WindowRef = nullptr;
        // ImguiVulkan* m_ImguiVulkanImpl = nullptr;
        VulkanContext* vkCtx = nullptr;
        SwapChain* swapChain = nullptr;
        uint8_t currentFrame = 0;
        std::array<FrameData, MAX_FRAMES_IN_FLIGHT> perFrameData;
        std::array<vk::Semaphore, 3> submitSemaphores{nullptr, nullptr, nullptr};

        vk::PipelineLayout m_PipelineLayout = VK_NULL_HANDLE;


        std::vector<VulkanImageSlot> m_Images;
        std::vector<VulkanBufferSlot> m_Buffers;
        std::vector<VulkanSamplerSlot> m_Samplers;
        std::vector<VulkanShaderSlot> m_ShaderModules;
        std::vector<VulkanPipelineSlot> m_Pipelines;

        std::vector<uint32_t> m_ImagesFreeList;
        std::vector<PendingFree> m_ImagesPendingFrees;

        std::vector<uint32_t> m_BuffersFreeList;
        std::vector<PendingFree> m_BuffersPendingFrees;

        std::vector<uint32_t> m_SamplersFreeList;
        std::vector<PendingFree> m_SamplersPendingFrees;

        std::vector<uint32_t> m_ShaderModulesFreeList;
        std::vector<PendingFree> m_ShaderModulesPendingFrees;

        std::vector<uint32_t> m_PipelinesFreeList;
        std::vector<PendingFree> m_PipelinesPendingFrees;


    };
}