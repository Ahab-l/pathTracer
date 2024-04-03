//
// Created by Ahab on 2024/3/31.
//
#include "Scene.h"

void Scene::encode_meshes() {
    int nTriangles = triangles.size();

    for(int i=0;i<nTriangles;i++){
        Triangle& t = triangles[i];

        //顶点位置
        encoded_triangles[i].p1 = t.p1;
        encoded_triangles[i].p2 = t.p2;
        encoded_triangles[i].p3 = t.p3;
        //顶点法线
        encoded_triangles[i].n1 = t.n1;
        encoded_triangles[i].n2 = t.n2;
        encoded_triangles[i].n3 = t.n3;
        //材质

        encoded_triangles[i].uv1       = t.uv1;
        encoded_triangles[i].uv2       = t.uv2;
        encoded_triangles[i].uv3       = t.uv3;

        encoded_triangles[i].param1    = vec3(t.materialId,t.padding1,t.padding2);
        encoded_triangles[i].param2    = t.padding3;

    }
    return ;
}
void Scene::createBLAS(){
    //遍历所有meshes 并建立 BVHS
#pragma omp parallel for
    for(int i=0;i<meshes.size();i++){
        std::cout<<"Build bvhs for"<<meshes[i]->name.c_str();
        meshes[i]->BuildBVH();
    }
}
int Scene::AddMeshInstance(const MeshInstance &meshInstance) {
    int id = meshInstances.size();
    meshInstances.push_back(meshInstance);
    return id;
}

void Scene::ProcessScene() {
    std::cout<<"Processing scene data\n";
    createBLAS();

    std::cout<<"Building scene BVH\N";
    createTLAS();

    std::cout<<"Flattening BVH\n";

}
Scene::~Scene(){};