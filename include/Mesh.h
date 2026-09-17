#pragma once 



#include <Metal/Metal.hpp>
#include <AppKit/AppKit.hpp>
#include <MetalKit/MetalKit.hpp>
#include <QuartzCore/CAMetalLayer.h>

#include <vector>
#include "Material.hpp"
#include "VertexData.hpp"

class Mesh{

    public :

    Mesh() = default;
    void Draw();


    Mesh(std::vector<Mesh_Vertices> &meshv ,const std::string &material_name, const std::vector<uint32_t> &indices,MTL::Device * metalDevice);

    public:

    MTL::Device* device;
    MTL::Buffer* Mesh_Data;
    MTL::Buffer* index_Data;
    unsigned long indexCount;
    std::string material_name;
    MTL::Device * metalDevice;


};