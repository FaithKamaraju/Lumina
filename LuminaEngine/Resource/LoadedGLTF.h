//
// Created by Faith Kamaraju on 2026-01-25.
//

#pragma once
#include "Rendering/Sampler.h"
#include "Rendering/Image.h"
#include "Rendering/VertexInput.h"
#include "../Scene/Scene.h"
#include <cereal/cereal.hpp>

#include "MeshAsset.h"


namespace LE {

    struct SerializableImage {
        bool bIsSRGB = false;
        uint32_t nrChannels = 3;
        uint64_t hashID{};
        ImageExtent3D extent{};
        std::vector<uint8_t> pixeldata;
        // std::vector<TextureMip> mips;
    };

    struct GLTFImageAndSamplerIDXHolder {
        size_t imageIndex{};
        size_t sampIndex{};
    };

    struct GLTFMetallicRoughnessMaterialData {

        int32_t baseColorTextureIndex = -1;
        int32_t baseColorTextureTexCoord = -1;
        int32_t metallicRoughnessTextureIndex = -1;
        int32_t metallicRoughnessTextureTexCoord = -1;
        glm::vec4 baseColorFactor = glm::vec4(1.0f);
        float metallicFactor = 1.0f;
        float roughnessFactor = 1.0f;
        int32_t normalMapTextureIndex = -1;
        int32_t normalMapTextureTexCoord = -1;
        CullMode cullMode = CullMode::Back;
        // float occlusionStrength = 1.0f;
    };

    struct SubMeshData {
        Topology topology = Topology::Triangles;
        uint32_t vertexOffset{};
        uint32_t indexOffset{};
        uint32_t indexCount{};
        int32_t materialIndex = -1;
    };

    struct MeshAssetData {
        uint32_t vertexCount{};     // unique vertices
        uint32_t indexCount{};      // indices for DrawIndexed
        std::vector<SubMeshData> subMeshes;
    };

    struct SceneNodeData {
        int32_t meshIndex{};
        glm::mat4 localTransform{1.f};
        glm::mat4 globalTransform{1.f};
        Hierarchy hierarchy{};
        std::string debug_name;
    };

    struct LoadedGLTF {

        std::vector<SamplerKey> samplers;
        std::vector<SerializableImage> images;
        std::vector<GLTFImageAndSamplerIDXHolder> textures;
        std::vector<GLTFMetallicRoughnessMaterialData> materials;

        std::vector<uint32_t> indices;
        std::vector<Vertex> vertices;
        std::vector<MeshAssetData> meshes;
        std::vector<SceneNodeData> nodes;
        glm::mat4 transform{1.f};
        std::string scene_debug_name;

        template<class Archive>
        void serialize(Archive & archive)
        {
            archive(samplers,
                images,
                textures,
                materials,
                indices,
                vertices,
                meshes,
                nodes,
                transform,
                scene_debug_name);
        }

    };

    template<class Archive>
    void serialize(Archive& archive, SamplerKey& sampler_key) {
        archive( sampler_key.magFilter, sampler_key.minFilter, sampler_key.mipmapMode,
            sampler_key.addressModeU, sampler_key.addressModeV, sampler_key.addressModeW, sampler_key.mipLodBias,
            sampler_key.anisotropyEnable, sampler_key.maxAnisotropy, sampler_key.compareEnable,
            sampler_key.compareOp, sampler_key.minLod, sampler_key.maxLod, sampler_key.unnormalizedCoordinates,
            sampler_key.hashID);
    }

    template<class Archive>
    void serialize(Archive& archive, ImageExtent2D& extent_2d) {
        archive(extent_2d.width, extent_2d.height);

    }
    template<class Archive>
    void serialize(Archive& archive, ImageExtent3D& extent_3d) {
        archive(extent_3d.width, extent_3d.height, extent_3d.depth);
    }

    template<class Archive>
    void serialize(Archive& archive, SerializableImage& serializable_image) {
        archive(serializable_image.bIsSRGB, serializable_image.nrChannels, serializable_image.hashID,
            serializable_image.extent, serializable_image.pixeldata);
    }

    template<class Archive>
    void serialize(Archive& archive, GLTFImageAndSamplerIDXHolder& holder) {
        archive(holder.imageIndex, holder.sampIndex);
    }

    template<class Archive>
    void serialize(Archive& archive, GLTFMetallicRoughnessMaterialData& mat) {
        archive(mat.baseColorTextureIndex,
                mat.baseColorTextureTexCoord,
                mat.baseColorFactor,
                mat.metallicRoughnessTextureIndex,
                mat.metallicRoughnessTextureTexCoord,
                mat.metallicFactor,
                mat.roughnessFactor,
                mat.normalMapTextureIndex,
                mat.normalMapTextureTexCoord,
                mat.cullMode);
    }

    template<class Archive>
    void serialize(Archive& archive, Vertex& vertex) {
        archive(vertex.position, vertex.padding1_, vertex.normal,
            vertex.padding2_, vertex.uv, vertex.tangent, vertex.color);
    }

    template<class Archive>
    void serialize(Archive& archive, MeshAssetData& mesh_asset) {
        archive(mesh_asset.vertexCount, mesh_asset.indexCount, mesh_asset.subMeshes);
    }

    template<class Archive>
    void serialize(Archive& archive, SubMeshData& sub_mesh) {
        archive(sub_mesh.vertexOffset, sub_mesh.indexOffset, sub_mesh.indexCount, sub_mesh.materialIndex, sub_mesh.topology);
    }

    template<class Archive>
    void serialize(Archive& archive, Hierarchy& hierarchy) {
        archive(hierarchy.parent, hierarchy.firstChild, hierarchy.nextSibling, hierarchy.lastSibling, hierarchy.level);
    }
    template<class Archive>
    void serialize(Archive& archive, SceneNodeData& scene_node) {
        archive(scene_node.meshIndex, scene_node.localTransform, scene_node.globalTransform,
            scene_node.hierarchy, scene_node.debug_name);
    }




}

