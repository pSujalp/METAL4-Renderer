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

    VERTEX_DATA = 1,
    COLTEXTURE_ID = 0,
    Transformation_DATA = 3

};


struct MVP{

    matrix_float4x4 MVP;
};