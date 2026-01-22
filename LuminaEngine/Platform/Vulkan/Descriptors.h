//
// Created by Faith Kamaraju on 2026-01-21.
//

#pragma once
#include "VulkanContext.h"

namespace LE {
    class DescriptorAllocator {
    public:
        void InitPool(VulkanContext& ctx, uint32_t maxSets, std::span<PoolSize> sizes, vk::DescriptorPoolCreateFlags poolFlags);
        void ClearDescriptors(VulkanContext& ctx) const;
        void DestroyPool(VulkanContext& ctx) const;

        vk::DescriptorSet Allocate(VulkanContext& ctx, vk::DescriptorSetLayout layout, void* pnext) const;
        std::vector<vk::DescriptorSet> Allocate(VulkanContext& ctx, const std::vector<vk::DescriptorSetLayout>& layouts,
            void* pnext) const;

    private:
        vk::DescriptorPool pool;
    };


    class DescriptorBuilder {
    public:
        void AddBinding(
            uint32_t binding,
            vk::DescriptorType type,
            uint32_t count,
            vk::ShaderStageFlags shaderStages,
            vk::Sampler* immutableSampler);
        vk::DescriptorSetLayout Build(
            VulkanContext& ctx,
            vk::DescriptorSetLayoutCreateFlags setInfoflags,
            vk::DescriptorBindingFlags* bindingFlags);
        void Clear();

    private:
        std::vector<vk::DescriptorSetLayoutBinding> bindings;

    };
}
