//
// Created by Faith Kamaraju on 2026-02-07.
//

#pragma once
#include "MaterialAsset.h"
#include "TextureAsset.h"
#include "MeshAsset.h"
#include "Rendering/GPUMaterialData.h"

namespace LE {

    struct TextureAssetSlot {
        TextureAsset textureAsset{};
        uint32_t generation{};
    };

    struct PBR_MR_MaterialInstanceSlot {
        PBR_MR_MaterialInstance materialInstance{};
        ShaderHandle vertexShader{};
        ShaderHandle fragmentShader{};
        uint32_t generation{};
    };

    struct MeshAssetSlot {
        MeshAsset meshAsset{};
        uint32_t generation{};
    };

    class RHI;

    class AssetRegistry {

    public:
        AssetRegistry(RHI* rhi);
        ~AssetRegistry() = default;

        TextureAssetHandle RegisterTextureAsset(const TextureAsset& texture_asset);
        void DeregisterTextureAsset(TextureAssetHandle handle);

        PBR_MR_MaterialInstanceHandle RegisterPBR_MR_MaterialInstance(const PBR_MR_MaterialInstance& pbr_asset);
        void DeregisterPBR_MR_MaterialInstance(PBR_MR_MaterialInstanceHandle handle);

        MeshAssetHandle RegisterMeshAsset(const MeshAsset& mesh_asset);
        void DeregisterMeshAsset(MeshAssetHandle handle);

        // TODO Get fn for materials, textures, meshes;



        void SetDefaultPBRShaderHandles(ShaderHandle vert, ShaderHandle frag);


        std::vector<TextureAssetSlot> Textures;
        std::vector<PBR_MR_MaterialInstanceSlot> PBR_MR_MaterialInstances;
        std::vector<MeshAssetSlot> Meshes;

    private:

        // Texture Asset handling
        TextureAssetHandle bindTextureAsset(const TextureAsset& texture_asset);
        void unbindTextureAsset(TextureAssetHandle handle);
        uint32_t allocTextureAssetIndex(const TextureAsset& texture_asset);
        bool isAlive(TextureAssetHandle handle) const;
        void freeTextureAssetIndex(uint32_t idx);

        // PBR_MR Material Asset handling
        PBR_MR_MaterialInstanceHandle bindPBR_MR_MaterialInstance(const PBR_MR_MaterialInstance& pbr_asset);
        void unbindPBR_MR_MaterialInstance(PBR_MR_MaterialInstanceHandle handle);
        uint32_t allocPBR_MR_MaterialInstanceIndex(const PBR_MR_MaterialInstance& pbr_asset);
        bool isAlive(PBR_MR_MaterialInstanceHandle handle) const;
        void freePBR_MR_MaterialInstanceIndex(uint32_t idx);

        void createPBRMatGPUData(PBR_MR_MaterialInstanceHandle handle,PBRMaterialGPUData& data);

        // Mesh Asset handling
        MeshAssetHandle bindMeshAsset(const MeshAsset& mesh_asset);
        void unbindMeshAsset(MeshAssetHandle handle);
        uint32_t allocMeshAssetIndex(const MeshAsset& mesh_asset);
        bool isAlive(MeshAssetHandle handle) const;
        void freeMeshAssetIndex(uint32_t idx);

        RHI* mRHI = nullptr;

        ShaderHandle DEFAULT_PBR_VERTEX_SHADER{};
        ShaderHandle DEFAULT_PBR_FRAG_SHADER{};


        std::vector<uint32_t> m_TextureAssetFreeList;
        std::vector<uint32_t> m_PBR_MR_MaterialAssetFreeList;
        std::vector<uint32_t> m_MeshAssetFreeList;


    };

}
