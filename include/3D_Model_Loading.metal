#include <metal_stdlib>
using namespace metal;

#include "VertexData.hpp"
#include "Material.hpp"

struct VertexOut{
    float4 position [[position]];
    float4 normal;
    float4 indices;
    float4 tangent;
    float4 bitangent;
    float2 uv;
};

vertex VertexOut modelVertexShader(uint vertexID [[vertex_id]],
             constant Mesh_Vertices* vertexData[[buffer(MESH_BUFFER_INDEX::MESH_VERTEX_DATA)]],
             constant MVP * mvp [[buffer(MESH_BUFFER_INDEX::MVP_DATA)]]) {
    VertexOut out;
    out.position = mvp->MVP  * float4(vertexData[vertexID].position.x, vertexData[vertexID].position.y, vertexData[vertexID].position.z , 1.0f) ;
    out.uv = float2(vertexData[vertexID].uv.U, vertexData[vertexID].uv.V);
    return out;
}

fragment float4 modelFragmentShader(VertexOut in [[stage_in]],
                                   texture2d<float> baseColorTexture [[texture(0)]]) {
                                    
    constexpr sampler textureSampler (mag_filter::linear,
                                      min_filter::linear);
    const float4 colorSample = baseColorTexture.sample(textureSampler, in.uv);
    return colorSample;
}