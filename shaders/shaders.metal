
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
                                   texture2d<float> colorTexture [[texture(TEX_INDEX::AAPL_TEX_ID)]]) {
    constexpr sampler textureSampler(mag_filter::linear, min_filter::linear);
            
    float4 result = colorTexture.sample(textureSampler, in.textureCoordinate);

    float gamma = 2.2f;
    result.rgb = pow(result.rgb, float3(1.0f/gamma));   
    return  result ;
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
