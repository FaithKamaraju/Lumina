//
// Created by Faith Kamaraju on 2026-01-15.
//

#pragma once

#include "shaderc/shaderc.hpp"
#include "LoadedGLTF.h"
#include "Core/LE_Types.h"
#include "fastgltf/core.hpp"
#include "Rendering/Shader.h"



namespace LE {

    class RHI;
    class AssetRegistry;

    class ResourceManager {
    public:
        explicit ResourceManager(RHI* rhi, AssetRegistry* registry);

        LEBool Initialize();


        LEBool ImportGLTFFile(const std::filesystem::path& filepath, std::string outAssetName = "");
        void LoadSceneAsset(const std::filesystem::path& filepath, const SceneNode& parentNode);

        ShaderHandle CreateShaderObject(const std::string& fileName, ShaderStage stage);

    private:

        void generateDefaultTextures();
        void prepareDefaultPBRShaders();

        std::optional<LoadedGLTF> processGLTFFile(const std::filesystem::path& filepath);

        void initializeShader_cCompiler(shaderc_optimization_level optimizationLevel = shaderc_optimization_level_zero,
            shaderc_source_language sourceLanguage = shaderc_source_language_glsl);
        static LEBool readShaderSource(const std::string& filePath, std::vector<char>& buffer);
        static LEBool readSPVFile(const std::string& filename, std::vector<char>& buffer);
        LEBool compileShader(const char* code, std::vector<uint32_t>& compiledSPV, shaderc_shader_kind shaderKind, const char* source_name,
                                            const shaderc::CompileOptions& compileOptions);
        static shaderc_shader_kind getShadercStageEnum(ShaderStage stage);

        // static void loadBuffer(LoadedGLTF& file, fastgltf::Asset& asset, fastgltf::Buffer& buffer);
        static void loadImage(LoadedGLTF& file, fastgltf::Asset& asset, fastgltf::Image& image);
        static void loadSampler(LoadedGLTF& file, fastgltf::Asset& asset, fastgltf::Sampler& sampler);
        static Filter extractFilter(fastgltf::Filter filter);
        static SamplerMipmapMode extractMipmapMode(fastgltf::Filter filter);
        static SamplerAddressMode extractSamplerAddressMode(fastgltf::Wrap wrap);
        static ImageFormat findSuitableSRGBorUNORMFormat(uint32_t nrChannels, bool bIsSRGB);


        shaderc::CompileOptions g_CompileOptions{};
        shaderc::Compiler g_ShaderCompiler{};



        fastgltf::Parser parser {};

        RHI* mRHI = nullptr;
        AssetRegistry* mRegistry = nullptr;



    };
}
