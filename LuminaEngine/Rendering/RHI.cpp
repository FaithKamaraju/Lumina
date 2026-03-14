//
// Created by Faith Kamaraju on 2026-02-15.
//

#include "RHI.h"
#include "Platform/Vulkan/VulkanRHI.h"

LE::Ref<LE::RHI> LE::CreateRHI(GraphicsAPI api) {

    switch (api) {
        case GraphicsAPI::VULKAN:
            return CreateRef<VulkanRHI>();
        case GraphicsAPI::D3D12:
        case GraphicsAPI::METAL:
            return nullptr;
    }
    return nullptr;
}
