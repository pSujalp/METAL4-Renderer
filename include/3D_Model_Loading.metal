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
             constant MESHMVP * mvp [[buffer(MESH_BUFFER_INDEX::MVP_DATA)]]) {
    VertexOut out;
    out.position = mvp->MVP  * float4(vertexData[vertexID].position , 1.0f) ;
    out.uv = vertexData[vertexID].uv;
    return out;
}

fragment float4 modelFragmentShader(VertexOut in [[stage_in]]) {
                                    
    // constexpr sampler textureSampler (mag_filter::linear,
    //                                   min_filter::linear);
    // const float4 colorSample = pbr_mat->base_color_texture.sample(textureSampler, in.uv);
    return float4(1.0f,0.0f,0.0f,1.0f);
}