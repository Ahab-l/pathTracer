//
// Created by Ahab on 2024/3/16.
//
#include "../glm/glm.hpp"
#include "../Triangle.h"
#ifndef RT_3_MAT4_H
#define RT_3_MAT4_H


inline mat4 getTransformMatrix(vec3 rotateCtrl,vec3 translateCtrl,vec3 scaleCtrl){
    glm::mat4 unit( // 单位矩阵
            glm::vec4(1, 0, 0, 0),
            glm::vec4(0, 1, 0, 0),
            glm::vec4(0, 0, 1, 0),
            glm::vec4(0, 0, 0, 1)
    );
    mat4 scale = glm::scale(unit, scaleCtrl);
    mat4 translate = glm::translate(unit, translateCtrl);
    mat4 rotate = unit;
    rotate = glm::rotate(rotate, glm::radians(rotateCtrl.x), glm::vec3(1, 0,
                                                                       0));
    rotate = glm::rotate(rotate, glm::radians(rotateCtrl.y), glm::vec3(0, 1,
                                                                       0));
    rotate = glm::rotate(rotate, glm::radians(rotateCtrl.z), glm::vec3(0, 0,
                                                                       1));
    mat4 model = translate * rotate * scale;
    return model;
}
#endif //RT_3_MAT4_H
