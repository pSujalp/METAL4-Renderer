
#include <metal_stdlib>
using namespace metal;

#include "SkyboxData.hpp"


struct SkyboxVOut {
    float4 position [[position]];
    float3 direction;  
};

vertex SkyboxVOut skyboxVertex(
    uint  vid  [[vertex_id]],
    constant SkyboxVertexData* verts    [[buffer(SKYBUFFER_INDEX::SKYBOX_BUFFER_INDEX)]],
    constant MVP&          mvp      [[buffer(SKYBUFFER_INDEX::MVP_BUFFER_INDEX)]])
{
    SkyboxVOut out;
    out.direction = verts[vid].position;
    float4 clip = mvp.MVP * float4(verts[vid].position, 1.0);
    out.position = clip.xyww;  
    return out;
}

fragment float4 skyboxFragment(
    SkyboxVOut             in       [[stage_in]],
    texturecube<half>      skyTex   [[texture(SKYTEX_INDEX::SKYTEX_TEXTURE_INDEX)]])
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