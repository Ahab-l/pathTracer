//
// Created by Ahab on 2024/3/16.
//


#ifndef RT_3_TRIANGLE_H
#define RT_3_TRIANGLE_H
#include "Material.h"
#include "vector"
#include "string"
#include ""
struct Triangle {
    vec3 p1, p2, p3; // 顶点坐标
    vec3 n1, n2, n3; // 顶点法线
    vec2 uv1,uv2,uv3;
    float materialId;
    float padding1;
    float padding2;
    vec3  padding3;


};

class Mesh
{
public:
    Mesh()
    {
        //bvh = new RadeonRays::SplitBvh(2.0f, 64, 0, 0.001f, 0);
        //bvh = new RadeonRays::Bvh(2.0f, 64, false);
    }
    ~Mesh() {  }

    void BuildBVH();
    //bool LoadFromFile(const std::string& filename);

    std::vector<vec3> vertices; // Vertex + texture Coord (u/s)
    std::vector<vec3> normals;  // Normal + texture Coord (v/t)
    std::vector<vec2> uvs;
    //RadeonRays::Bvh* bvh;
    std::string name;
};

class MeshInstance{
public:
    MeshInstance(std::string name,int meshID,mat4 xform,int matID)
        :name(name),meshID(meshID),transform(xform),materialID(matID)
        {}
        ~MeshInstance(){}

        mat4 transform;
        std::string name;

        float materialID;
        float meshID;
};

struct Triangle_encode{
    vec3 p1, p2, p3; // 顶点坐标
    vec3 n1, n2, n3; // 顶点法线
    vec2 uv1,uv2,uv3;
    vec3 param1; // (u, v, materialIndex)
    vec3 param2; // (transform)


    static inline std::vector<Triangle_encode> EncodeTriangles(std::vector<Triangle>&triangles) {
        int nTriangles = triangles.size();
        std::vector<Triangle_encode> triangles_encoded(nTriangles);
        for(int i=0;i<nTriangles;i++){
            Triangle& t = triangles[i];

            //顶点位置
            triangles_encoded[i].p1 = t.p1;
            triangles_encoded[i].p2 = t.p2;
            triangles_encoded[i].p3 = t.p3;
            //顶点法线
            triangles_encoded[i].n1 = t.n1;
            triangles_encoded[i].n2 = t.n2;
            triangles_encoded[i].n3 = t.n3;
            //材质

            triangles_encoded[i].uv1       = t.uv1;
            triangles_encoded[i].uv2       = t.uv2;
            triangles_encoded[i].uv3       = t.uv3;

            triangles_encoded[i].param1    = vec3(t.materialId,t.padding1,t.padding2);
            triangles_encoded[i].param2    = t.padding3;


        }
        return triangles_encoded;
    }
};

#endif //RT_3_TRIANGLE_H
