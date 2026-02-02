//
// Created by Faith Kamaraju on 2026-01-15.
//

#pragma once

#include "shaderc/shaderc.hpp"
#include "LoadedGLTF.h"
#include "Core/LE_Types.h"
#include "fastgltf/core.hpp"
#include "Rendering/RHI.h"


namespace LE {

    class ResourceManager {

    public:
        explicit ResourceManager(RHI* rhi);

        static std::vector<char> ReadShaderSource(const char *filePath);
        static std::vector<char> ReadSPVFile(const std::string& filename);
        std::vector<uint32_t> CompileShader(const char* code, shaderc_shader_kind shaderKind, const char* source_name,
                                            const shaderc::CompileOptions* compileOptions);

        LEBool ImportGLTFFile(const std::filesystem::path& filepath, std::string outAssetName = "");
        static void LoadSceneAsset(const std::filesystem::path& filepath);


    private:

        std::optional<Ref<LoadedGLTF>> loadGLTFFileIntoCPUMemory(const std::filesystem::path& filepath);

        void InitializeShader_cCompiler(shaderc_optimization_level optimizationLevel = shaderc_optimization_level_zero,
            shaderc_source_language sourceLanguage = shaderc_source_language_glsl);

        // static void loadBuffer(LoadedGLTF& file ,fastgltf::Asset& asset, fastgltf::Buffer& buffer);
        static void loadImage(LoadedGLTF& file, fastgltf::Asset& asset, fastgltf::Image& image);
        static void loadSampler(LoadedGLTF& file, fastgltf::Asset& asset, fastgltf::Sampler& sampler);
        static Filter extractFilter(fastgltf::Filter filter);
        static SamplerMipmapMode extractMipmapMode(fastgltf::Filter filter);
        static SamplerAddressMode extractSamplerAddressMode(fastgltf::Wrap wrap);
        static ImageFormat findSuitableSRGBorUNORMFormat(int nrChannels, bool bIsSRGB);


        shaderc::CompileOptions g_CompileOptions{};
        shaderc::Compiler g_ShaderCompiler{};

        fastgltf::Parser parser {};

        std::hash<std::pmr::string> pmrStringHasher{};

        RHI* mRHI = nullptr;



    };
}
