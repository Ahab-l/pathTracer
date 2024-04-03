//
// Created by Ahab on 2024/3/16.
//

#ifndef RT_3_MATHUTILS_H
#define RT_3_MATHUTILS_H
#include "../glm/glm.hpp"
#include "../Triangle.h"
using  namespace glm;
struct Math
{
public:
    static  bool cmpx(const Triangle& t1, const Triangle& t2) {
        vec3 center1 = (t1.p1 + t1.p2 + t1.p3) / vec3(3, 3, 3);
        vec3 center2 = (t2.p1 + t2.p2 + t2.p3) / vec3(3, 3, 3);
        return center1.x < center2.x;
    }
    static  bool cmpy(const Triangle& t1, const Triangle& t2) {
        vec3 center1 = (t1.p1 + t1.p2 + t1.p3) / vec3(3, 3, 3);
        vec3 center2 = (t2.p1 + t2.p2 + t2.p3) / vec3(3, 3, 3);
        return center1.y < center2.y;
    }
    static  bool cmpz(const Triangle& t1, const Triangle& t2) {
        vec3 center1 = (t1.p1 + t1.p2 + t1.p3) / vec3(3, 3, 3);
        vec3 center2 = (t2.p1 + t2.p2 + t2.p3) / vec3(3, 3, 3);
        return center1.z < center2.z;
    }
};
#endif //RT_3_MATHUTILS_H
