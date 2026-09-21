#include <metal_stdlib>
using namespace metal;

#include "VertexData.hpp"
#include "Material.hpp"

struct VertexOut{
    float4 position [[position]];
    float2 uv;
};

vertex VertexOut modelVertexShader(uint vertexID [[vertex_id]],
             constant VertexData* vertexData[[buffer(MESH_BUFFER_INDEX::MESH_VERTEX_DATA)]],
             constant MESHMVP * mvp [[buffer(MESH_BUFFER_INDEX::MVP_DATA)]]) {
    VertexOut out;
    out.position = mvp->MVP  * vertexData[vertexID].position ;
    out.uv = vertexData[vertexID].textureCoordinate;
    return out;
}

fragment float4 modelFragmentShader(VertexOut in [[stage_in]],
                                   texture2d<float> base_color_texture [[texture(MESH_TEXTURE_INDEX::base_color_texture)]],
                                   texture2d<float> normalmap_texture [[texture(MESH_TEXTURE_INDEX::normalmap_texture)]],
                                   texture2d<float> metallic_texture [[texture(MESH_TEXTURE_INDEX::metallic_texture)]],
                                   texture2d<float> roughness_texture [[texture(MESH_TEXTURE_INDEX::roughness_texture)]],
                                   texture2d<float> specular_texture [[texture(MESH_TEXTURE_INDEX::specular_texture)]]
                                   ) {

    constexpr sampler textureSampler (mag_filter::linear,
                                      min_filter::linear);
    const float4 colorSample = base_color_texture.sample(textureSampler, in.uv);

                
    float w = base_color_texture.get_width() / 4096.0;
    return float4(w, w, w, 1);
}