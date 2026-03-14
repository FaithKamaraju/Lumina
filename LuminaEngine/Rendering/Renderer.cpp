//
// Created by Faith Kamaraju on 2026-01-15.
//

#include "Renderer.h"

#include "Core/EngineStatics.h"
#include "ECS/Coordinator.h"
#include "Resource/AssetRegistry.h"
#include "Rendering/RHI.h"
#include "Scene/Scenegraph.h"


LE::Renderer::Renderer(AssetRegistry *registry, RHI* rhi) : mRegistry(registry), mRHI(rhi) {

}

void LE::Renderer::WalkScenegraph(float timestep) {
    std::vector<Renderable> renderables;
    auto scenegraph = Globals::GetCurrentSceneGraph();
    renderables.reserve(scenegraph->mNodeHierarchies.size());
    for ( auto& node : scenegraph->mNodeHierarchies) {
        auto& meshHandle = Globals::GetCoordinator()->GetComponent<MeshAssetHandle>(node.entityID);
        if (meshHandle.id == -1) continue;
        auto& mesh = mRegistry->Meshes[meshHandle.id];

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
            auto& transform = Globals::GetCoordinator()->GetComponent<TransformComponent>(node.entityID);
            item.modelMatrix = transform.globalTransform;
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

