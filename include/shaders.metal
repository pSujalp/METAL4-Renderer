
#include <metal_stdlib>
using namespace metal;

#include "VertexData.hpp"
#include "SkyboxData.hpp"

struct VertexOut {

    float4 position [[position]];
    float2 textureCoordinate;
};

struct SkyboxVOut {
    float4 position [[position]];
    float3 direction;  
};

vertex VertexOut vertexShader(uint vertexID [[vertex_id]],
             constant VertexData* vertexData[[buffer(BUFFER_INDEX::VERTEX_DATA)]],
             constant MVP * mvp [[buffer(BUFFER_INDEX::Transformation_DATA)]]) {
    VertexOut out;
    out.position = mvp->MVP  * vertexData[vertexID].position;
    out.textureCoordinate = vertexData[vertexID].textureCoordinate;
    return out;
}

fragment float4 fragmentShader(VertexOut in [[stage_in]],
                               texture2d<float> colorTexture [[texture(TEX_INDEX::COLTEXTURE_ID)]]) {
    constexpr sampler textureSampler (mag_filter::linear,
                                      min_filter::linear);
    const float4 colorSample = colorTexture.sample(textureSampler, in.textureCoordinate);
    return colorSample;
}

vertex SkyboxVOut skyboxVertex(
    uint  vid  [[vertex_id]],
    constant SkyboxVertexData* verts    [[buffer(BUFFER_INDEX::SKYBOX_BUFFER_INDEX)]],
    constant MVP&          mvp      [[buffer(BUFFER_INDEX::MVP_BUFFER_INDEX)]])
{
    SkyboxVOut out;
    out.direction = verts[vid].position;
    float4 clip = mvp.MVP * float4(verts[vid].position, 1.0);
    out.position = clip.xyww;  
    return out;
}

fragment float4 skyboxFragment(
    SkyboxVOut             in       [[stage_in]],
    texturecube<half>      skyTex   [[texture(TEX_INDEX::SKYTEX_TEXTURE_INDEX)]])
{
    constexpr sampler cubeSampler(mip_filter::linear,
                                   mag_filter::linear,
                                   min_filter::linear,
                                   s_address::clamp_to_edge,
                                   t_address::clamp_to_edge,
                                   r_address::clamp_to_edge);

    float3 texCoords = float3(in.direction.x, in.direction.y, -in.direction.z);

    return float4(skyTex.sample(cubeSampler, texCoords));
}