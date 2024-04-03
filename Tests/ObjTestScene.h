//
// Created by Ahab on 2024/3/16.
//

#include "../includes/Loader/Loader.h"
#include "../includes/Triangle.h"
#include "../includes/Math/Mat4.h"
#ifndef RT_3_OBJTESTSCENE_H
#define RT_3_OBJTESTSCENE_H
void testScene_load_0(std::vector<Triangle>&triangles,std::vector<Material>&materials){
    Material m;
    m.baseColor = vec3(0,1,1);
    materials.push_back(m);
    ObjLoader::readObj("../models/StanfordBunny.obj",triangles, materials,
                       getTransformMatrix(vec3(0, 0, 0), vec3(0.3, -1.6, -.5f), vec3(1.5, 1.5,1.5)),false);


    m.baseColor = vec3(0.725, 0.71, 0.68);
    materials.push_back(m);
    ObjLoader::readObj("../models/quad.obj", triangles, materials, getTransformMatrix(vec3(0, 0, 0),vec3(0, -1.4, 0), vec3(18.83, 0.01, 18.83)), false);

    m.baseColor = vec3(1, 1, 1);
    m.emissive = vec3(20, 20, 20);
    materials.push_back(m);
    ObjLoader::readObj("../models/quad.obj", triangles, materials, getTransformMatrix(vec3(0, 0, 0),vec3(0.0, 1.38, -0.0), vec3(0.7, 0.01, 0.7)), false);
    //readObj("../models/quad1.obj", triangles, m, getTransformMatrix(vec3(0, 0, 0),vec3(0.0, 0.0f, 0.0), vec3(2.f, 2.f, 2.f)), false);
}
void testScene_load_1(std::vector<Triangle>&triangles,std::vector<Material>&materials){
    Material m;
    //m.baseColor  = vec3(1, 0.5, 0.5);
    //m.roughness  = 0.1f;
    //m.specular   = 0.5f;
    //m.subsurface = 0.f;
    //m.metallic   = 0.3f;
    //materials.push_back(m);
    //ObjLoader::readObj("../models/sphere2.obj",triangles, materials,getTransformMatrix(vec3(0, 0, 0), vec3(0.0, 0.f, -0.f), vec3(1.f, 1.f,1.f)),
    //                   false);



    m.baseColor = vec3(0.5, 0.5, 1.);

    //m.subsurface =1.f;
    //m.metallic  = 0.25f;
    //m.emissive = vec3(20, 20, 20);
    materials.push_back(m);
    ObjLoader::readObj("../models/sphere2.obj", triangles, materials, getTransformMatrix(vec3(10, 10, 20),vec3(0.0, 1.38, -0.0), vec3(5., 5.,5.)),
                       true);
    //readObj("../models/quad1.obj", triangles, m, getTransformMatrix(vec3(0, 0, 0),vec3(0.0, 0.0f, 0.0), vec3(2.f, 2.f, 2.f)), false);
    m.baseColor = vec3(1, 0.5, 0.5);
    m.emissive = vec3(20, 20, 20);
    //readObj("../models/quad.obj", triangles, m, getTransformMatrix(vec3(0, 0, 0),vec3(0.0, 1.38, -0.0), vec3(0.7, 0.01, 0.7)), false);
}
void testScene_load_2(std::vector<Triangle>&triangles,std::vector<Material>&materials){
    Material m;
    m.metallic = 0.8f;
    m.roughness =0.4f;
    m.anisotropic =0.4f;
    m.baseColor = vec3(0.725, 0.71, 0.68);
    ObjLoader::readObj("../models/quad.obj", triangles, materials, getTransformMatrix(vec3(0, 0, 0),vec3(0, -1.4, 0), vec3(50, 0.01, 50)),
                       true);

    m.baseColor = vec3(1, 1, 1);
    m.emissive = vec3(20, 10, 10);

    ObjLoader::readObj("../models/sphere2.obj",triangles, materials,getTransformMatrix(vec3(0, 0, 0), vec3(0.0, 1.f, -4.5f), vec3(1.5, 1.5,1.5)),true);
    //readObj("../models/quad1.obj", triangles, m, getTransformMatrix(vec3(0, 0, 0),vec3(0.0, 0.0f, 0.0), vec3(2.f, 2.f, 2.f)), false);
}
#endif //RT_3_OBJTESTSCENE_H
