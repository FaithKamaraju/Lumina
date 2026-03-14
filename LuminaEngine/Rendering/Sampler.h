//
// Created by Faith Kamaraju on 2026-01-21.
//

#pragma once
#include <cereal/cereal.hpp>
#include "Core/UtilFunctions.h"

namespace LE {

    enum class Filter : uint8_t
    {
        Nearest = 0 ,
        Linear  ,
        CubicIMG
    };

    enum class SamplerMipmapMode : uint8_t
    {
        Nearest,
        Linear
    };

    enum class SamplerAddressMode : uint8_t
    {
        Repeat               ,
        MirroredRepeat       ,
        ClampToEdge          ,
        ClampToBorder        ,
        MirrorClampToEdge
    };

    enum class CompareOp : uint8_t
    {
        Never          ,
        Less           ,
        Equal          ,
        LessOrEqual    ,
        Greater        ,
        NotEqual       ,
        GreaterOrEqual ,
        Always
      };



    struct SamplerKey {

        Filter magFilter;
        Filter minFilter;
        SamplerMipmapMode mipmapMode;
        SamplerAddressMode addressModeU;
        SamplerAddressMode addressModeV;
        SamplerAddressMode addressModeW;
        float mipLodBias;
        bool anisotropyEnable;
        float maxAnisotropy;
        bool compareEnable;
        CompareOp compareOp;
        float minLod;
        float maxLod;
        // float borderColor[4];
        bool unnormalizedCoordinates;

        size_t hashID{};

        void GenerateHash(){

            HashCombine(hashID, std::hash<Filter>{}(magFilter));
            HashCombine(hashID, std::hash<Filter>{}(minFilter));
            HashCombine(hashID, std::hash<SamplerMipmapMode>{}(mipmapMode));

            HashCombine(hashID, std::hash<SamplerAddressMode>{}(addressModeU));
            HashCombine(hashID, std::hash<SamplerAddressMode>{}(addressModeV));
            HashCombine(hashID, std::hash<SamplerAddressMode>{}(addressModeW));

            HashCombine(hashID, std::hash<float>{}(mipLodBias));
            HashCombine(hashID, std::hash<bool>{}(anisotropyEnable));
            // HashCombine(h, std::hash<float>{}(maxAnisotropy));

            HashCombine(hashID, std::hash<bool>{}(compareEnable));
            HashCombine(hashID, std::hash<CompareOp>{}(compareOp));

            HashCombine(hashID, std::hash<float>{}(minLod));
            HashCombine(hashID, std::hash<float>{}(maxLod));

            // HashCombine(h, std::hash<uint32_t>{}(borderColor));
            HashCombine(hashID, std::hash<bool>{}(unnormalizedCoordinates));

        }

        bool operator==(const SamplerKey& a) const {
            return this->hashID == a.hashID;
        }
        bool operator!=(const SamplerKey& a) const {
            return this->hashID != a.hashID;
        }

    };
    

    struct SamplerHandle {
        int32_t id = -1;
        uint32_t generation{};

        bool operator<(const SamplerHandle& other) const {
            return std::tie(id, generation) < std::tie(other.id, other.generation);
        }
    };
}