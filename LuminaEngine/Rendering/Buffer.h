//
// Created by Faith Kamaraju on 2026-01-18.
//

#pragma once

namespace LE {

    enum class BufferUsageFlags : uint32_t {
        TransferSrc            = 0x00000001,
        TransferDst            = 0x00000002,
        UniformBuffer          = 0x00000004,
        StorageBuffer          = 0x00000008,
        IndexBuffer            = 0x00000010,
        VertexBuffer           = 0x00000020,
        ShaderDeviceAddress    = 0x00000040,
    };

    // Flag ops:
    inline BufferUsageFlags operator|(BufferUsageFlags a, BufferUsageFlags b) {
        return static_cast<BufferUsageFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }
    inline BufferUsageFlags& operator|=(BufferUsageFlags& a, BufferUsageFlags b) {
        a = a | b; return a;
    }
    inline BufferUsageFlags operator&(BufferUsageFlags a, BufferUsageFlags b) {
        return static_cast<BufferUsageFlags>(
            static_cast<uint32_t>(a) & static_cast<uint32_t>(b)
        );
    }

    inline bool Any(BufferUsageFlags a) {
        return static_cast<uint32_t>(a) != 0;
    }


    struct BufferHandle {
        int32_t id = -1;
        uint32_t generation{};
    };

}