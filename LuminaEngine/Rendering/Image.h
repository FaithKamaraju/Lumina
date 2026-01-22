//
// Created by Faith Kamaraju on 2026-01-18.
//

#pragma once

namespace LE {

    struct ImageExtent2D {
        uint32_t    width;
        uint32_t    height;
    };

    struct ImageExtent3D {
        uint32_t    width;
        uint32_t    height;
        uint32_t    depth;
    };

    enum class ImageFormat
    {
        // ---- 8-bit formats ----
        R8_UNorm,
        R8_SNorm,
        R8_UInt,
        R8_SInt,
        R8_Srgb,

        // ---- 16-bit formats ----

        R16_UNorm,
        R16_SNorm,
        R16_UInt,
        R16_SInt,
        R16_Float,
        R8G8_UNorm,
        R8G8_SNorm,
        R8G8_UInt,
        R8G8_SInt,

        // ---- 24-bit formats ----

        R8G8B8_UNorm,
        R8G8B8_SNorm,
        R8G8B8_UInt,
        R8G8B8_SInt,
        R8G8B8_SRGB,
        B8G8R8_UNorm,
        B8G8R8_SNorm,
        B8G8R8_UInt,
        B8G8R8_SInt,
        B8G8R8_SRGB,


        // ---- 32-bit formats ----
        R32_UInt,
        R32_SInt,
        R32_Float,
        R16G16_UNorm,
        R16G16_SNorm,
        R16G16_UInt,
        R16G16_SInt,
        R16G16_Float,
        R8G8B8A8_UNorm,
        R8G8B8A8_SNorm,
        R8G8B8A8_UInt,
        R8G8B8A8_SInt,
        R8G8B8A8_SRGB,
        B8G8R8A8_UNorm,
        B8G8R8A8_SNorm,
        B8G8R8A8_UInt,
        B8G8R8A8_SInt,
        B8G8R8A8_SRGB,

        // ---- high precision formats ----
        R32G32B32A32_Float,
        R16G16B16A16_Float,

        // ---- Depth/Stencil ----
        D16_UNorm,
        D16_UNorm_S8_UInt,
        D24_UNorm_S8_UInt,
        D32_Float,
        D32_Float_S8_UInt,
    };

    enum class ImageUsageFlags : uint32_t
    {
        TransferSrc             = 0x00000001,
        TransferDst             = 0x00000002,
        ShaderRead              = 0x00000004,
        ShaderWrite             = 0x00000008,
        ColorAttachment         = 0x00000010,
        DepthStencilAttachment  = 0x00000020,
        Storage                 = 0x00000040,
    };

    // Flag ops:
    inline ImageUsageFlags operator|(ImageUsageFlags a, ImageUsageFlags b) {
        return static_cast<ImageUsageFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }
    inline ImageUsageFlags& operator|=(ImageUsageFlags& a, ImageUsageFlags b) {
        a = a | b; return a;
    }
    inline ImageUsageFlags operator&(ImageUsageFlags a, ImageUsageFlags b) {
        return static_cast<ImageUsageFlags>(
            static_cast<uint32_t>(a) & static_cast<uint32_t>(b)
        );
    }

    inline bool Any(ImageUsageFlags a) {
        return static_cast<uint32_t>(a) != 0;
    }

    struct ImageHandle {
        uint32_t id{};
        uint32_t generation{};
    };

}
