//
// Created by Faith Kamaraju on 2026-01-15.
//

#include "ResourceManager.h"
#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/optional.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <xxhash.h>
#include <stb_image.h>
#include <fastgltf/tools.hpp>
#include "Rendering/RHI.h"
#include "AssetRegistry.h"
#include "Core/MathUtils.h"
#include "Core/Logger.h"
#include "Core/DirectoryPathnames.h"
#include "Core/CerealUtils.h"
#include "TextureAsset.h"
#include "MaterialAsset.h"
#include "Core/EngineStatics.h"
#include "Rendering/RenderingConstants.h"


constexpr XXH64_hash_t HASH_SEED = 12345;



LE::ResourceManager::ResourceManager(RHI *rhi, AssetRegistry *registry) : mRHI(rhi), mRegistry(registry) {

    Initialize();

}

LEBool LE::ResourceManager::Initialize() {

    initializeShader_cCompiler();
    generateDefaultTextures();
    prepareDefaultPBRShaders();

    return LE_SUCCESS;
}


LEBool LE::ResourceManager::ImportGLTFFile(const std::filesystem::path& filepath, std::string outAssetName) {

    auto gltfScene = processGLTFFile(filepath);

    if (!gltfScene.has_value()) {
        LE_CORE_INFO("Couldn't load the gltf file from filepath: {0}", filepath.string());
        return LE_FAILURE;
    }

    if (outAssetName.empty()) {
        outAssetName = filepath.stem().string() + ".LEASSET";
    }
    if (outAssetName.find(".LEASSET") == std::string::npos) {
        outAssetName += ".LEASSET";
    }
    std::ofstream file{"../../../Content/Assets/" + outAssetName, std::ios::binary};
    {
        cereal::PortableBinaryOutputArchive oarchive(file);
        oarchive(gltfScene.value());
    }

    return LE_SUCCESS;

}

void LE::ResourceManager::LoadSceneAsset(const std::filesystem::path &filepath, const SceneNode& parentNode) {

    std::ifstream file{filepath, std::ios::binary};
    LoadedGLTF gltf;
    {
        cereal::PortableBinaryInputArchive iarchive(file);
        iarchive(gltf);
    }

    std::vector<SamplerHandle> sampler_handles;
    sampler_handles.reserve(gltf.samplers.size());
    for (auto& sampler : gltf.samplers) {
        sampler_handles.push_back(mRHI->CreateImageSampler(sampler));
    }

    std::vector<ImageHandle> image_handles;
    image_handles.reserve(gltf.images.size());
    for (auto& image : gltf.images) {
        auto format = findSuitableSRGBorUNORMFormat(image.nrChannels, image.bIsSRGB);
        image_handles.push_back(mRHI->CreateImage(image.hashID, image.pixeldata.data(), image.nrChannels, image.extent,
            format, ImageUsageFlags::ShaderRead, false));
    }

    std::vector<TextureAssetHandle> texture_asset_handles;
    texture_asset_handles.reserve(gltf.textures.size());
    for (auto& texture : gltf.textures) {
        TextureAsset texAsset{image_handles[texture.imageIndex], sampler_handles[texture.sampIndex]};
        texture_asset_handles.push_back( mRegistry->RegisterTextureAsset(texAsset));
    }

    std::vector<PBR_MR_MaterialInstanceHandle> material_asset_handles;
    material_asset_handles.reserve(gltf.materials.size());
    for (auto& material : gltf.materials) {
        // TODO  - ADD AND ELSE STATEMENT TO LOAD ERROR OR DEFAULT TEXTURES IF BASE COLOR TEXTURE IS NOT PRESENT!
        PBR_MR_MaterialInstance mat{};
        if (material.baseColorTextureIndex != -1) {
            mat.baseColorTextureAsset = texture_asset_handles[material.baseColorTextureIndex];
            mat.baseColorTextureTexCoord = material.baseColorTextureTexCoord;
        }
        if (material.metallicRoughnessTextureIndex != -1) {
            mat.metallicRoughnessTextureAsset = texture_asset_handles[material.metallicRoughnessTextureIndex];
            mat.metallicRoughnessTextureTexCoord = material.metallicRoughnessTextureTexCoord;
        }
        if (material.normalMapTextureIndex != -1) {
            mat.normalMapTextureAsset = texture_asset_handles[material.normalMapTextureIndex];
            mat.normalMapTextureTexCoord = material.normalMapTextureTexCoord;
        }
        mat.baseColorFactor = material.baseColorFactor;
        mat.metallicFactor = material.metallicFactor;
        mat.roughnessFactor = material.roughnessFactor;
        mat.cullMode = material.cullMode;

        material_asset_handles.push_back(mRegistry->RegisterPBR_MR_MaterialInstance(mat));
    }

    auto indexBuffer = mRHI->AllocateAndCopyIndexBuffer(gltf.indices);
    auto vertexBuffer = mRHI->AllocateAndCopyVertexBuffer(gltf.vertices);

    std::vector<MeshAssetHandle> mesh_asset_handles;
    mesh_asset_handles.reserve(gltf.meshes.size());
    for (auto& mesh : gltf.meshes) {
        MeshAsset mesh_asset{};
        mesh_asset.indicesBufferHandle = indexBuffer;
        mesh_asset.vertexBufferHandle = vertexBuffer;
        mesh_asset.subMeshes.reserve(mesh.subMeshes.size());
        for (auto& submesh : mesh.subMeshes) {
            SubMesh sub_mesh_asset{};
            sub_mesh_asset.topology = submesh.topology;
            sub_mesh_asset.indexCount = submesh.indexCount;
            sub_mesh_asset.indexOffset = submesh.indexOffset;
            sub_mesh_asset.vertexOffset = submesh.vertexOffset;
            sub_mesh_asset.materialHandle = material_asset_handles[submesh.materialIndex];
            mesh_asset.subMeshes.push_back(sub_mesh_asset);
        }
        mesh_asset_handles.push_back(mRegistry->RegisterMeshAsset(mesh_asset));
    }
    std::vector<SceneNode> scene_nodes;
    scene_nodes.reserve(gltf.nodes.size());
    for (auto& node : gltf.nodes) {
        SceneNode scene_node{};
        scene_node.localTransform = node.localTransform;
        scene_node.globalTransform = node.globalTransform;
        scene_node.debug_name = node.debug_name;
        scene_node.hierarchy = node.hierarchy;
        scene_node.meshHandle = mesh_asset_handles[node.meshIndex];
        scene_nodes.push_back(scene_node);
    }

    Globals::GetCurrentSceneGraph().nodes.insert(Globals::GetCurrentSceneGraph().nodes.end(),
        scene_nodes.begin(), scene_nodes.end());

}


LE::ShaderHandle LE::ResourceManager::CreateShaderObject(const std::string &fileName, ShaderStage stage) {

    std::vector<char> shaderSource;
    auto result = readShaderSource(fileName, shaderSource);
    assert(result && "Error in reading shader file!");

    std::vector<uint32_t> spvBinary;
    result = compileShader(shaderSource.data(),spvBinary, getShadercStageEnum(stage),
        fileName.c_str(), g_CompileOptions );
    assert(result && "Error in compiling shader!");

    uint64_t hash =  XXH3_64bits_withSeed(spvBinary.data(), spvBinary.size(), HASH_SEED);

    return mRHI->CreateShaderModule(hash, spvBinary);
}

void LE::ResourceManager::generateDefaultTextures() {

}

void LE::ResourceManager::prepareDefaultPBRShaders() {

    auto vert = CreateShaderObject(SHADERS_DIR_PATH + PBR_METALLIC_ROUGHNESS_VERTEX_SHADER,
        ShaderStage::Vertex);
    auto frag = CreateShaderObject(SHADERS_DIR_PATH + PBR_METALLIC_ROUGHNESS_FRAGMENT_SHADER,
        ShaderStage::Fragment);

    mRegistry->SetDefaultPBRShaderHandles(vert, frag);

}


std::optional<LE::LoadedGLTF> LE::ResourceManager::processGLTFFile(const std::filesystem::path& filepath) {

    LoadedGLTF file{};

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

        GLTFMetallicRoughnessMaterialData material_asset{};
        // tex.texCoord = mat.pbrData.baseColorTexture->texCoordIndex;

        if (mat.pbrData.baseColorTexture.has_value()) {
            material_asset.baseColorTextureIndex = static_cast<int32_t>(mat.pbrData.baseColorTexture->textureIndex);
            material_asset.baseColorTextureTexCoord = static_cast<int32_t>(mat.pbrData.baseColorTexture->texCoordIndex);
            file.images[file.textures[material_asset.baseColorTextureIndex].imageIndex].bIsSRGB = true;
        }
        if (mat.pbrData.metallicRoughnessTexture.has_value()) {
            material_asset.metallicRoughnessTextureIndex = static_cast<int32_t>(mat.pbrData.metallicRoughnessTexture->textureIndex);
            material_asset.metallicRoughnessTextureTexCoord = static_cast<int32_t>(mat.pbrData.metallicRoughnessTexture->texCoordIndex);
        }
        if (mat.normalTexture.has_value()) {
            material_asset.normalMapTextureIndex = static_cast<int32_t>(mat.normalTexture->textureIndex);
            material_asset.normalMapTextureTexCoord = static_cast<int32_t>(mat.normalTexture->textureIndex);
        }
        material_asset.baseColorFactor.r  = mat.pbrData.baseColorFactor[0];
        material_asset.baseColorFactor.g  = mat.pbrData.baseColorFactor[1];
        material_asset.baseColorFactor.b  = mat.pbrData.baseColorFactor[2];
        material_asset.baseColorFactor.a  = mat.pbrData.baseColorFactor[3];
        material_asset.metallicFactor = mat.pbrData.metallicFactor;
        material_asset.roughnessFactor = mat.pbrData.roughnessFactor;
        material_asset.cullMode = mat.doubleSided ? CullMode::None : CullMode::Back;

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
        MeshAssetData newMesh{};

        for (auto&& p : mesh.primitives) {
            SubMeshData subMesh{};
            subMesh.topology = getPrimitiveType(p.type);
            subMesh.vertexOffset = initial_vtx;
            subMesh.indexOffset = file.indices.size();
            subMesh.indexCount = gltf.accessors[p.indicesAccessor.value()].count;
            newMesh.indexCount += subMesh.indexCount;
            if (p.materialIndex.has_value()) {
                subMesh.materialIndex = (int32_t)p.materialIndex.value();
            }
            // load indexes
            {
                fastgltf::Accessor& indexAcc = gltf.accessors[p.indicesAccessor.value()];
                fastgltf::iterateAccessor<std::uint32_t>(gltf, indexAcc,
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
        SceneNodeData newSceneNode{};
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

    return file;
}

void LE::ResourceManager::initializeShader_cCompiler(shaderc_optimization_level optimizationLevel,
                                              shaderc_source_language sourceLanguage) {

    g_CompileOptions.SetOptimizationLevel(optimizationLevel);
    g_CompileOptions.SetSourceLanguage(sourceLanguage);

}

LEBool LE::ResourceManager::readShaderSource(const std::string& filePath,std::vector<char>& buffer ) {

    std::ifstream file(filePath, std::ios::ate);

    if (!file.is_open()) {
        LE_CORE_ERROR("Failed to read shader file at {0}", filePath);
        return LE_FAILURE;
    }
    size_t fileSize = file.tellg();
    buffer.resize(fileSize);

    file.seekg(0);
    file.read(buffer.data(), static_cast<std::streamsize>(fileSize));
    file.close();

    return LE_SUCCESS;
}

LEBool LE::ResourceManager::readSPVFile(const std::string &filePath, std::vector<char>& buffer) {

    std::ifstream file(filePath, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        LE_CORE_ERROR("Failed to read shader file at {0}", filePath);
        return LE_FAILURE;
    }

    size_t fileSize = file.tellg();
    buffer.resize(fileSize);

    file.seekg(0);
    file.read(buffer.data(), static_cast<std::streamsize>(fileSize));
    file.close();

    return LE_SUCCESS;
}

LEBool LE::ResourceManager::compileShader(const char *code, std::vector<uint32_t>& compiledSPV, shaderc_shader_kind shaderKind,
    const char *source_name, const shaderc::CompileOptions& compileOptions) {

    auto spvBinary= g_ShaderCompiler.CompileGlslToSpv(code, shaderKind,
                                                       source_name, g_CompileOptions);

    if (spvBinary.GetCompilationStatus() != shaderc_compilation_status_success) {
        LE_CORE_ERROR("SHADER COMPILATION ERROR: {0}",spvBinary.GetErrorMessage());
        return LE_FAILURE;
    }

    compiledSPV.assign(spvBinary.cbegin(), spvBinary.cend());

    return LE_SUCCESS;
}

shaderc_shader_kind LE::ResourceManager::getShadercStageEnum(ShaderStage stage) {
    switch (stage) {
        case ShaderStage::Vertex:
            return shaderc_vertex_shader;
        case ShaderStage::TessellationControl:
            return shaderc_tess_control_shader;
        case ShaderStage::TessellationEvaluation:
            return shaderc_tess_evaluation_shader;
        case ShaderStage::Geometry:
            return shaderc_geometry_shader;
        case ShaderStage::Fragment:
            return shaderc_fragment_shader;
        case ShaderStage::Compute:
            return shaderc_compute_shader;
        case ShaderStage::Mesh:
            return shaderc_mesh_shader;
    }
}


void LE::ResourceManager::loadImage(LoadedGLTF& file ,fastgltf::Asset &asset, fastgltf::Image &image) {

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
                unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 4);
                if (data) {
                    file.images.push_back({});
                    SerializableImage& tex = file.images.back();
                    tex.hashID = XXH3_64bits_withSeed(data, width * height * 4, HASH_SEED);
                    tex.pixeldata.resize(width * height * 4);
                    tex.nrChannels = 4;
                    tex.extent = {
                        .width = static_cast<uint32_t>(width),
                        .height = static_cast<uint32_t>(height),
                        .depth = 1
                    };
                    memcpy(tex.pixeldata.data(), data, width * height * 4 );
                    stbi_image_free(data);
                }
            },
            [&](fastgltf::sources::Vector& vector) {
                unsigned char* data = stbi_load_from_memory(reinterpret_cast<stbi_uc const *>(vector.bytes.data()), static_cast<int>(vector.bytes.size()),
                                                            &width, &height, &nrChannels, 4);

                if (data) {
                    file.images.push_back({});
                    SerializableImage& tex = file.images.back();
                    tex.hashID = XXH3_64bits_withSeed(data, width * height * 4, HASH_SEED);
                    tex.pixeldata.resize(width * height * 4);
                    tex.nrChannels = 4;
                    tex.extent = {
                       .width = static_cast<uint32_t>(width),
                       .height = static_cast<uint32_t>(height),
                       .depth = 1
                   };
                   memcpy(tex.pixeldata.data(), data, width * height * 4 );

                    stbi_image_free(data);
                }
            },
            [&](fastgltf::sources::Array& array) {
                unsigned char* data = stbi_load_from_memory(reinterpret_cast<stbi_uc const *>(array.bytes.data()), static_cast<int>(array.bytes.size()),
                                                            &width, &height, &nrChannels, 4);
                if (data) {
                    file.images.push_back({});
                    SerializableImage& tex = file.images.back();
                    tex.hashID = XXH3_64bits_withSeed(data, width * height * 4, HASH_SEED);
                    tex.pixeldata.resize(width * height * 4);
                    tex.nrChannels = 4;
                    tex.extent = {
                       .width = static_cast<uint32_t>(width),
                       .height = static_cast<uint32_t>(height),
                       .depth = 1
                   };
                   memcpy(tex.pixeldata.data(), data, width * height * 4 );

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
                                                                           &width, &height, &nrChannels, 4);
                            if (data) {
                                file.images.push_back({});
                                SerializableImage& tex = file.images.back();
                                tex.hashID = XXH3_64bits_withSeed(data, width * height * 4, HASH_SEED);
                                tex.pixeldata.resize(width * height * 4);
                                tex.nrChannels = 4;
                                tex.extent = {
                                   .width = static_cast<uint32_t>(width),
                                   .height = static_cast<uint32_t>(height),
                                   .depth = 1
                                };
                               memcpy(tex.pixeldata.data(), data, width * height * 4 );
                               stbi_image_free(data);
                            }
                    },[&](fastgltf::sources::Array& array) {
                            unsigned char* data = stbi_load_from_memory(reinterpret_cast<stbi_uc const *>(array.bytes.data() + bufferView.byteOffset),
                                                                           static_cast<int>(bufferView.byteLength),
                                                                           &width, &height, &nrChannels, 4);
                            if (data) {
                                file.images.push_back({});
                                SerializableImage& tex = file.images.back();
                                tex.hashID = XXH3_64bits_withSeed(data, width * height * 4, HASH_SEED);
                                tex.pixeldata.resize(width * height * 4);
                                tex.nrChannels = 4;
                                tex.extent = {
                                   .width = static_cast<uint32_t>(width),
                                   .height = static_cast<uint32_t>(height),
                                   .depth = 1
                                };
                               memcpy(tex.pixeldata.data(), data, width * height * 4 );
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

LE::ImageFormat LE::ResourceManager::findSuitableSRGBorUNORMFormat(uint32_t nrChannels, bool bIsSRGB) {


    if (bIsSRGB) {
        switch (nrChannels) {
            case 1: return ImageFormat::R8_SRGB;
            case 3: return ImageFormat::R8G8B8_SRGB;
            case 4: return ImageFormat::R8G8B8A8_SRGB;
            default: return ImageFormat::Undefined;
        }
    }
    switch (nrChannels) {
        case 1: return ImageFormat::R8_UNorm;
        case 3: return ImageFormat::R8G8B8_UNorm;
        case 4: return ImageFormat::R8G8B8A8_UNorm;
        default: return ImageFormat::Undefined;
    }
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
