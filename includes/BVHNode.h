//
// Created by Ahab on 2024/3/16.
//
#ifndef RT_3_BVHNODE_H
#define RT_3_BVHNODE_H
#include <algorithm>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "Math/MathUtils.h"
#define INF 114514.0

using namespace glm;
struct BVHNode{
    int left,right;
    int n,index;
    vec3 AA,BB;
    public:
    static int buildBVH(std::vector<Triangle>&triangles,std::vector<BVHNode>& nodes,int l,int r,int n){
        if(l>r) return 0;

        nodes.push_back(BVHNode());
        int id =nodes.size()-1;
        nodes[id].left=nodes[id].right=nodes[id].n=nodes[id].index=0;
        nodes[id].AA  =vec3 (1145141919,1145141919,1145141919);
        nodes[id].BB  =vec3 (-1145141919,-1145141919,-1145141919);

        for(int i=l;i<=r;i++){
            float minx = min(   triangles[i].p1.x,min(   triangles[i].p2.x,triangles[i].p3.x));
            float miny = min(triangles[i].p1.y,min(triangles[i].p2.y,triangles[i].p3.y));
            float minz = min(triangles[i].p1.z,min(triangles[i].p2.z,triangles[i].p3.z));
            nodes[id].AA.x=min(   nodes[id].AA.x,minx);
            nodes[id].AA.y=min(nodes[id].AA.y,miny);
            nodes[id].AA.z=min(nodes[id].AA.z,minz);
            //最大点BB
            float maxx = max(triangles[i].p1.x,max(triangles[i].p2.x,triangles[i].p3.x));
            float maxy = max(triangles[i].p1.y,max(triangles[i].p2.y,triangles[i].p3.y));
            float maxz = max(triangles[i].p1.z,max(triangles[i].p2.z,triangles[i].p3.z));
            nodes[id].BB.x=min(   nodes[id].BB.x,maxx);
            nodes[id].BB.y=min(nodes[id].BB.y,maxy);
            nodes[id].BB.z=min(nodes[id].BB.z,maxz);
        }
        if((r-l+1)<=n){
            nodes[id].n=r-l+1;
            nodes[id].index=l;
            return id;
        }
        float lenx = nodes[id].BB.x - nodes[id].AA.x;
        float leny = nodes[id].BB.y - nodes[id].AA.y;
        float lenz = nodes[id].BB.z -nodes[id].AA.z;
        if(lenx>=leny&&lenx>=lenz){
            std::sort(triangles.begin()+l,triangles.begin()+r+l,Math::cmpx);
        }
        // 按 y 划分
        if (leny >= lenx && leny >= lenz)
            std::sort(triangles.begin() + l, triangles.begin() + r + 1, Math::cmpy);
        // 按 z 划分
        if (lenz >= lenx && lenz >= leny)
            std::sort(triangles.begin() + l, triangles.begin() + r + 1, Math::cmpz);
        // 递归
        int mid = (l + r) / 2;
        int left = buildBVH(triangles, nodes, l, mid, n);
        int right = buildBVH(triangles, nodes, mid + 1, r, n);
        nodes[id].left = left;
        nodes[id].right = right;
        return id;

    }
    static int buildBVH_SAH(std::vector<Triangle>& triangles,std::vector<BVHNode>&nodes,int l,int r,int n){
        if(l>r) return 0;

        nodes.push_back(BVHNode());
        int id=nodes.size()-1;
        nodes[id].left=nodes[id].right=nodes[id].n=nodes[id].index=0;
        nodes[id].AA = vec3(1145141919, 1145141919, 1145141919);
        nodes[id].BB = vec3(-1145141919, -1145141919, -1145141919);

        // 计算 AABB
        for (int i = l; i <= r; i++) {
            // 最小点 AA
            float minx = min(triangles[i].p1.x, min(triangles[i].p2.x,
                                                    triangles[i].p3.x));
            float miny = min(triangles[i].p1.y, min(triangles[i].p2.y,
                                                    triangles[i].p3.y));
            float minz = min(triangles[i].p1.z, min(triangles[i].p2.z,
                                                    triangles[i].p3.z));
            nodes[id].AA.x = min(nodes[id].AA.x, minx);
            nodes[id].AA.y = min(nodes[id].AA.y, miny);
            nodes[id].AA.z = min(nodes[id].AA.z, minz);
            // 最大点 BB
            float maxx = max(triangles[i].p1.x, max(triangles[i].p2.x,
                                                    triangles[i].p3.x));
            float maxy = max(triangles[i].p1.y, max(triangles[i].p2.y,
                                                    triangles[i].p3.y));
            float maxz = max(triangles[i].p1.z, max(triangles[i].p2.z,
                                                    triangles[i].p3.z));
            nodes[id].BB.x = max(nodes[id].BB.x, maxx);
            nodes[id].BB.y = max(nodes[id].BB.y, maxy);
            nodes[id].BB.z = max(nodes[id].BB.z, maxz);
        }
        if((r-l+1)<=n){
            nodes[id].n=r-l+1;
            nodes[id].index=l;
            return id;
        }
        float Cost = INF;
        int Axis   = 0;
        int Split  = (l+r)/2;
        for(int axis = 0;axis<3;axis++) {
            //分别按x,y,z排序
            //分别按x,y,z排序
            if (axis == 0) std::sort(triangles.begin() + l, triangles.begin() + r + 1, Math::cmpx);
            //分别按x,y,z排序
            if (axis == 1) std::sort(triangles.begin() + l, triangles.begin() + r + 1, Math::cmpy);
            //分别按x,y,z排序
            if (axis == 2) std::sort(triangles.begin() + l, triangles.begin() + r + 1, Math::cmpz);

            //leftMax[i] [l,i]最大的xyz值
            // leftMin[i]: [l, i] 中最小的 xyz 值
            std::vector<vec3> leftMax(r - l + 1, vec3(-INF, -INF, -INF));
            std::vector<vec3> leftMin(r - l + 1, vec3(INF, INF, INF));
            // 计算前缀 注意 i-l 以对齐到下标 0
            for (int i = l; i <= r; i++) {
                Triangle &t = triangles[i];
                int bias = (i == l) ? 0 : 1; // 第一个元素特殊处理
                leftMax[i - l].x = max(leftMax[i - l - bias].x, max(t.p1.x, max(t.p2.x, t.p3.x)));
                leftMax[i - l].y = max(leftMax[i - l - bias].y, max(t.p1.y, max(t.p2.y, t.p3.y)));
                leftMax[i - l].z = max(leftMax[i - l - bias].z, max(t.p1.z, max(t.p2.z, t.p3.z)));
                leftMin[i - l].x = min(leftMin[i - l - bias].x, min(t.p1.x, min(t.p2.x, t.p3.x)));
                leftMin[i - l].y = min(leftMin[i - l - bias].y, min(t.p1.y, min(t.p2.y, t.p3.y)));
                leftMin[i - l].z = min(leftMin[i - l - bias].z, min(t.p1.z, min(t.p2.z, t.p3.z)));
            }
            // rightMax[i]: [i, r] 中最大的 xyz 值
            // rightMin[i]: [i, r] 中最小的 xyz 值
            std::vector<vec3> rightMax(r - l + 1, vec3(-INF, -INF, -INF));
            std::vector<vec3> rightMin(r - l + 1, vec3(INF, INF, INF));
            // 计算后缀 注意 i-l 以对齐到下标 0
            for (int i = r; i >= l; i--) {
                Triangle &t = triangles[i];
                int bias = (i == r) ? 0 : 1; // 第一个元素特殊处理
                rightMax[i - l].x = max(rightMax[i - l + bias].x, max(t.p1.x, max(t.p2.x, t.p3.x)));
                rightMax[i - l].y = max(rightMax[i - l + bias].y, max(t.p1.y, max(t.p2.y, t.p3.y)));
                rightMax[i - l].z = max(rightMax[i - l + bias].z, max(t.p1.z, max(t.p2.z, t.p3.z)));
                rightMin[i - l].x = min(rightMin[i - l + bias].x, min(t.p1.x, min(t.p2.x, t.p3.x)));
                rightMin[i - l].y = min(rightMin[i - l + bias].y, min(t.p1.y, min(t.p2.y, t.p3.y)));
                rightMin[i - l].z = min(rightMin[i - l + bias].z, min(t.p1.z, min(t.p2.z, t.p3.z)));
            }
            // 遍历寻找分割
            float cost = INF;
            int split = l;
            for (int i = l; i <= r - 1; i++) {
                float lenx, leny, lenz;
                // 左侧 [l, i]
                vec3 leftAA = leftMin[i - l];
                vec3 leftBB = leftMax[i - l];
                lenx = leftBB.x - leftAA.x;
                leny = leftBB.y - leftAA.y;
                lenz = leftBB.z - leftAA.z;
                float leftS = 2.0 * ((lenx * leny) + (lenx * lenz) + (leny * lenz));
                float leftCost = leftS * (i - l + 1);
                // 右侧 [i+1, r]
                vec3 rightAA = rightMin[i + 1 - l];
                vec3 rightBB = rightMax[i + 1 - l];
                lenx = rightBB.x - rightAA.x;
                leny = rightBB.y - rightAA.y;
                lenz = rightBB.z - rightAA.z;
                float rightS = 2.0 * ((lenx * leny) + (lenx * lenz) + (leny *
                                                                       lenz));
                float rightCost = rightS * (r - i);
                // 记录每个分割的最小答案
                float totalCost = leftCost + rightCost;
                if (totalCost < cost) {
                    cost = totalCost;
                    split = i;
                }
            }
            if (cost < Cost) {
                Cost = cost;
                Axis = axis;
                Split = split;
            }
        }
        // 按最佳轴分割
        if (Axis == 0) std::sort(&triangles[0] + l, &triangles[0] + r + 1, Math::cmpx);
        if (Axis == 1) std::sort(&triangles[0] + l, &triangles[0] + r + 1, Math::cmpy);
        if (Axis == 2) std::sort(&triangles[0] + l, &triangles[0] + r + 1, Math::cmpz);
        // 递归
        int left = buildBVH_SAH(triangles, nodes, l, Split, n);
        int right = buildBVH_SAH(triangles, nodes, Split + 1, r, n);
        //必须返回递归建立返回的下标，因为不知道递归建立的深度，该node[id].left 可以直接由left+1得到，但是node[id].right则必须根据左树的深度确定，毕竟是深度优先的。
        nodes[id].left = left;
        nodes[id].right = right;
        return id;
    }
};
struct BVHNode_encode {
    vec3 childs; // (left, right, 保留)
    vec3 leafInfo; // (n, index, 保留)
    vec3 AA, BB;
    static inline std::vector<BVHNode_encode> EncodeBVH(std::vector<BVHNode>&nodes){
        std::vector<BVHNode_encode> nodes_encoded(nodes.size());
        for (int i = 0; i < nodes.size(); i++) {
            nodes_encoded[i].childs   = vec3(nodes[i].left, nodes[i].right, 0);
            nodes_encoded[i].leafInfo = vec3(nodes[i].n, nodes[i].index, 0);
            nodes_encoded[i].AA       = nodes[i].AA;
            nodes_encoded[i].BB       = nodes[i].BB;
        }
        return nodes_encoded;
    }
};
#endif //RT_3_BVHNODE_H
