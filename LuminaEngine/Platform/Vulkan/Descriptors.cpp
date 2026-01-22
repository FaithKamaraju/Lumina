//
// Created by Faith Kamaraju on 2026-01-21.
//

#include "Descriptors.h"

void LE::DescriptorAllocator::InitPool(VulkanContext &ctx, uint32_t maxSets, std::span<PoolSize> sizes,
    vk::DescriptorPoolCreateFlags poolFlags) {

    std::vector<vk::DescriptorPoolSize> poolSizes;
    for (PoolSize size : sizes) {
        poolSizes.push_back(vk::DescriptorPoolSize{
            .type = size.type,
            .descriptorCount = size.count
        });
    }
    vk::DescriptorPoolCreateInfo poolInfo = {};
    poolInfo.maxSets = maxSets;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.flags = poolFlags;

    pool = ctx.device.createDescriptorPool(poolInfo);
}

void LE::DescriptorAllocator::ClearDescriptors(VulkanContext &ctx) const {
    ctx.device.resetDescriptorPool(pool);
}

void LE::DescriptorAllocator::DestroyPool(VulkanContext &ctx) const {
    ctx.device.destroyDescriptorPool(pool);
}

vk::DescriptorSet LE::DescriptorAllocator::Allocate(VulkanContext &ctx, vk::DescriptorSetLayout layout,
    void *pnext) const {
    vk::DescriptorSetAllocateInfo allocInfo{};
    allocInfo.descriptorPool = pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout;
    allocInfo.pNext = pnext;

    return ctx.device.allocateDescriptorSets(allocInfo).front();
}

std::vector<vk::DescriptorSet> LE::DescriptorAllocator::Allocate(VulkanContext &ctx,
    const std::vector<vk::DescriptorSetLayout> &layouts, void *pnext) const {

    vk::DescriptorSetAllocateInfo allocInfo{};
    allocInfo.descriptorPool = pool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
    allocInfo.pSetLayouts = layouts.data();

    return ctx.device.allocateDescriptorSets(allocInfo);
}





void LE::DescriptorBuilder::AddBinding(uint32_t binding, vk::DescriptorType type, uint32_t count,
                                       vk::ShaderStageFlags shaderStages, vk::Sampler *immutableSampler) {

    vk::DescriptorSetLayoutBinding newBinding{
        .binding = binding,
        .descriptorType = type,
        .descriptorCount = count,
        .stageFlags = shaderStages,
        .pImmutableSamplers = immutableSampler,
    };
    bindings.push_back(newBinding);
}

vk::DescriptorSetLayout LE::DescriptorBuilder::Build(VulkanContext &ctx,
    vk::DescriptorSetLayoutCreateFlags setInfoflags, vk::DescriptorBindingFlags *bindingFlags) {

    vk::DescriptorSetLayoutBindingFlagsCreateInfoEXT bindingFlagsCreateInfo{};
    if (bindingFlags != nullptr) {
        bindingFlagsCreateInfo.bindingCount   = static_cast<uint32_t>(bindings.size());
        bindingFlagsCreateInfo.pBindingFlags  = bindingFlags;
    }

    vk::DescriptorSetLayoutCreateInfo layoutInfo{
        .pNext = &bindingFlagsCreateInfo ,
        .flags = setInfoflags,
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.data(),

    };
    return ctx.device.createDescriptorSetLayout(layoutInfo, nullptr);
}

void LE::DescriptorBuilder::Clear() {
    bindings.clear();
}
