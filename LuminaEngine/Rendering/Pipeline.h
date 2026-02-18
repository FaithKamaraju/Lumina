//
// Created by Faith Kamaraju on 2026-01-23.
//

#pragma once
#include "Shader.h"
#include <fastgltf/core.hpp>
#include "Core/UtilFunctions.h"

namespace LE {

    enum class PipelineType : uint8_t {
        Graphics,
        Compute
    };

    enum class PolygonMode : uint8_t
    {
        Fill            ,
        Line            ,
        Point           ,
    };

    enum class Topology : std::uint8_t {
        Points = 0,
        Lines = 1,
        LineLoop = 2,
        LineStrip = 3,
        Triangles = 4,
        TriangleStrip = 5,
        TriangleFan = 6,
    };

    inline Topology getPrimitiveType(fastgltf::PrimitiveType mode) {
        switch (mode) {
            case fastgltf::PrimitiveType::Points:
                return Topology::Points;
            case fastgltf::PrimitiveType::Lines:
                return Topology::Lines;
            case fastgltf::PrimitiveType::LineLoop:
                return Topology::LineLoop;
            case fastgltf::PrimitiveType::LineStrip:
                return Topology::LineStrip;
            case fastgltf::PrimitiveType::Triangles:
                return Topology::Triangles;
            case fastgltf::PrimitiveType::TriangleStrip:
                return Topology::TriangleStrip;
            case fastgltf::PrimitiveType::TriangleFan:
                return Topology::TriangleFan;
        }
        return Topology::Triangles;
    }

    enum class CullMode : uint8_t
    {
        None         ,
        Front        ,
        Back         ,
        FrontAndBack
    };

    enum class FrontFace : uint8_t
    {
        CounterClockwise ,
        Clockwise
    };

    // struct RasterState {
    //
    // };
    //
    // struct DepthStencilState {
    //
    // };
    //
    // struct BlendState {
    //
    // };


    // struct PipelineLayoutHandle {
    //     int32_t id = -1;
    //     uint32_t generation{};
    // };



    struct GraphicsPipelineDesc {

        ShaderHandle vertexHandle{};
        ShaderHandle fragmentHandle{};

        CullMode cullMode = CullMode::Back;
        LE::Topology topology = LE::Topology::Triangles;
        // RasterState raster;
        // DepthStencilState depthStencil;
        // BlendState blend;

        // bool operator==(const GraphicsPipelineDesc& a) const {
        //     return std::tie(vertexHandle, fragmentHandle, cullMode, topology) ==
        //         std::tie(a.vertexHandle, a.fragmentHandle, a.cullMode, a.topology);
        // }


    };

    struct GraphicsPipelineHasher {

        size_t operator()(const GraphicsPipelineDesc& p) const {
            size_t hash{};
            HashCombine(hash, std::hash<int32_t>{}(p.vertexHandle.id));
            HashCombine(hash, std::hash<int32_t>{}(p.fragmentHandle.id));
            HashCombine(hash, std::hash<CullMode>{}(p.cullMode));
            HashCombine(hash, std::hash<LE::Topology>{}(p.topology));
            return hash;
        }
    };

    struct GraphicsPipelineEqPred {

        bool operator()(const GraphicsPipelineDesc& a, const GraphicsPipelineDesc& b) const {
            return std::tie(a.vertexHandle, a.fragmentHandle, a.cullMode, a.topology) ==
                std::tie(b.vertexHandle, b.fragmentHandle, b.cullMode, b.topology);
        }

    };

    struct ComputePipelineDesc {

    };

    struct PipelineHandle {
        int32_t id = -1;
        uint32_t generation{};

        bool operator==(const PipelineHandle& a) const {
            return std::tie(id, generation) ==
                std::tie(a.id, a.generation);
        }

        bool operator<(const PipelineHandle& other) const {
            return std::tie(id, generation) < std::tie(other.id, other.generation);

        }

    };

}