//
// Created by Faith Kamaraju on 2026-01-23.
//

#pragma once
#include "Image.h"
#include "Shader.h"

namespace LE {

    enum class PipelineType {
        Graphics,
        Compute
    };

    enum class PolygonMode
    {
        Fill            ,
        Line            ,
        Point           ,
    };

    enum class PrimitiveTopology
    {
        PointList                  ,
        LineList                   ,
        LineStrip                  ,
        TriangleList               ,
        TriangleStrip              ,
        TriangleFan                ,
        LineListWithAdjacency      ,
        LineStripWithAdjacency     ,
        TriangleListWithAdjacency  ,
        TriangleStripWithAdjacency ,
        PatchList
    };

    enum class CullModeFlagBits
    {
        None         ,
        Front        ,
        Back         ,
        FrontAndBack
    };

    enum class FrontFace
    {
        CounterClockwise ,
        Clockwise
    };

    struct RasterState {

    };

    struct DepthStencilState {

    };

    struct BlendState {

    };


    struct PipelineLayoutHandle {
        uint32_t id{};
        uint32_t generation{};
    };

    struct PipelineHandle {
        uint32_t id{};
        uint32_t generation{};
    };

    struct GraphicsPipelineDesc {

        std::vector<ShaderStageDesc> shaderStages{};

        // Dynamic rendering compatible
        std::vector<ImageFormat> colorFormats;
        ImageFormat depthFormat;

        RasterState raster;
        DepthStencilState depthStencil;
        BlendState blend;

        PrimitiveTopology topology;

    };

    struct ComputePipelineDesc {

    };

}