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
        Mesh                   ,
    };


    struct ShaderHandle {
        int32_t id{};
        uint32_t generation{};

        bool operator==(const ShaderHandle& a) const {
            return std::tie(id, generation) ==
                std::tie(a.id, a.generation);
        }
    };



}