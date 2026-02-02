//
// Created by Faith Kamaraju on 2026-01-15.
//

#include "ResourceManager.h"
#include <cereal/cereal.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <stb_image.h>
#include <fastgltf/tools.hpp>
#include "Core/MathUtils.h"
#include "Core/Logger.h"
#include "TextureAsset.h"
#include "Rendering/RenderingConstants.h"
#include "Core/CerealUtils.h"
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/optional.hpp>
#include <cereal/types/memory.hpp>

LE::Filter LE::ResourceManager::extractFilter(fastgltf::Filter filter)
{
    switch (filter) {

        case fastgltf::Filter::Nearest:                 return LE::Filter::Nearest;
        case fastgltf::Filter::Linear:
        default:                                        return LE::Filter::Linear;
    }
}

LE::SamplerMipmapMode LE::ResourceManager::extractMipmapMode(fastgltf::Filter filter)
{
    switch (filter) {
        case fastgltf::Filter::NearestMipMapNearest:
        case fastgltf::Filter::LinearMipMapNearest:     return LE::SamplerMipmapMode::Nearest;

        case fastgltf::Filter::NearestMipMapLinear:
        case fastgltf::Filter::LinearMipMapLinear:
        default:                                        return LE::SamplerMipmapMode::Linear;
    }
}

LE::SamplerAddressMode LE::ResourceManager::extractSamplerAddressMode(fastgltf::Wrap wrap) {

    switch (wrap) {
        case fastgltf::Wrap::ClampToEdge:
            return SamplerAddressMode::ClampToEdge;
        case fastgltf::Wrap::MirroredRepeat:
            return SamplerAddressMode::MirroredRepeat;
        case fastgltf::Wrap::Repeat:
            return SamplerAddressMode::Repeat;
        default:
            return SamplerAddressMode::ClampToEdge;
    }
}

LE::ImageFormat LE::ResourceManager::findSuitableSRGBorUNORMFormat(int nrChannels, bool bIsSRGB) {


    if (bIsSRGB) {
        switch (nrChannels) {
            case 1: return ImageFormat::R8_SRGB;
            case 3: return ImageFormat::R8G8B8_SRGB;
            case 4: return ImageFormat::R8G8B8A8_SRGB;
            default: return ImageFormat::Undefined;
        }
    }
    else {
        switch (nrChannels) {
            case 1: return ImageFormat::R8_UNorm;
            case 3: return ImageFormat::R8G8B8_UNorm;
            case 4: return ImageFormat::R8G8B8A8_UNorm;
            default: return ImageFormat::Undefined;
        }
    }
}

LE::ResourceManager::ResourceManager(RHI* rhi) : mRHI(rhi) {

    InitializeShader_cCompiler();

}

void LE::ResourceManager::InitializeShader_cCompiler(shaderc_optimization_level optimizationLevel,
                                              shaderc_source_language sourceLanguage) {

    g_CompileOptions.SetOptimizationLevel(optimizationLevel);
    g_CompileOptions.SetSourceLanguage(sourceLanguage);

}

std::vector<char> LE::ResourceManager::ReadShaderSource(const char *filePath) {

    std::ifstream file(filePath, std::ios::ate);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), static_cast<std::streamsize>(fileSize));

    file.close();
    return buffer;
}

std::vector<char> LE::ResourceManager::ReadSPVFile(const std::string &filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), static_cast<std::streamsize>(fileSize));

    file.close();
    return buffer;
}

std::vector<uint32_t> LE::ResourceManager::CompileShader(const char *code, shaderc_shader_kind shaderKind,
    const char *source_name, const shaderc::CompileOptions *compileOptions) {
    InitializeShader_cCompiler();
    auto spvBinary= g_ShaderCompiler.CompileGlslToSpv(code, shaderKind,
                                                       source_name, g_CompileOptions);

    if (spvBinary.GetCompilationStatus() != shaderc_compilation_status_success) {
        std::cerr << spvBinary.GetErrorMessage();
        return {};
    }

    return {spvBinary.cbegin(), spvBinary.cend()};
}

LEBool LE::ResourceManager::ImportGLTFFile(const std::filesystem::path& filepath, std::string outAssetName) {

    auto gltfScene = loadGLTFFileIntoCPUMemory(filepath);

    if (!gltfScene.has_value()) {
        LE_CORE_INFO("Couldn't load the gltf file from filepath: {0}", filepath.c_str());
        return LE_FAILURE;
    }

    if (outAssetName.empty()) {
        outAssetName = std::string(filepath.stem()) + ".LEASSET";
    }
    if (outAssetName.find(".LEASSET") == std::string::npos) {
        outAssetName += ".LEASSET";
    }
    std::ofstream file{"../../../Assets/" + outAssetName, std::ios::binary};
    {
        cereal::PortableBinaryOutputArchive oarchive(file);
        oarchive(gltfScene.value());
    }

    return LE_SUCCESS;

}

void LE::ResourceManager::LoadSceneAsset(const std::filesystem::path &filepath) {

    std::ifstream file{filepath, std::ios::binary};
    Ref<LoadedGLTF> gltf;
    {
        cereal::PortableBinaryInputArchive iarchive(file);
        iarchive(gltf);
    }


}

std::optional<LE::Ref<LE::LoadedGLTF>> LE::ResourceManager::loadGLTFFileIntoCPUMemory(const std::filesystem::path& filepath) {

    auto scene = CreateRef<LoadedGLTF>();
    LoadedGLTF& file = *scene.get();

    constexpr auto gltfOptions = fastgltf::Options::DontRequireValidAssetMember | fastgltf::Options::LoadExternalBuffers
    | fastgltf::Options::LoadExternalImages;

    auto data = fastgltf::GltfDataBuffer::FromPath(filepath);
    if (data.error() != fastgltf::Error::None) {
        // The file couldn't be loaded, or the buffer could not be allocated.
        std::cerr << "Failed to load glTF: " << fastgltf::to_underlying(data.error()) << std::endl;
        return {};
    }

    fastgltf::Asset gltf;
    auto load =  parser.loadGltf(data.get(), filepath.parent_path(), gltfOptions);
    if (load) {
        gltf = std::move(load.get());
    } else {
        LE_CORE_ERROR("Failed to load glTF: {0}", fastgltf::to_underlying(load.error()));
        return {};
    }

    for (fastgltf::Sampler& sampler : gltf.samplers) {
        loadSampler(file, gltf, sampler);
    }

    for (fastgltf::Image& image : gltf.images) {
        loadImage(file, gltf, image);
    }

    for (fastgltf::Texture& texture : gltf.textures) {
        GLTFImageAndSamplerIDXHolder holder{};
        holder.imageIndex = texture.imageIndex.value();
        holder.sampIndex = texture.samplerIndex.value();
        file.textures.push_back(holder);
    }


    for (fastgltf::Material& mat: gltf.materials) {

        GLTFMetallicRoughnessMaterialAsset material_asset{};
        // tex.texCoord = mat.pbrData.baseColorTexture->texCoordIndex;

        if (mat.pbrData.baseColorTexture.has_value()) {
            material_asset.baseColorTextureIndex = mat.pbrData.baseColorTexture.value().textureIndex;
            material_asset.baseColorTextureTexCoord = mat.pbrData.baseColorTexture->texCoordIndex;
        }

        if (mat.pbrData.metallicRoughnessTexture.has_value()) {
            material_asset.metallicRoughnessTextureIndex = mat.pbrData.metallicRoughnessTexture.value().textureIndex;
            material_asset.metallicRoughnessTextureTexCoord = mat.pbrData.metallicRoughnessTexture->texCoordIndex;
        }
        // if (mat.normalTexture.has_value()) {
        //     auto tex_idx = mat.normalTexture->textureIndex;
        // }
        //
        // if (mat.occlusionTexture.has_value()) {
        //     auto tex_idx = mat.occlusionTexture->textureIndex;
        // }
        material_asset.baseColorFactor.r  = mat.pbrData.baseColorFactor[0];
        material_asset.baseColorFactor.g  = mat.pbrData.baseColorFactor[1];
        material_asset.baseColorFactor.b  = mat.pbrData.baseColorFactor[2];
        material_asset.baseColorFactor.a  = mat.pbrData.baseColorFactor[3];
        material_asset.metallicFactor = mat.pbrData.metallicFactor;
        material_asset.roughnessFactor = mat.pbrData.roughnessFactor;

        file.materials.push_back(material_asset);
    }

    size_t verticesCount = 0;
    size_t indicesCount = 0;
    for (fastgltf::Mesh& mesh : gltf.meshes) {

        for (auto&& p : mesh.primitives) {
            fastgltf::Accessor& indexAcc = gltf.accessors[p.indicesAccessor.value()];
            indicesCount += indexAcc.count;

            fastgltf::Accessor& posAccessor = gltf.accessors[p.findAttribute("POSITION")->accessorIndex];
            verticesCount += posAccessor.count;
        }
    }
    file.vertices.resize(verticesCount);
    file.meshes.reserve(gltf.meshes.size());
    file.indices.reserve(indicesCount);
    size_t initial_vtx = 0;
    for (fastgltf::Mesh& mesh : gltf.meshes) {

        MeshAsset newMesh{};

        for (auto&& p : mesh.primitives) {
            SubMesh subMesh{};
            subMesh.vertexOffset = initial_vtx;
            subMesh.indexOffset = file.indices.size();
            subMesh.indexCount = gltf.accessors[p.indicesAccessor.value()].count;
            newMesh.indexCount += subMesh.indexCount;
            if (p.materialIndex.has_value()) {
                subMesh.materialIndex = p.materialIndex.value();
            }
            // load indexes
            {
                fastgltf::Accessor& indexaccessor = gltf.accessors[p.indicesAccessor.value()];
                fastgltf::iterateAccessor<std::uint32_t>(gltf, indexaccessor,
                    [&](std::uint32_t idx) {
                        file.indices.push_back(idx + subMesh.vertexOffset);
                    });
            }
            // load vertex positions
            {
                fastgltf::Accessor& posAccessor = gltf.accessors[p.findAttribute("POSITION")->accessorIndex];
                initial_vtx += posAccessor.count;
                fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, posAccessor,
                    [&](glm::vec3 v, size_t index) {
                        Vertex newvtx{};
                        newvtx.position = v;
                        newvtx.normal = { 1, 0, 0 };
                        newvtx.color = glm::vec4 { 1.f };
                        for (auto & i : newvtx.uv) {
                            i = {0.f,0.f};
                        }
                        file.vertices[subMesh.vertexOffset + index] = newvtx;
                        newMesh.vertexCount += 1;
                    });
            }
            // load vertex normals
            auto normals = p.findAttribute("NORMAL");
            if (normals != p.attributes.end()) {
                fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, gltf.accessors[(*normals).accessorIndex],
                    [&](glm::vec3 v, size_t index) {
                        file.vertices[subMesh.vertexOffset + index].normal = v;
                    });
            }
            // load UVs
                std::string texCoordStr = "TEXCOORD_";
            for (int i = 0; i < MAX_NUM_TEX_COORDS; i++) {
                auto uv = p.findAttribute(texCoordStr + std::to_string(i));
                if (uv != p.attributes.end()) {
                    fastgltf::iterateAccessorWithIndex<glm::vec2>(gltf, gltf.accessors[(*uv).accessorIndex],
                        [&](glm::vec2 v, size_t index) {
                            file.vertices[subMesh.vertexOffset + index].uv[i].x = v.x;
                            file.vertices[subMesh.vertexOffset + index].uv[i].y = v.y;
                        });
                }
            }
            // load tangent
            auto tangent = p.findAttribute("TANGENT");
            if (tangent != p.attributes.end()) {
                fastgltf::iterateAccessorWithIndex<glm::vec4>(gltf, gltf.accessors[(*tangent).accessorIndex],
                    [&](glm::vec4 v, size_t index) {
                        file.vertices[subMesh.vertexOffset + index].tangent = v;
                    });
            }
            // load vertex colors
            auto colors = p.findAttribute("COLOR_0");
            if (colors != p.attributes.end()) {
                fastgltf::iterateAccessorWithIndex<glm::vec4>(gltf, gltf.accessors[(*colors).accessorIndex],
                    [&](glm::vec4 v, size_t index) {
                        file.vertices[subMesh.vertexOffset + index].color = v;
                    });
            }
            newMesh.subMeshes.push_back(subMesh);

        }
        file.meshes.push_back(newMesh);
    }

    file.scene_debug_name = gltf.scenes[0].name;
    file.nodes.reserve(gltf.scenes[0].nodeIndices.size());

    auto traverse = [&](size_t nodeIndex, int parent, int atLevel, fastgltf::math::fmat4x4 nodeMatrix, auto& self) -> void {
        assert(gltf.nodes.size() > nodeIndex);
        SceneNode newSceneNode{};
        auto& node = gltf.nodes[nodeIndex];
        nodeMatrix = fastgltf::getTransformMatrix(node, nodeMatrix);
        if (node.meshIndex.has_value()) newSceneNode.meshIndex = node.meshIndex.value();
        newSceneNode.debug_name = node.name;
        Math::toGLM(newSceneNode.localTransform, fastgltf::getLocalTransformMatrix(node));
        Math::toGLM(newSceneNode.globalTransform, nodeMatrix);

        newSceneNode.hierarchy.parent = parent;
        // newSceneNode.hierarchy.firstChild = node.children.empty() ? -1 : static_cast<int>(node.children[0]);
        newSceneNode.hierarchy.level = atLevel;
        if (parent > -1) {
            const int parentsFirstChild = file.nodes[parent].hierarchy.firstChild;
            if (parentsFirstChild == -1) {
                file.nodes[parent].hierarchy.firstChild = static_cast<int>(nodeIndex) ;
                file.nodes[nodeIndex].hierarchy.lastSibling = static_cast<int>(nodeIndex);
            }
            else {
                int dest = file.nodes[parentsFirstChild].hierarchy.lastSibling;
                if (dest <= -1) {
                    for (dest = parentsFirstChild;
                         file.nodes[dest].hierarchy.nextSibling !=-1;
                         dest = file.nodes[dest].hierarchy.nextSibling);
                }
                file.nodes[dest].hierarchy.nextSibling = static_cast<int>(nodeIndex);
                file.nodes[parentsFirstChild].hierarchy.lastSibling = static_cast<int>(nodeIndex);
            }
        }

        file.nodes.push_back(newSceneNode);
        for (auto& child : node.children) {
            self(child, nodeIndex, atLevel+1, nodeMatrix, self);
        }
    };

    traverse(gltf.scenes[0].nodeIndices[0] ,-1,0,fastgltf::math::fmat4x4{1.f},traverse);

    return scene;
}


void LE::ResourceManager::loadImage(LoadedGLTF& file ,fastgltf::Asset &asset, fastgltf::Image &image) {

    // size_t hashedName = pmrStringHasher(image.name);

    int width, height, nrChannels;
    std::visit(
        fastgltf::visitor {
            [](auto& arg) {},
            [&](fastgltf::sources::URI& filePath) {
                assert(filePath.fileByteOffset == 0); // We don't support offsets with stbi.
                assert(filePath.uri.isLocalPath()); // We're only capable of loading
                // local files.

                const std::string path(filePath.uri.path().begin(),
                                       filePath.uri.path().end()); // Thanks C++.
                unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
                if (data) {
                    file.images.push_back({});
                    SerializableImage& tex = file.images.back();
                    tex.pixeldata.resize(width * height * nrChannels);
                    tex.extent = {
                        .width = static_cast<uint32_t>(width),
                        .height = static_cast<uint32_t>(height),
                        .depth = 1
                    };
                    memcpy(tex.pixeldata.data(), data, width * height * nrChannels );
                    stbi_image_free(data);
                }
            },
            [&](fastgltf::sources::Vector& vector) {
                unsigned char* data = stbi_load_from_memory(reinterpret_cast<stbi_uc const *>(vector.bytes.data()), static_cast<int>(vector.bytes.size()),
                                                            &width, &height, &nrChannels, 0);
                if (data) {
                    file.images.push_back({});
                    SerializableImage& tex = file.images.back();
                    tex.pixeldata.resize(vector.bytes.size());
                    tex.extent = {
                       .width = static_cast<uint32_t>(width),
                       .height = static_cast<uint32_t>(height),
                       .depth = 1
                   };
                   memcpy(tex.pixeldata.data(), data, vector.bytes.size() );

                    stbi_image_free(data);
                }
            },
            [&](fastgltf::sources::Array& array) {
                unsigned char* data = stbi_load_from_memory(reinterpret_cast<stbi_uc const *>(array.bytes.data()), static_cast<int>(array.bytes.size()),
                                                            &width, &height, &nrChannels, 0);
                if (data) {
                    file.images.push_back({});
                    SerializableImage& tex = file.images.back();
                    tex.pixeldata.resize(array.bytes.size());
                    tex.extent = {
                       .width = static_cast<uint32_t>(width),
                       .height = static_cast<uint32_t>(height),
                       .depth = 1
                   };
                   memcpy(tex.pixeldata.data(), data, array.bytes.size() );

                    stbi_image_free(data);
                }
            },
            [&](fastgltf::sources::BufferView& view) {
                auto& bufferView = asset.bufferViews[view.bufferViewIndex];
                auto& buffer = asset.buffers[bufferView.bufferIndex];

                std::visit(fastgltf::visitor {
                    [](auto& arg) {},
                        [&](fastgltf::sources::Vector& vector) {
                            unsigned char* data = stbi_load_from_memory(reinterpret_cast<stbi_uc const *>(vector.bytes.data() + bufferView.byteOffset),
                                                                           static_cast<int>(bufferView.byteLength),
                                                                           &width, &height, &nrChannels, 0);
                            if (data) {
                               file.images.push_back({});
                               SerializableImage& tex = file.images.back();
                               tex.pixeldata.resize(bufferView.byteLength);
                               tex.extent = {
                                   .width = static_cast<uint32_t>(width),
                                   .height = static_cast<uint32_t>(height),
                                   .depth = 1
                                };
                               memcpy(tex.pixeldata.data(), data, bufferView.byteLength );
                               stbi_image_free(data);
                            }
                    },[&](fastgltf::sources::Array& array) {
                            unsigned char* data = stbi_load_from_memory(reinterpret_cast<stbi_uc const *>(array.bytes.data() + bufferView.byteOffset),
                                                                           static_cast<int>(bufferView.byteLength),
                                                                           &width, &height, &nrChannels, 0);
                            if (data) {
                               file.images.push_back({});
                               SerializableImage& tex = file.images.back();
                               tex.pixeldata.resize(bufferView.byteLength);
                               tex.extent = {
                                   .width = static_cast<uint32_t>(width),
                                   .height = static_cast<uint32_t>(height),
                                   .depth = 1
                                };
                               memcpy(tex.pixeldata.data(), data, bufferView.byteLength );
                               stbi_image_free(data);
                            }
                    }
                },buffer.data);},
        },image.data);


}

void LE::ResourceManager::loadSampler(LoadedGLTF& file ,fastgltf::Asset &asset, fastgltf::Sampler &sampler) {

    SamplerKey sampl{};
    sampl.maxLod = 1000.f;
    sampl.minLod = 0;
    sampl.addressModeU = extractSamplerAddressMode(sampler.wrapS);
    sampl.addressModeV = extractSamplerAddressMode(sampler.wrapT);

    sampl.magFilter = extractFilter(sampler.magFilter.value_or(fastgltf::Filter::Nearest));
    sampl.minFilter = extractFilter(sampler.minFilter.value_or(fastgltf::Filter::Nearest));

    sampl.mipmapMode= extractMipmapMode(sampler.minFilter.value_or(fastgltf::Filter::Nearest));

    sampl.GenerateHash();


    file.samplers.push_back(sampl);
}




// void LE::ResourceManager::loadBuffer(LoadedGLTF& file ,fastgltf::Asset &asset, fastgltf::Buffer &buffer) {
//
//
//     std::visit(fastgltf::visitor{
//         [](auto& arg) {
//             LE_CORE_ERROR("Currently doesn't support CustomBuffer, buffers from URI and BufferView");
//             throw std::runtime_error("Currently doesn't support CustomBuffer, buffers from URI and BufferView");
//         },
//         [&](const fastgltf::sources::Array& src) {
//
//                 // memcpy(file.vertexBuffers.data(),src.bytes.data(), src.bytes.size());
//             },
//
//         [&](const fastgltf::sources::Vector& src) {
//                 // memcpy( ,src.bytes.data(), src.bytes.size());
//             },
//         [&](fastgltf::sources::BufferView& view) {
//                 const auto& bufferView = asset.bufferViews[view.bufferViewIndex];
//                 auto& buf = asset.buffers[bufferView.bufferIndex];
//
//                 std::visit(fastgltf::visitor { // We only care about VectorWithMime here, because we
//                                // specify LoadExternalBuffers, meaning all buffers
//                                // are already loaded into a vector.
//                                [](auto& arg) {},
//                                [&](fastgltf::sources::Vector& vector) {
//
//
//                                } },
//                            buffer.data);
//             },
//
//         }, buffer.data);
//
//
// }
