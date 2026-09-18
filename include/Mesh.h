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

class Mesh{

    public :

    Mesh() = default;
    void Draw();


    Mesh(std::vector<Mesh_Vertices> &meshv ,const std::string &material_name,
         const std::vector<uint32_t> &indices,MTL::Device * metalDevice, DeletionQueue &dq);

    public:

    MTL::Device * metalDevice;

    MTL::Buffer* index_Data;
    unsigned long indexCount;


    MTL::Buffer* Mesh_Data;
    MTL::Buffer* material_Data;
    MTL::Buffer* MVP_Data;

    
    std::string material_name;
    

    MTL::RenderPipelineState *MeshPSO;
    MTL::DepthStencilState * MeshDepthStencilState;
    MTL4::ArgumentTable * Mesharg_table;
};