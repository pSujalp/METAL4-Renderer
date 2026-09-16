
#include <simd/simd.h>
#include "Texture.hpp"


Texture::Texture(const char* filepath, MTL::Device* metalDevice) {
    device = metalDevice;

    stbi_set_flip_vertically_on_load(true);
    unsigned char* image = stbi_load(filepath, &width, &height, &channels, STBI_rgb_alpha);
    assert(image != NULL);

    MTL::TextureDescriptor* desc = MTL::TextureDescriptor::alloc()->init();
    desc->setPixelFormat(MTL::PixelFormatRGBA8Unorm_sRGB);
    desc->setWidth(width);
    desc->setHeight(height);
    desc->setUsage(MTL::TextureUsageShaderRead);
    desc->setStorageMode(MTL::StorageModeShared);

    texture = device->newTexture(desc);
    texture->replaceRegion(MTL::Region(0, 0, 0, width, height, 1), 0, image, 4 * width);
    desc->release();
    stbi_image_free(image);
}

Texture::~Texture() {
    texture->release();
}


CubeTexture::CubeTexture(const char* facePaths[6], MTL::Device* metalDevice) {
    device = metalDevice;

   
    stbi_set_flip_vertically_on_load(false);

    int faceSize = 0;
    unsigned char* faces[6] = {};

    for (int i = 0; i < 6; ++i) {
        int w, h, ch;
        faces[i] = stbi_load(facePaths[i], &w, &h, &ch, STBI_rgb_alpha);
        assert(faces[i] && "CubeTexture: failed to load face image");
        assert(w == h   && "CubeTexture: face image must be square");

        if (i == 0) {
            faceSize = w;
        } else {
            assert(w == faceSize && "CubeTexture: all faces must be the same size");
        }
    }

    MTL::TextureDescriptor* desc = MTL::TextureDescriptor::alloc()->init();
    desc->setTextureType(MTL::TextureTypeCube);
    desc->setPixelFormat(MTL::PixelFormatRGBA8Unorm_sRGB);
    desc->setWidth(faceSize);
    desc->setHeight(faceSize);
    desc->setMipmapLevelCount(1);
    desc->setUsage(MTL::TextureUsageShaderRead);
    desc->setStorageMode(MTL::StorageModeShared);

    texture = device->newTexture(desc);
    assert(texture && "CubeTexture: newTexture returned nil");
    desc->release();

    NS::UInteger bytesPerRow   = 4 * faceSize;
    NS::UInteger bytesPerImage = bytesPerRow * faceSize;

    for (int slice = 0; slice < 6; ++slice) {
        texture->replaceRegion(
            MTL::Region(0, 0, 0, faceSize, faceSize, 1),
           0,
           slice,
            faces[slice],
            bytesPerRow,
            bytesPerImage
        );
        stbi_image_free(faces[slice]);
    }
}

CubeTexture::~CubeTexture() {
    texture->release();
}