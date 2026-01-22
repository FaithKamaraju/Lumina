//
// Created by Faith Kamaraju on 2026-01-21.
//

#pragma once

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

        size_t GenerateHash() const {
            size_t h = 0;

            HashCombine(h, std::hash<Filter>{}(magFilter));
            HashCombine(h, std::hash<Filter>{}(minFilter));
            HashCombine(h, std::hash<SamplerMipmapMode>{}(mipmapMode));

            HashCombine(h, std::hash<SamplerAddressMode>{}(addressModeU));
            HashCombine(h, std::hash<SamplerAddressMode>{}(addressModeV));
            HashCombine(h, std::hash<SamplerAddressMode>{}(addressModeW));

            HashCombine(h, std::hash<float>{}(mipLodBias));
            HashCombine(h, std::hash<bool>{}(anisotropyEnable));
            // HashCombine(h, std::hash<float>{}(maxAnisotropy));

            HashCombine(h, std::hash<bool>{}(compareEnable));
            HashCombine(h, std::hash<CompareOp>{}(compareOp));

            HashCombine(h, std::hash<float>{}(minLod));
            HashCombine(h, std::hash<float>{}(maxLod));

            // HashCombine(h, std::hash<uint32_t>{}(borderColor));
            HashCombine(h, std::hash<bool>{}(unnormalizedCoordinates));

            return h;
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