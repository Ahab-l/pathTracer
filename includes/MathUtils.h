//
// Created by Ahab on 2024/4/2.
//

#ifndef RT_3_MATHUTILS_H
#define RT_3_MATHUTILS_H
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
struct Math
{
    public:

    static inline glm::vec3 max(glm::vec3 p1,glm::vec3 p2){
        glm::vec3 res (std::max(p1.x,p2.x),
                       std::max(p1.y,p2.y),
                       std::max(p1.z,p2.z));
        return res;
    };
    static inline glm::vec3 min(glm::vec3 p1,glm::vec3 p2){
        glm::vec3 res (std::min(p1.x,p2.x),
                       std::min(p1.y,p2.y),
                       std::min(p1.z,p2.z));
        return res;
    };

};
#endif //RT_3_MATHUTILS_H
