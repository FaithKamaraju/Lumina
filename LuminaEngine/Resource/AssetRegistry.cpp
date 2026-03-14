//
// Created by Faith Kamaraju on 2026-02-07.
//

#include "AssetRegistry.h"

#include "Rendering/RenderingConstants.h"
#include "Rendering/RHI.h"

LE::AssetRegistry::AssetRegistry(RHI* rhi) : mRHI(rhi) {

    Textures.reserve(MAX_BINDLESS_TEXTURES * 2);
    PBR_MR_MaterialInstances.reserve(MAX_PBR_MATERIAL_INSTANCES);
    Meshes.reserve(MAX_MESHES_TOTAL);

}

LE::TextureAssetHandle LE::AssetRegistry::RegisterTextureAsset(const TextureAsset &texture_asset) {
    auto handle = bindTextureAsset(texture_asset);
    mRHI->IncrementImageRef(texture_asset.imgHandle);
    mRHI->IncrementSamplerRef(texture_asset.samplerHandle);
    return handle;
}

void LE::AssetRegistry::DeregisterTextureAsset(TextureAssetHandle handle) {
    unbindTextureAsset(handle);
    mRHI->DecrementImageRef(Textures[handle.id].textureAsset.imgHandle);
    mRHI->DecrementSamplerRef(Textures[handle.id].textureAsset.samplerHandle);
}

LE::PBR_MR_MaterialInstanceHandle LE::AssetRegistry::RegisterPBR_MR_MaterialInstance(const PBR_MR_MaterialInstance &pbr_asset) {
    auto handle = bindPBR_MR_MaterialInstance(pbr_asset);
    PBRMaterialGPUData gpuData{};
    createPBRMatGPUData(handle, gpuData);
    mRHI->UpdateMaterialDataSSBOs(gpuData, handle.id);
    mRHI->IncrementShaderModuleRef(DEFAULT_PBR_VERTEX_SHADER);
    mRHI->IncrementShaderModuleRef(DEFAULT_PBR_FRAG_SHADER);
    return handle;
}

void LE::AssetRegistry::DeregisterPBR_MR_MaterialInstance(PBR_MR_MaterialInstanceHandle handle) {
    unbindPBR_MR_MaterialInstance(handle);
    mRHI->UpdateMaterialDataSSBOs({}, handle.id);
    mRHI->DecrementShaderModuleRef(DEFAULT_PBR_VERTEX_SHADER);
    mRHI->DecrementShaderModuleRef(DEFAULT_PBR_FRAG_SHADER);
}

LE::MeshAssetHandle LE::AssetRegistry::RegisterMeshAsset(const MeshAsset &mesh_asset) {
    return bindMeshAsset(mesh_asset);
}

void LE::AssetRegistry::DeregisterMeshAsset(MeshAssetHandle handle) {
    unbindMeshAsset(handle);
}

void LE::AssetRegistry::SetDefaultSamplerHandle(SamplerHandle handle) {
    DEFAULT_SAMPLER_HANDLE = handle;
}

LE::SamplerHandle LE::AssetRegistry::GetDefaultSamplerHandle() {
    return DEFAULT_SAMPLER_HANDLE;
}

void LE::AssetRegistry::SetDefaultCheckboardErrorImageHandle(ImageHandle handle) {
    DEFAULT_CHECKERBOARD_ERROR_IMAGE = handle;
}

LE::ImageHandle LE::AssetRegistry::GetDefaultCheckboardErrorImageHandle() {
    return DEFAULT_CHECKERBOARD_ERROR_IMAGE;
}

void LE::AssetRegistry::SetDefaultPBRShaderHandles(ShaderHandle vert, ShaderHandle frag) {
    DEFAULT_PBR_VERTEX_SHADER= vert;
    DEFAULT_PBR_FRAG_SHADER = frag;
}


// ## private section

// Texture Asset handling

LE::TextureAssetHandle LE::AssetRegistry::bindTextureAsset(const TextureAsset &texture_asset) {
    uint32_t idx = allocTextureAssetIndex(texture_asset);
    Textures[idx].generation++;

    return {.id = (int32_t)idx, .generation = Textures[idx].generation};
}

void LE::AssetRegistry::unbindTextureAsset(TextureAssetHandle handle) {
    if (!isAlive(handle)) return;
    Textures[handle.id].textureAsset = {};
    Textures[handle.id].generation++;
    freeTextureAssetIndex(handle.id);
}

uint32_t LE::AssetRegistry::allocTextureAssetIndex(const TextureAsset &texture_asset) {
    uint32_t idx{};
    if (!m_TextureAssetFreeList.empty())
    {
        idx = m_TextureAssetFreeList.back();
        m_TextureAssetFreeList.pop_back();
        Textures[idx] = {texture_asset};
    }
    else {
        idx = static_cast<uint32_t>(Textures.size());
        Textures.emplace_back(texture_asset, 0);
    }
    return idx;
}

bool LE::AssetRegistry::isAlive(TextureAssetHandle handle) const {
    return (handle.generation == Textures[handle.id].generation);
}

void LE::AssetRegistry::freeTextureAssetIndex(uint32_t idx) {
    m_TextureAssetFreeList.push_back(idx);
}

// PBR MR Material handling

LE::PBR_MR_MaterialInstanceHandle LE::AssetRegistry::bindPBR_MR_MaterialInstance(const PBR_MR_MaterialInstance &pbr_asset) {
    uint32_t idx = allocPBR_MR_MaterialInstanceIndex(pbr_asset);
    PBR_MR_MaterialInstances[idx].generation++;

    return {.id = (int32_t)idx, .generation = PBR_MR_MaterialInstances[idx].generation};
}

void LE::AssetRegistry::unbindPBR_MR_MaterialInstance(PBR_MR_MaterialInstanceHandle handle) {
    if (!isAlive(handle)) return;
    PBR_MR_MaterialInstances[handle.id].materialInstance = {};
    PBR_MR_MaterialInstances[handle.id].generation++;
    freePBR_MR_MaterialInstanceIndex(handle.id);
}

uint32_t LE::AssetRegistry::allocPBR_MR_MaterialInstanceIndex(const PBR_MR_MaterialInstance &pbr_asset) {
    uint32_t idx{};
    if (!m_PBR_MR_MaterialAssetFreeList.empty())
    {
        idx = m_PBR_MR_MaterialAssetFreeList.back();
        m_PBR_MR_MaterialAssetFreeList.pop_back();
        PBR_MR_MaterialInstances[idx] = {pbr_asset, DEFAULT_PBR_VERTEX_SHADER, DEFAULT_PBR_FRAG_SHADER};
    }
    else {
        idx = static_cast<uint32_t>(PBR_MR_MaterialInstances.size());
        PBR_MR_MaterialInstances.emplace_back(pbr_asset, DEFAULT_PBR_VERTEX_SHADER ,
            DEFAULT_PBR_FRAG_SHADER, 0);
    }
    return idx;
}

bool LE::AssetRegistry::isAlive(PBR_MR_MaterialInstanceHandle handle) const {
    return (handle.generation == PBR_MR_MaterialInstances[handle.id].generation);
}

void LE::AssetRegistry::freePBR_MR_MaterialInstanceIndex(uint32_t idx) {
    m_PBR_MR_MaterialAssetFreeList.push_back(idx);
}

void LE::AssetRegistry::createPBRMatGPUData(PBR_MR_MaterialInstanceHandle handle,PBRMaterialGPUData& data) {

    auto instance = PBR_MR_MaterialInstances[handle.id].materialInstance;

    data.baseColorFactor[0] = instance.baseColorFactor.x;
    data.baseColorFactor[1] = instance.baseColorFactor.y;
    data.baseColorFactor[2] = instance.baseColorFactor.z;
    data.baseColorFactor[3] = instance.baseColorFactor.w;
    data.metallicFactor = instance.metallicFactor;
    data.roughnessFactor = instance.roughnessFactor;
    data.baseColorImageIndex = static_cast<uint32_t>(Textures[instance.baseColorTextureAsset.id].textureAsset.imgHandle.id);
    data.baseColorImageSamplerIndex = static_cast<uint32_t>(Textures[instance.baseColorTextureAsset.id].textureAsset.samplerHandle.id);
    data.baseColorTextureTexCoord = instance.baseColorTextureTexCoord;
    data.metallicRoughnessImageIndex = static_cast<uint32_t>(Textures[instance.metallicRoughnessTextureAsset.id].textureAsset.imgHandle.id);
    data.metallicRoughnessImageSamplerIndex = static_cast<uint32_t>(Textures[instance.metallicRoughnessTextureAsset.id].textureAsset.samplerHandle.id);
    data.metallicRoughnessTextureTexCoord = instance.metallicRoughnessTextureTexCoord;
    data.normalMapImageIndex = static_cast<uint32_t>(Textures[instance.normalMapTextureAsset.id].textureAsset.imgHandle.id);
    data.normalMapImageSamplerIndex = static_cast<uint32_t>(Textures[instance.normalMapTextureAsset.id].textureAsset.samplerHandle.id);
    data.normalMapTextureTexCoord = instance.normalMapTextureTexCoord;

}

// Mesh Asset handling

LE::MeshAssetHandle LE::AssetRegistry::bindMeshAsset(const MeshAsset &mesh_asset) {
    uint32_t idx = allocMeshAssetIndex(mesh_asset);
    Meshes[idx].generation++;

    return {.id = (int32_t)idx, .generation = Meshes[idx].generation};
}

void LE::AssetRegistry::unbindMeshAsset(MeshAssetHandle handle) {
    if (!isAlive(handle)) return;
    Meshes[handle.id].meshAsset = {};
    Meshes[handle.id].generation++;
    freeMeshAssetIndex(handle.id);
}

uint32_t LE::AssetRegistry::allocMeshAssetIndex(const MeshAsset &mesh_asset) {
    uint32_t idx{};
    if (!m_MeshAssetFreeList.empty())
    {
        idx = m_MeshAssetFreeList.back();
        m_MeshAssetFreeList.pop_back();
        Meshes[idx] = {mesh_asset};
    }
    else {
        idx = static_cast<uint32_t>(Meshes.size());
        Meshes.emplace_back(mesh_asset, 0);
    }
    return idx;
}

bool LE::AssetRegistry::isAlive(MeshAssetHandle handle) const {
    return (handle.generation == Meshes[handle.id].generation);
}

void LE::AssetRegistry::freeMeshAssetIndex(uint32_t idx) {
    m_MeshAssetFreeList.push_back(idx);
}
