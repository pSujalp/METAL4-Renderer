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

fragment float4 modelFragmentShader(VertexOut in [[stage_in]]) {
    return float4(1.0f,0.0f,0.0f,1.0f);
}