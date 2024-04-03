//
// Created by Ahab on 2024/3/30.
//
#ifndef RT_3_SCENE_H
#define RT_3_SCENE_H
#include <string>
#include <vector>
#include <Material.h>
#include <Triangle.h>
#include <Texture.h>
#include <EnvironmentMap.h>
#include <BVHNode.h>


class Scene{
    public:
        Scene(){
            BVHNode testNode;
            testNode.left  = 255;
            testNode.right = 128;
            testNode.n     = 30 ;
            testNode.AA    = vec3 (1,1,0);
            testNode.BB    = vec3 (0,1,0);
            nodes.push_back(testNode);
        };
        Scene(std::string filepath){};
        ~Scene();
        void ProcessScene();
        int AddMeshInstance(const MeshInstance & meshInstance);
        void encode_meshes();

        std::vector<Triangle>        triangles;
        std::vector<Triangle_encode> encoded_triangles;

        std::vector<Mesh*>            meshes;
        std::vector<MeshInstance>    meshInstances;

        std::vector<Material>      materials;
        std::vector<Texture*>      textures;
        std::vector<unsigned char> textureMapsArray;
        EnvironmentMap             *envMap;
        std::vector<BVHNode>       nodes;

        bool initialized;

    private:
    //RadeonRays::Bvh* sceneBvh;
    void   createBLAS();
    void   createTLAS();
};



#endif //RT_3_SCENE_H
