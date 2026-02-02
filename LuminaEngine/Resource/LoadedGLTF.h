//
// Created by Faith Kamaraju on 2026-01-25.
//

#pragma once
#include "Core/LE_Types.h"
#include "Rendering/Buffer.h"
#include "Rendering/Sampler.h"
#include "Rendering/Image.h"
#include "MeshAsset.h"
#include "Rendering/VertexInput.h"
#include "TextureAsset.h"
#include "MaterialAsset.h"
#include "Scene.h"
#include <cereal/cereal.hpp>


namespace LE {

    struct GLTFImageAndSamplerIDXHolder {
        size_t imageIndex{};
        size_t sampIndex{};
    };


    struct LoadedGLTF {

        LoadedGLTF() = default;

        std::vector<SamplerKey> samplers;
        std::vector<SerializableImage> images;
        std::vector<GLTFImageAndSamplerIDXHolder> textures;
        std::vector<GLTFMetallicRoughnessMaterialAsset> materials;

        std::vector<uint32_t> indices;
        std::vector<Vertex> vertices;
        std::vector<MeshAsset> meshes;
        std::vector<SceneNode> nodes;
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
            sampler_key.compareOp, sampler_key.minLod, sampler_key.maxLod, sampler_key.unnormalizedCoordinates);
    }

    template<class Archive>
    void serialize(Archive& archive, SerializableImage& serializable_image) {
        archive(serializable_image.format, serializable_image.extent, serializable_image.pixeldata);
    }

    template<class Archive>
    void serialize(Archive& archive, GLTFImageAndSamplerIDXHolder& holder) {
        archive(holder.imageIndex, holder.sampIndex);
    }

    template<class Archive>
    void serialize(Archive& archive, GLTFMetallicRoughnessMaterialAsset& mat) {
        archive(mat.baseColorTextureIndex, mat.baseColorTextureTexCoord,
            mat.baseColorFactor, mat.metallicRoughnessTextureIndex,
            mat.metallicRoughnessTextureTexCoord, mat.metallicFactor, mat.roughnessFactor);
    }

    template<class Archive>
    void serialize(Archive& archive, Vertex& vertex) {
        archive(vertex.position, vertex.padding1_, vertex.normal,
            vertex.padding2_, vertex.uv, vertex.tangent, vertex.color);
    }

    template<class Archive>
    void serialize(Archive& archive, MeshAsset& mesh_asset) {
        archive(mesh_asset.vertexCount, mesh_asset.indexCount, mesh_asset.subMeshes, mesh_asset.materials);
    }

    template<class Archive>
    void serialize(Archive& archive, SubMesh& sub_mesh) {
        archive(sub_mesh.vertexOffset, sub_mesh.indexOffset, sub_mesh.indexCount, sub_mesh.materialIndex);
    }

    template<class Archive>
    void serialize(Archive& archive, Hierarchy& hierarchy) {
        archive(hierarchy.parent, hierarchy.firstChild, hierarchy.nextSibling, hierarchy.lastSibling, hierarchy.level);
    }
    template<class Archive>
    void serialize(Archive& archive, SceneNode& scene_node) {
        archive(scene_node.meshIndex, scene_node.localTransform, scene_node.globalTransform,
            scene_node.hierarchy, scene_node.debug_name);
    }




}

