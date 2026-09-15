
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
