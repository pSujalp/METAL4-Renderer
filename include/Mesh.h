#pragma once 



#include <Metal/Metal.hpp>
#include <AppKit/AppKit.hpp>
#include <MetalKit/MetalKit.hpp>
#include <QuartzCore/CAMetalLayer.h>

#include <vector>
#include "Material.hpp"
#include "VertexData.hpp"
#include <iostream>
#include "magic_enum/magic_enum.hpp"
#include "DeletionQueue.h"
#include "ShaderFunctionDescriptor.h"


class Mesh{

    public :

    Mesh() = default;
    void Draw();


    Mesh(std::vector<VertexData> &meshv ,const std::string &material_name,
         const std::vector<uint32_t> &indices,MTL::Device * metalDevice, DeletionQueue &dq);

    void UpdateShaders(const MTL::Library *lib, DeletionQueue &dq, MTL4::Compiler *metal4Complier, const MTL::PixelFormat &pf);

    void UpdateResidency(MTL::ResidencySet * residency_set);

    void Draw(MTL4::RenderCommandEncoder *encoder, MESHMVP & mvp);

    public:

    MTL::Device * metalDevice;

    MTL::Buffer* index_Data;
    unsigned long indexCount;


    MTL::Buffer* Mesh_Data;
    MTL::Buffer* MVP_Data;

    
    std::string material_name;
    

    MTL::RenderPipelineState *MeshPSO;
    MTL::DepthStencilState * MeshDepthStencilState;
    MTL4::ArgumentTable * Mesharg_table;
};