

#pragma once
#include <Metal/Metal.hpp>
#include <stb_image.h>

class Texture
{
public:
    Texture(const char *filepath, MTL::Device *metalDevice);
    ~Texture();
    MTL::Texture *texture;
    int width, height, channels;

private:
    MTL::Device *device;
};

class CubeTexture
{
public:
    CubeTexture(const char *facePaths[6], MTL::Device *metalDevice);
    ~CubeTexture();
    MTL::Texture *texture;

private:
    MTL::Device *device;
};