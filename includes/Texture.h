//
// Created by Ahab on 2024/3/20.
//

#ifndef RT_3_TEXTURE_H
#define RT_3_TEXTURE_H
#include <iostream>
#include <vector>
#include "stb/stb_image.h"
#include "stb/stb_image_resize.h"
class Texture
{
public:
    Texture() : width(0), height(0), components(0) {};
    Texture(std::string texName, unsigned char* data, int w, int h, int c);
    ~Texture() { }

    bool LoadTexture(const std::string& filename);

    int width;
    int height;
    int components;
    std::vector<unsigned char> texData;
    std::string name;
};
#endif //RT_3_TEXTURE_H
