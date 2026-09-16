
#pragma once
#include <simd/simd.h>
using namespace simd;

#include "VertexData.hpp" 

struct SkyboxVertexData {
    float3 position;
};

enum class SKYBUFFER_INDEX : uint8_t
{
    SKYBOX_BUFFER_INDEX ,
    MVP_BUFFER_INDEX 
};


enum class SKYTEX_INDEX : uint8_t
{
    SKYTEX_TEXTURE_INDEX 
};