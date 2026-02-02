//
// Created by Faith Kamaraju on 2026-01-24.
//

#pragma once


namespace LE {

    enum class ShaderStage
    {
        Vertex                 ,
        TessellationControl    ,
        TessellationEvaluation ,
        Geometry               ,
        Fragment               ,
        Compute                ,
        AllGraphics            ,
        Mesh                   ,
    };


    struct ShaderHandle {
        uint32_t id{};
        uint32_t generation{};
    };

    struct ShaderStageDesc {
        ShaderStage stage{};
        ShaderHandle handle{};
    };


}