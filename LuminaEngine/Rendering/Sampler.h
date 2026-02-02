//
// Created by Faith Kamaraju on 2026-01-21.
//

#pragma once
#include <cereal/cereal.hpp>

namespace LE {

    enum class Filter : uint32_t
    {
        Nearest = 0 ,
        Linear  ,
        CubicIMG
    };

    enum class SamplerMipmapMode : uint32_t
    {
        Nearest,
        Linear
    };

    enum class SamplerAddressMode : uint32_t
    {
        Repeat               ,
        MirroredRepeat       ,
        ClampToEdge          ,
        ClampToBorder        ,
        MirrorClampToEdge
    };

    enum class CompareOp : uint32_t
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

    inline void HashCombine(size_t& seed, size_t value) {
        seed ^= value + 0x9e3779b97f4a7c15ull + (seed << 6) + (seed >> 2);
    }

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
        uint32_t id{};
        uint32_t generation{};
    };
}