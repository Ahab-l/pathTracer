//
// Created by Ahab on 2024/3/17.
//
#include "RenderPass.h"
using namespace glm;
void RenderPass::bindData(bool finlPass){
    //生成帧缓冲对象，在帧缓冲对象上附加颜色附件
    if(!finlPass) glGenFramebuffers(1,&FBO);
    glBindFramebuffer(GL_FRAMEBUFFER,FBO);

    //创建顶点缓冲对象,这个对象里面存了的是用来拼成屏幕的两个三角形
    glGenBuffers(1,&vbo);
    glBindBuffer(GL_ARRAY_BUFFER,vbo);

    std::vector<vec3>squre= { vec3(-1, -1, 0), vec3(1, -1, 0), vec3(-1,1, 0), vec3(1, 1, 0), vec3(-1, 1, 0), vec3(1, -1, 0) };
    glBufferData(GL_ARRAY_BUFFER,sizeof(vec3)*squre.size(),&squre[0],GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER,0,sizeof (vec3)*squre.size(),&squre[0]);
    //顶点序列，安排怎么处理这个之前的顶点数据，vao 用来存相关的操作
    glGenVertexArrays(1,&vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
    //不是 finalPass 则生成帧数缓冲的附件
    if(!finlPass){
        std::vector<GLuint>attachments;
        //colorAttachments 里面存的的纹理对象，因此可以直接bind，然后将第i号颜色纹理绑定到 i 号颜色附件，附加的形式是2D纹理，附加的对象是第i号纹理对象
        for (int i = 0; i < colorAttachments.size(); i++) {
            glBindTexture(GL_TEXTURE_2D, colorAttachments[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                                   GL_TEXTURE_2D, colorAttachments[i], 0);// 将颜色纹理绑定到 i 号颜色附件
            attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
        }
        //drawbuffers 告诉OPENGL要使用哪些颜色附件来存储存储渲染结果
        glDrawBuffers(attachments.size(), &attachments[0]);
    }
}
void RenderPass::draw(std::vector<GLuint> texPassArray) {
    //开画，使用这个着色器程序，绑定fbo 和vao
    glUseProgram(program);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glBindVertexArray(vao);
    // 传上一帧的帧缓冲颜色附件

    for (int i = 0; i < texPassArray.size(); i++) {
        glActiveTexture(GL_TEXTURE0+i);
        glBindTexture(GL_TEXTURE_2D, texPassArray[i]);
        std::string uName = "texPass" + std::to_string(i);
        glUniform1i(glGetUniformLocation(program, uName.c_str()), i);
    }
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(0);
}