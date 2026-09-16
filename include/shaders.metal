
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
    float4 color;
    float2 textureCoordinate;
};




vertex AAPLOut vertexRenderPass(uint vertexID [[vertex_id]],
                                constant AAPLVertex* vertexData [[buffer(BUFFER_INDEX::AAPL_Vertex_DATA)]]) {
    AAPLOut out;
    out.position = float4(vertexData[vertexID].position, 0.0, 1.0);
    out.color = vertexData[vertexID].color;
    out.textureCoordinate = vertexData[vertexID].textureCoordinate;
    return out;
}


fragment float4 fragmentRenderPass(AAPLOut in [[stage_in]],
                                   texture2d<float> colorTexture [[texture(TEX_INDEX::AAPL_TEX_ID)]]) {
    constexpr sampler textureSampler(mag_filter::linear, min_filter::linear);
            
    float4 result = colorTexture.sample(textureSampler, in.textureCoordinate);


    const float offset = 1.0 / 300.0;  

    float2 offsets[9] = {
        float2(-offset,  offset), // top-left
        float2( 0.0f,    offset), // top-center
        float2( offset,  offset), // top-right
        float2(-offset,  0.0f),   // center-left
        float2( 0.0f,    0.0f),   // center-center
        float2( offset,  0.0f),   // center-right
        float2(-offset, -offset), // bottom-left
        float2( 0.0f,   -offset), // bottom-center
        float2( offset, -offset)  // bottom-right    
    };

    const int kernels[9] = {
        1,2,1,
        2,4,2,
        1,2,1
    };
    float3 sampleTex[9];

    for(int i = 0; i < 9; i++){
        sampleTex[i] = colorTexture.sample(textureSampler, in.textureCoordinate + offsets[i]).rgb;
    }
    float3 col = float3(0.0);
    for(int i = 0; i < 9; i++)
        col += sampleTex[i] * (kernels[i]/16.0f);

    return  float4(col, 1.0); ;
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
