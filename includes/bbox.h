//
// Created by Ahab on 2024/4/1.
//

#ifndef RT_3_BBOX_H
#define RT_3_BBOX_H

#include "cmath"
#include "algorithm"
#include "limits"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "MathUtils.h"
namespace RadeonRays{
    class bbox{
    public:
        bbox()
            : pmin(glm::vec3 (std::numeric_limits<float>::max(),
                              std::numeric_limits<float>::max(),
                              std::numeric_limits<float>::max()))
            , pmax(glm::vec3 (-std::numeric_limits<float>::max(),
                              -std::numeric_limits<float>::max(),
                              -std::numeric_limits<float>::max()))
        {
        }
        bbox(glm::vec3 const&p)
            :pmin(p)
            ,pmax(p)
        {
        }

        bbox(glm::vec3 const& p1,glm::vec3 const &p2)
        {
            pmax = glm::vec3(
                    std::max(p1.x, p2.x),
                    std::max(p1.y, p2.y),
                    std::max(p1.z, p2.z)
            );
            pmin = glm::vec3 (
                    std::min(p1.x, p2.x),
                    std::min(p1.y, p2.y),
                    std::min(p1.z, p2.z)
            );
        }
        glm::vec3 center()  const;//bounding box的中心点
        glm::vec3 extents() const;//bounding box的边长

        bool contains(glm::vec3 const& p) const;

        inline int maxdim() const{
            glm::vec3 ext = extents();

            if (ext.x >= ext.y && ext.x >= ext.z)
                return 0;
            if (ext.y >= ext.x && ext.y >= ext.z)
                return 1;
            if (ext.z >= ext.x && ext.z >= ext.y)
                return 2;

            return 0;
        }

        float surface_area() const;//常量成员函数 不能修改任何成员变量

        glm::vec3  const& operator[](int i) const {return  *(&pmin+i);}

        void grow(glm::vec3 const&p);

        void grow(bbox const&b);

        glm::vec3 pmin;
        glm::vec3 pmax;


    };
    bbox bboxunion(bbox const& box1, bbox const& box2);
    bbox intersection(bbox const& box1, bbox const& box2);
    void intersection(bbox const& box1, bbox const& box2, bbox& box);
    bool intersects(bbox const& box1, bbox const& box2);
    bool contains(bbox const& box1, bbox const& box2);
}
#endif //RT_3_BBOX_H
