//
// Created by Ahab on 2024/3/16.
//

#ifndef RT_3_MATERIAL_H
#define RT_3_MATERIAL_H
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
using namespace glm;
enum AlphaMode
{
    Opaque,
    Blend,
    Mask
};

enum MediumType
{
    None,
    Absorb,
    Scatter,
    Emissive
};
struct Material {
//每次读三个
    vec3 emissive    = vec3(0, 0, 0); // 作为光源时的发光颜色✔
    vec3 baseColor   = vec3(0.2, 0.4, 0.8);// 基础颜色✔

    float subsurface     = 0.0;         //次表面✔
    float metallic       = 0.0;//✔
    float specular       = 0.0;           //高光项✔

    float specularTint   = 0.0;       //
    float roughness      = 0.5f;          //粗糙度✔
    float anisotropic    = 0.0;        //各项异性✔

    float sheen          = 0.0;              //✔
    float sheenTint      = 0.0;          //✔
    float clearcoat      = 0.0;          //✔

    float clearcoatGloss = 0.0;     //✔
    float IOR            = 1.5f;                //✔
    float transmission   = 0.0;

    float specTrans      = 0.0f;
    float mediumType     = 0.0f;
    float mediumDensity  = 0.0f;

    vec3 mediumColor     = vec3(1.0f,1.0f,1.0f);

    float mediumAnisortropy      =  0.0f;
    float baseColorTexId         = -1.0f;
    float metallicRoughnessTexID = -1.0f;

    float normalmapTexID         = -1.0f;
    float emissionmapTexID       = -1.0f;
    float opacity                =  1.0f;

    float alphaMode              =  0.0f;
    float alphaCutoff            =  0.0f;
    float padding                =  0.0f;

};
#endif //RT_3_MATERIAL_H
