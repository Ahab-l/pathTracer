//
// Created by Ahab on 2024/3/17.
//

#ifndef RT_3_RENDERPASS_H
#define RT_3_RENDERPASS_H
#include "RenderPass.h"
#include <vector>
//#include "glew/include/GL/glew.h"
//#include "freeglut/include/GL/freeglut.h"
#include "glad/glad.h"
//#include "freeglut/include/GL/freeglut.h"
#include "glfw/glfw3.h"
#include "string"
#include "glm/glm.hpp"
class RenderPass{
public:
    GLuint  FBO =0;
    GLuint  vao,vbo;
    std::vector<GLuint> colorAttachments;
    GLuint program;
    int width =1080;
    int height = 1080;

    void bindData(bool finlPass = false);
    void draw(std::vector<GLuint> texPassArray = {});
};
#endif //RT_3_RENDERPASS_H
