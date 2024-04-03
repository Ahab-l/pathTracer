//
// Created by Ahab on 2024/4/2.
//

#include "Triangle.h"
#include "split_bvh.h"
void Mesh::BuildBVH() {
    const int tri_nums = vertices.size()/3;

    std::vector<RadeonRays::bbox> bounds(tri_nums);
#pragma omp parallel for
    for (int i = 0; i < tri_nums; ++i)
    {
        const glm::vec3 v1 = glm::vec3 (vertices[i * 3 + 0]);
        const glm::vec3 v2 = glm::vec3 (vertices[i * 3 + 0]);
        const glm::vec3 v3 = glm::vec3 (vertices[i * 3 + 0]);

        bounds[i].grow(v1);
        bounds[i].grow(v2);
        bounds[i].grow(v3);
    }

    bvh->Build(&bounds[0], numTris);

}