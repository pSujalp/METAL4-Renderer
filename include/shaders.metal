
#include <metal_stdlib>
using namespace metal;

#include "VertexData.hpp"

struct VertexOut {

    float4 position [[position]];
    float2 textureCoordinate;
};

struct SkyboxVOut {
    float4 position [[position]];
    float3 direction;  
};


struct AAPLOut {
    float4 position [[position]];
    float2 textureCoordinate;
};




vertex AAPLOut vertexRenderPass(uint vertexID [[vertex_id]],
                                constant AAPLVertex* vertexData [[buffer(BUFFER_INDEX::AAPL_Vertex_DATA)]]) {
    AAPLOut out;
    out.position = float4(vertexData[vertexID].position, 0.0, 1.0);
    out.textureCoordinate = vertexData[vertexID].textureCoordinate;
    return out;
}


fragment float4 fragmentRenderPass(AAPLOut in [[stage_in]],
                                      texture2d<float> sceneColor [[texture(TEX_INDEX::AAPL_TEX_ID)]])
{
    constexpr sampler pointSampler(mag_filter::nearest, min_filter::nearest);

    float2 texel = 1.0 / float2(sceneColor.get_width(), sceneColor.get_height());
    float2 p = in.textureCoordinate;

    float3 cL = sceneColor.sample(pointSampler, p + float2(-texel.x, 0)).rgb;
    float3 cR = sceneColor.sample(pointSampler, p + float2( texel.x, 0)).rgb;
    float3 cU = sceneColor.sample(pointSampler, p + float2(0, -texel.y)).rgb;
    float3 cD = sceneColor.sample(pointSampler, p + float2(0,  texel.y)).rgb;

    float lumaDiff = length(cL - cR) + length(cU - cD);
    float edge = smoothstep(0.08, 0.09, lumaDiff);

    float3 base = sceneColor.sample(pointSampler, p).rgb;
    return float4(mix(base, float3(0.0), edge), 1.0);
}

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
