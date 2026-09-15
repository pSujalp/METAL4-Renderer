
#pragma once
#include <simd/simd.h>
using namespace simd;

#include "VertexData.hpp" 

struct SkyboxVertexData {
    float3 position;
};


enum class SKYBUFFER_INDEX{

    SKYBOX_BUFFER_INDEX = 0,
    MVP_BUFFER_INDEX = 1
};

enum class SKYTEXTURE_INDEX{
    SKYTEX_TEXTURE_INDEX = 0
};

enum class SKYSAMPLER_INDEX{
     SAMPLER_INDEX = 0
};