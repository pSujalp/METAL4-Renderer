#pragma once 



#include <Metal/Metal.hpp>
#include <AppKit/AppKit.hpp>
#include <MetalKit/MetalKit.hpp>
#include <QuartzCore/CAMetalLayer.h>

#include <vector>
#include "Material.hpp"

class Mesh{

    public :

    Mesh() = default;
    void Draw();


    Mesh(const std::vector<Vertex>& position,
    const std::vector<Vertex>& normal,
    const std::vector<Vertex>& indices,
    const std::vector<Vertex>& tangent,
    const std::vector<Vertex>& bitangent,
    const std::vector<UV> &uv,MTL::Device * metalDevice,
    const std::string &material);

    public:

    MTL::Device* device;
    MTL::Buffer* vertexBuffer;
    MTL::Buffer* indexBuffer;
    MTL::Buffer * uvBuffer;
    MTL::Buffer * tanBuffer;
    MTL::Buffer * bitangetBuffer;
    unsigned long indexCount;
    std::string material;
    MTL::Device * metalDevice;


};