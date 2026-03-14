//
// Created by Faith Kamaraju on 2026-01-15.
//

#pragma once
#include "Renderable.h"
#include "RenderingConstants.h"
#include "Rendering/Pipeline.h"


namespace LE {

    class AssetRegistry;
    class RHI;


    class Renderer {

    public:
        Renderer(AssetRegistry* registry, RHI* rhi);
        ~Renderer() = default;

        void WalkScenegraph(float timestep);





    private:

        PipelineHandle getPipelineHandle(GraphicsPipelineDesc& desc);

        AssetRegistry* mRegistry = nullptr;
        RHI* mRHI = nullptr;

        std::unordered_map<GraphicsPipelineDesc, PipelineHandle,
        GraphicsPipelineHasher, GraphicsPipelineEqPred> pipelineCacheCPU{};




    };

}
