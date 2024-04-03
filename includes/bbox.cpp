//
// Created by Ahab on 2024/4/2.
//
#include "bbox.h"

namespace RadeonRays
{
    glm::vec3 bbox::center() const {return  (pmax+pmin)*0.5f;}
    glm::vec3 bbox::extents() const {return pmax-pmin;}

    float bbox::surface_area() const {
        //计算面积
        glm::vec3 ext = extents();
        return 2.f * (ext.x * ext.y + ext.x * ext.z + ext.y * ext.z);
    }

    void bbox::grow(const glm::vec3 &p) {
        pmin = Math::min(pmin,p);
        pmax = Math::max(pmax,p);
    }

    void bbox::grow(bbox const&b) {
        //吞掉一个包围盒形成新的
        pmin = Math::min(pmin,b.pmin);
        pmax = Math::max(pmax,b.pmax);
    }

    bool bbox::contains(glm::vec3 const& p) const
    {//检查包围盒1是否包围某个点
        glm::vec3 radius = extents() * 0.5f;
        return std::abs(center().x - p.x) <= radius.x &&
               fabs(center().y - p.y) <= radius.y &&
               fabs(center().z - p.z) <= radius.z;
    }
    bbox bboxunion(bbox const& box1, bbox const& box2)
    {   //两个包围盒组成一个新的
        bbox res;
        res.pmin = Math::min(box1.pmin, box2.pmin);
        res.pmax = Math::max(box1.pmax, box2.pmax);
        return res;
    }

    bbox intersection(bbox const& box1,bbox const& box2){
        //返回两个包围盒相交的一部分
        return bbox(Math::max(box1.pmin,box2.pmin),Math::min(box1.pmax,box2.pmax));
    }

    void intersection(bbox const& box1, bbox const& box2, bbox& box)
    {   //返回两个包围盒相交的一部分
        box.pmin = Math::max(box1.pmin, box2.pmin);
        box.pmax = Math::min(box1.pmax, box2.pmax);
    }

    #define BBOX_INTERSECTION_EPS 0.F
    bool intersects(bbox const& box1, bbox const& box2)
    {
        glm::vec3 b1c = box1.center();
        glm::vec3 b1r = box1.extents() * 0.5f;
        glm::vec3 b2c = box2.center();
        glm::vec3 b2r = box2.extents() * 0.5f;

        return (fabs(b2c.x - b1c.x) - (b1r.x + b2r.x)) <= BBOX_INTERSECTION_EPS &&
               (fabs(b2c.y - b1c.y) - (b1r.y + b2r.y)) <= BBOX_INTERSECTION_EPS &&
               (fabs(b2c.z - b1c.z) - (b1r.z + b2r.z)) <= BBOX_INTERSECTION_EPS;
    }

    bool contains(bbox const& box1, bbox const& box2)
    {//检查包围盒1 是否包含包围盒2
        return box1.contains(box2.pmin) && box1.contains(box2.pmax);
    }
}