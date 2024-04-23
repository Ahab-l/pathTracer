//
// Created by Ahab on 2024/4/5.
//

#ifndef RT_3_BVH_TRANSLATOR_H
#define RT_3_BVH_TRANSLATOR_H
#include <map>
#include "bvh.h"
#include "Triangle.h"

namespace RadeonRays
{
    /// This class translates pointer based BVH representation into
    /// index based one suitable for feeding to GPU or any other accelerator
    //
    class BvhTranslator
    {
    public:
        // Constructor
        BvhTranslator() = default;

        struct Node
        {
            glm::vec3 bboxmin;
            glm::vec3 bboxmax;
            glm::vec3 LRLeaf;
        };

        void ProcessBLAS();
        void ProcessTLAS();
        void UpdateTLAS(const Bvh* topLevelBvh, const std::vector<MeshInstance>& instances);
        void Process(const Bvh* topLevelBvh, const std::vector<Mesh*>& meshes, const std::vector<MeshInstance>& instances);
        int topLevelIndex = 0;
        std::vector<Node> nodes;
        int nodeTexWidth;

    private:
        int curNode = 0;
        int curTriIndex = 0;
        std::vector<int> bvhRootStartIndices;
        int ProcessBLASNodes(const Bvh::Node* root);
        int ProcessTLASNodes(const Bvh::Node* root);
        std::vector<MeshInstance> meshInstances;
        std::vector<Mesh*> meshes;


        const Bvh* topLevelBvh;
    };
}
#endif //RT_3_BVH_TRANSLATOR_H
