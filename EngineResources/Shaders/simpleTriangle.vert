#version 460
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

layout(buffer_reference, std430) readonly buffer VertexBuffer{
    Vertex vertices[];
};

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    float time;
} ubo;

layout(std140, set = 3, binding = 0) readonly buffer MaterialData {
    PBRMaterialGPUData mats[];
} matData;

layout(push_constant) uniform PerRenderableConstants
{
    mat4 modelMatrix;
    VertexBuffer vertexBuffer;
    uint materialIndex;

} per_object_data;

layout (location = 0) out vec3 outFragColor;
layout (location = 1) out vec2 outTexCoord;

void main() {
    Vertex v = per_object_data.vertexBuffer.vertices[gl_VertexIndex];

    gl_Position = ubo.proj * ubo.view * per_object_data.modelMatrix * vec4(v.pos, 1.0);
    outFragColor = v.color.rgb;
    uint baseColorTexCoord = matData.mats[per_object_data.materialIndex].baseColorTextureTexCoord;
    outTexCoord = v.texCoord[baseColorTexCoord];

}