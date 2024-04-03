//
// Created by Ahab on 2024/3/20.
//
#include "Texture.h"

Texture::Texture(std::string texName, unsigned char* data, int w, int h, int c) : name(texName)
        , width(w)
        , height(h)
        , components(c)
{
    texData.resize(width * height * components);
    std::copy(data, data + width * height * components, texData.begin());
}

bool Texture::LoadTexture(const std::string& filename)
{
    name = filename;
    components = 4;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, NULL, components);
    if (data == nullptr)
        return false;
    texData.resize(width * height * components);
    std::copy(data, data + width * height * components, texData.begin());
    stbi_image_free(data);
    return true;
}