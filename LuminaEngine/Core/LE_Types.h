//
// Created by Faith Kamaraju on 2026-01-14.
//

#pragma once

using LEBool = bool;

constexpr LEBool LE_SUCCESS = true;
constexpr LEBool LE_FAILURE = false;

namespace LE {

    enum class GraphicsAPI : int {
        VULKAN = 0,
        D3D12 = 1,
        METAL = 2
    };



    template<typename T>
    using Scope = std::unique_ptr<T>;
    template<typename T, typename ... Args>
    constexpr Scope<T> CreateScope(Args&& ... args)
    {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    template<typename T>
    using Ref = std::shared_ptr<T>;
    template<typename T, typename ... Args>
    constexpr Ref<T> CreateRef(Args&& ... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }
}