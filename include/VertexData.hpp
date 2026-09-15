//
//  VertexData.h
//  Metal-Tutorial
//

#pragma once
#include <simd/simd.h>

using namespace simd;

struct VertexData {
    float4 position;
    float2 textureCoordinate;
};


enum class BUFFER_INDEX : int {

    VERTEX_DATA ,
    Transformation_DATA ,



    COLTEXTURE_ID = 0 ,

};


struct MVP{

    matrix_float4x4 MVP;
};

struct Uniforms
{   float2 time;
    int intAsBool;
};
