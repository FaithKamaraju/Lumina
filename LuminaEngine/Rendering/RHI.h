//
// Created by Faith Kamaraju on 2026-01-14.
//

#pragma once

#include "Core/LE_Types.h"
#include "Image.h"
#include "Buffer.h"
#include "Sampler.h"
#include "Pipeline.h"
#include "VertexInput.h"
#include "Rendering/Renderable.h"

struct GLFWwindow;

namespace LE {



    class RHI {

    public:
        virtual ~RHI() = default;

        virtual LEBool InitDevice(GLFWwindow* windowHandle) = 0;
        virtual LEBool InitImgui() = 0;
        virtual BufferHandle AllocateAndCopyBufferFromBytes(const std::byte* dataSource, size_t dataSize, BufferUsageFlags usage) = 0;
        virtual BufferHandle AllocateAndCopyVertexBuffer(const std::vector<Vertex>& vertices) = 0;
        virtual BufferHandle AllocateAndCopyIndexBuffer(const std::vector<uint32_t>& indices) = 0;
        virtual BufferHandle AllocateAndCopyUniformBuffer(size_t uniformBufferSize) = 0;
        virtual void CopyUniformData(BufferHandle ub, void* uniformData, size_t dataSize) = 0;
        virtual void DeallocateBuffer(BufferHandle handle) = 0;
        virtual ImageHandle CreateImage(size_t hashedName, unsigned char* data, uint32_t nrChannels, ImageExtent3D imagesize, ImageFormat format,
                            ImageUsageFlags imageUsage, bool mipmapped) = 0;
        virtual void DestroyImage(ImageHandle handle) = 0;
        virtual SamplerHandle CreateImageSampler(SamplerKey samplerInfo) = 0;
        virtual void DestroyImageSampler(SamplerHandle samplerHandle) = 0;
        virtual PipelineHandle CreateGraphicsPipeline(GraphicsPipelineDesc desc) = 0;
        virtual PipelineHandle CreateComputePipeline(ComputePipelineDesc desc) = 0;

        virtual void ProcessDeferredFrees() = 0;

        virtual void DrawFrame(const std::vector<Renderable>& renderables, float timestep) = 0; // TO:DEFINE Draw frame with a list of renderables I get from the scenegraph!


    };

}
