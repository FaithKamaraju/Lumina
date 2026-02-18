#version 460
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference : require

const uint MAX_NUM_TEX_COORDS = 4;

struct Vertex {
    vec3 pos;
    float padding1;
    vec3 normal;
    float padding2;
    vec2 texCoord[MAX_NUM_TEX_COORDS];
    vec4 tangent;
    vec4 color;
};

struct PBRMaterialGPUData {
    vec4 baseColorFactor;
    float metallicFactor;
    float roughnessFactor;
    uint baseColorImageIndex;
    uint baseColorImageSamplerIndex;
    uint baseColorTextureTexCoord;
    uint metallicRoughnessImageIndex;
    uint metallicRoughnessImageSamplerIndex;
    uint metallicRoughnessTextureTexCoord;
    uint normalMapImageIndex;
    uint normalMapImageSamplerIndex;
    uint normalMapTextureTexCoord;
};

layout (location = 0) in vec3 outFragColor;
layout (location = 1) in vec2 outTexCoord;

layout(set = 1, binding = 0) uniform texture2D Textures[];
layout(set = 2, binding = 0) uniform sampler Samplers[];
layout(std140, set = 3, binding = 0) readonly buffer MaterialData {
    PBRMaterialGPUData mats[];
} matData;

layout(buffer_reference, std430) readonly buffer VertexBuffer{
    Vertex vertices[];
};

layout(push_constant) uniform PerRenderableConstants {
    mat4 modelMatrix;
    VertexBuffer vertexBuffer;
    uint materialIndex;
} per_object_data;

layout (location = 0) out vec4 outColor;

void main() {

    uint baseColor = matData.mats[per_object_data.materialIndex].baseColorImageIndex;
    uint baseColorSamp = matData.mats[per_object_data.materialIndex].baseColorImageSamplerIndex;
    uint baseColorTexCoord = matData.mats[per_object_data.materialIndex].baseColorTextureTexCoord;
    outColor = texture(sampler2D(Textures[nonuniformEXT(baseColor)], Samplers[nonuniformEXT(baseColorSamp)]), outTexCoord);

//    outColor = vec4(0.5, 0.2, 0.8, 1.0);
}