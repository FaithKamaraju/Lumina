//
// Created by Faith Kamaraju on 2026-01-15.
//

#include "Renderer.h"

#include "GPUMaterialData.h"
#include "SceneData.h"
#include "Resource/AssetRegistry.h"
#include "Rendering/RHI.h"


LE::Renderer::Renderer(AssetRegistry *registry, RHI* rhi) : mRegistry(registry), mRHI(rhi) {

}

void LE::Renderer::WalkScenegraph(const Scenegraph &scenegraph, float timestep) {
    std::vector<Renderable> renderables;
    renderables.reserve(scenegraph.nodes.size());
    for ( auto& node : scenegraph.nodes) {
        if (node.meshHandle.id == -1) continue;
        auto& mesh = mRegistry->Meshes[node.meshHandle.id];

        for (auto& submesh : mesh.meshAsset.subMeshes) {
            Renderable item{};
            item.indexCount = submesh.indexCount;
            item.indexOffset = submesh.indexOffset;
            item.indexBufferHandle = mesh.meshAsset.indicesBufferHandle;
            item.vertexBufferHandle = mesh.meshAsset.vertexBufferHandle;

            auto& mat = mRegistry->PBR_MR_MaterialInstances[submesh.materialHandle.id];
            GraphicsPipelineDesc desc{};
            desc.topology = submesh.topology;
            desc.vertexHandle = mat.vertexShader;
            desc.fragmentHandle = mat.fragmentShader;
            desc.cullMode = mat.materialInstance.cullMode;

            item.pipelineHandle = getPipelineHandle(desc);
            item.modelMatrix = node.globalTransform;
            item.materialIndex = submesh.materialHandle.id;
            renderables.emplace_back(item);
        }


    }

    mRHI->DrawFrame(renderables, timestep);
}

LE::PipelineHandle LE::Renderer::getPipelineHandle(GraphicsPipelineDesc &desc) {
    if (pipelineCacheCPU.contains(desc)) return pipelineCacheCPU[desc];

    auto handle = mRHI->CreateGraphicsPipeline(desc);
    pipelineCacheCPU[desc] = handle;
    return handle;
}

