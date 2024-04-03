#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <ctime>

#define  GLUT_DISABLE_ATEXIT_HACK
#include "includes/glad/glad.h"
#include "includes/glfw/glfw3.h"
//#include "includes/glew/include/GL/glew.h"
//#include "includes/freeglut/include/GL/freeglut.h"
#include "includes/glm/glm.hpp"
#include "includes/glm/gtc/matrix_transform.hpp"
#include "includes/glm/gtc/type_ptr.hpp"


#include "includes/Triangle.h"
#include "includes/BVHNode.h"
#include "includes/Math/MathUtils.h"
#include "includes/Math/Mat4.h"
#include "includes/Texture.h"
#include "includes/Loader/Loader.h"
#include "Tests/ObjTestScene.h"
#include "includes/RenderPass.h"
#include "includes/Scene.h"
#include "includes/stb/stb_image_resize.h"


// -----------------------------------------------------------------------------
//


// -----------------------------------------------------------------------------
//

using namespace glm;

Scene* scene = nullptr;

RenderPass pass1;
RenderPass pass2;
RenderPass pass3;
// 相机参数
float upAngle = 0.0;
float rotatAngle = 0.0;
float r_x = 4.0;

GLuint trianglesTextureBuffer;
GLuint nodesTextureBuffer;
GLuint materialTextureBuffer;
GLuint TexturesBuffer;
GLuint lastFrame;
GLuint hdrMap;

static GLFWwindow* window;
std::string gltf_filepath = "../assets/Camera"
                            "_01_4k.gltf";

GLuint getshaderProgram(std::string vshader,std::string fshader){
    //读取源文件
    std::string vSource  = ShaderLoader::readShaderFile(vshader);
    std::string fSource  = ShaderLoader::readShaderFile(fshader);
    const char* vpointer = vSource.c_str();
    const char* fpointer = fSource.c_str();

    //容错
    GLint success;
    GLchar infoLog[512];

    //创建并编译顶点着色器
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader,1,(const GLchar**)(&vpointer),NULL);
    glCompileShader(vertexShader);
    //返回编译器的状态
    glGetShaderiv(vertexShader,GL_COMPILE_STATUS,&success);
    if(!success){
        glGetShaderInfoLog(vertexShader,512,NULL,infoLog);
        std::cout<<"vertex shader compile failed"<<infoLog<<std::endl;
        exit(-1);
    }
    //创建并获取对象着色器
    GLuint fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentshader,1,(const GLchar**)(&fpointer),NULL);
    glCompileShader(fragmentshader);
    glGetShaderiv(fragmentshader,GL_COMPILE_STATUS,&success);
    if(!success){
        glGetShaderInfoLog(fragmentshader,512,NULL,infoLog);
        std::cout<<"fragment shader compile failed:\n"<<infoLog<<std::endl;
        exit(-1);
    }

    //连接着色器到program 对象
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram,vertexShader);
    glAttachShader(shaderProgram,fragmentshader);
    glLinkProgram(shaderProgram);

    //删除着色器
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentshader);

    return shaderProgram;

}
// 绘制
clock_t t1, t2;
double dt, fps;
unsigned int frameCounter = 0;
void display() {
    // 帧计时
    t2 = clock();
    dt = (double)(t2 - t1) / CLOCKS_PER_SEC;
    fps = 1.0 / dt;
    std::cout << "\r";
    //std::cout << std::fixed << std::setprecision(2) << "FPS : " << fps << " iteration times: " << frameCounter;
    t1 = t2;
    // 相机参数
    vec3 eye = vec3(-sin(radians(rotatAngle)) * cos(radians(upAngle)),
                    sin(radians(upAngle)), cos(radians(rotatAngle)) * cos(radians(upAngle)));
    eye.x *= r_x; eye.y *= r_x; eye.z *= r_x;

    mat4 cameraRotate = lookAt(eye, vec3(0, 0, 0), vec3(0, 1, 0)); // 相机注视着原点
    cameraRotate = inverse(cameraRotate); // lookat 的逆矩阵将光线方向进行转换
    // 传 uniform 给 pass1
    glUseProgram(pass1.program);
    glUniform3fv(glGetUniformLocation(pass1.program, "eye"), 1, value_ptr(eye));
    glUniformMatrix4fv(glGetUniformLocation(pass1.program, "cameraRotate"), 1,GL_FALSE, value_ptr(cameraRotate));
    glUniform1ui(glGetUniformLocation(pass1.program, "frameCounter"),frameCounter++);// 传计数器用作随机种子

    //一个时间只能够激活一个纹理
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_BUFFER, trianglesTextureBuffer);
    glUniform1i(glGetUniformLocation(pass1.program, "triangles"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_BUFFER, nodesTextureBuffer);
    glUniform1i(glGetUniformLocation(pass1.program, "nodes"), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, lastFrame);
    glUniform1i(glGetUniformLocation(pass1.program, "lastFrame"), 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, hdrMap);
    glUniform1i(glGetUniformLocation(pass1.program, "hdrMap"), 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_BUFFER, materialTextureBuffer);
    glUniform1i(glGetUniformLocation(pass1.program, "materials"), 4);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D_ARRAY, TexturesBuffer);
    glUniform1i(glGetUniformLocation(pass1.program, "textureMapsArrayTex"), 5);
// 绘制
    pass1.draw();
    pass2.draw(pass1.colorAttachments);
    pass3.draw(pass2.colorAttachments);
    glfwSwapBuffers(window);
}
// 每一帧
void frameFunc() {
    glfwPostEmptyEvent();
}
double lastX = 0.0,lastY=0.0;
void mouse(int x,int y){
    frameCounter = 0;
    //调整旋转
    rotatAngle += 150*(x-lastX)/512;
    upAngle    += 150*(y-lastY)/512;
    upAngle     = min(upAngle,89.0f);
    upAngle     = max(upAngle,-89.0f);
    lastX = x,lastY = y;
    glfwPostEmptyEvent();
}
void mouseDown(int button,int state,int x,int y){
    if(button == GLFW_MOUSE_BUTTON_LEFT&& state==GLFW_PRESS){
        lastX=x,lastY=y;
    }
}
void mouseWheel(int wheel,int direction,int x,int y){
    frameCounter = 0;
    r_x+=-direction*0.1;
    //if(r_x<1.5f)
    //    r_x=1.5f;
    glfwPostEmptyEvent();
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    // yoffset 表示滚轮的滚动距离，向前滚动为正，向后滚动为负
    // 你可以根据 yoffset 的值来更改你的数据
    frameCounter = 0;
    r_x+=-yoffset*0.1;
    //if(r_x<1.5f)
    //    r_x=1.5f;
    glfwPostEmptyEvent();

}
void init(int &argc,char** argv){
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(1080, 1080, "OpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return ;
    }
    glfwMakeContextCurrent(window);
    //glutInit(&argc,argv);
    //glutInitDisplayMode(GLUT_RGBA|GLUT_DEPTH);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return ;
    }
    //glutInitWindowSize(1080,1080);
    //glutInitWindowPosition(350,50);
    //glutCreateWindow("OpenGL Path Tracing");


}
bool mousePressed = false;
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        mousePressed = true; // 鼠标左键被按下，设置标志位

        glfwGetCursorPos(window, &lastX, &lastY);
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        mousePressed = false; // 鼠标左键被释放，重置标志位
    }
}
void cursorPositionCallback(GLFWwindow* window, double x, double y)
{
    if (mousePressed) // 如果鼠标被按下
    {
        frameCounter = 0;
        //调整旋转
        rotatAngle += 150*(x-lastX)/512;
        upAngle    += 150*(y-lastY)/512;
        upAngle     = min(upAngle,89.0f);
        upAngle     = max(upAngle,-89.0f);
        lastX = x,lastY = y;
        //glfwPostEmptyEvent();
    }
}

void loop(){
    while (!glfwWindowShouldClose(window))
    {
        display();
        glfwSwapBuffers(window);
        glfwSetMouseButtonCallback(window, mouseButtonCallback); // 设置鼠标按键的回调函数
        glfwSetCursorPosCallback(window, cursorPositionCallback); // 设置鼠标移动的回调函数
        glfwSetScrollCallback(window, scroll_callback);
        glfwPollEvents();
    }
    //frameFunc();
    //glutMotionFunc(mouse);
    //glutMouseFunc(mouseDown);
    //glutMouseWheelFunc(mouseWheel);
    //glutMainLoop();
}
int main(int argc,char** argv) {
    int right =-1;
    std::vector<int> test = {1,2,3,4};

    if(!scene->initialized)
        scene->ProcessScene();
    while( right<test.size())
        right++;
    //创造缓存空间
    init(argc,argv);

    //----------------------------------------------------------------------

    //testScene_load_1(triangles,materials);
    bool y =GLTF_SceneLoader::readGLTF(scene,gltf_filepath);
    int nTriangles = scene->triangles.size();
    std::cout<<"model load compelete:"<<nTriangles<<" triangles"<<std::endl;
    //----------------------------------------------------------------------

    BVHNode::buildBVH_SAH(scene->triangles, scene->nodes, 0, nTriangles - 1, 8);
    std::cout<<"BVH BUILD:total"<<scene->nodes.size()<<" nodes"<<std::endl;
    std::vector<Triangle_encode> triangles_encoded=Triangle_encode::EncodeTriangles(scene->triangles);


    // 编码 BVHNode, aabb
    std::vector<BVHNode_encode> nodes_encoded = BVHNode_encode::EncodeBVH(scene->nodes);
    //生成纹理
    //三角形数组
    //生成缓冲区 把数据存入缓冲区，然后生成纹理,通过GL_TEXTURE_BUFFER把数据绑定到，数据在之前创建的buffer之中，把纹理单元和buffer 绑定，、
    // 之后纹理单元就是使用buffer之中的数据了,这里是texture buffer 是一种纹理，他不是实际占用了一块内存，是去绑定的buffer对象那里取数据
    GLuint tbo0;
    glGenBuffers(1, &tbo0);
    glBindBuffer(GL_TEXTURE_BUFFER, tbo0);
    glBufferData(GL_TEXTURE_BUFFER, triangles_encoded.size() *sizeof(Triangle_encode), &triangles_encoded[0], GL_STATIC_DRAW);//生成缓存 存储三角形信息
    glGenTextures(1, &trianglesTextureBuffer);//生成纹理
    glBindTexture(GL_TEXTURE_BUFFER, trianglesTextureBuffer);//绑定纹理到GL_TEXTURE_BUFFER
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, tbo0);//把纹理绑定到缓冲对象上 这样就可以读纹理了
    glBindTexture(GL_TEXTURE_BUFFER, 0);

    //生成纹理
    GLuint tbo1;
    glGenBuffers(1,&tbo1);
    glBindBuffer(GL_TEXTURE_BUFFER,tbo1);
    glBufferData(GL_TEXTURE_BUFFER,nodes_encoded.size()*sizeof (BVHNode_encode),&nodes_encoded[0],GL_STATIC_DRAW);
    glGenTextures(1,&nodesTextureBuffer);
    glBindTexture(GL_TEXTURE_BUFFER,nodesTextureBuffer);
    glTexBuffer(GL_TEXTURE_BUFFER,GL_RGB32F,tbo1);
    glBindTexture(GL_TEXTURE_BUFFER, 0);

    //生成材质
    GLuint  tbo2;
    glGenBuffers(1,&tbo2);
    glBindBuffer(GL_TEXTURE_BUFFER,tbo2);
    glBufferData(GL_TEXTURE_BUFFER,scene->materials.size()*sizeof (Material),&scene->materials[0],GL_STATIC_DRAW);
    glGenTextures(1,&materialTextureBuffer);
    glBindTexture(GL_TEXTURE_BUFFER,materialTextureBuffer);
    glTexBuffer(GL_TEXTURE_BUFFER,GL_RGB32F,tbo2);
    glBindTexture(GL_TEXTURE_BUFFER, 0);


    glGenTextures(1, &TexturesBuffer);
    glBindTexture(GL_TEXTURE_2D_ARRAY, TexturesBuffer);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, 2048, 2048, scene->textureMapsArray.size()/(2048*2048*4), 0, GL_RGBA, GL_UNSIGNED_BYTE, &scene->textureMapsArray[0]);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    //管线配置
    //创建三张2D纹理，然后把为pass1身处帧缓冲，把这三个纹理作为帧缓冲的颜色附件，附上去，FBO对象直接存在pass1对象里面了
    pass1.program = getshaderProgram("../vshaders/vshader.vsh","../fshaders/fshader.fsh");//读入着色器
    pass1.colorAttachments.push_back(TextureLoader::getTextureRGB32F(pass1.width,pass1.height));                         //添加颜色附件
    pass1.colorAttachments.push_back(TextureLoader::getTextureRGB32F(pass1.width,pass1.height));
    pass1.colorAttachments.push_back(TextureLoader::getTextureRGB32F(pass1.width,pass1.height));
    pass1.bindData();

    HDRLoaderResult hdrRes;
    bool r = HDRLoader::load("../HDR/peppermint_powerplant_4k.hdr",hdrRes);
    hdrMap = TextureLoader::getTextureRGB32F(hdrRes.width,hdrRes.height);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB32F,hdrRes.width,hdrRes.height,0,GL_RGB,GL_FLOAT,hdrRes.cols);

    glUseProgram(pass1.program);
    glUniform1i(glGetUniformLocation(pass1.program, "nTriangles"),scene->triangles.size());
    glUniform1i(glGetUniformLocation(pass1.program, "nNodes"), scene->nodes.size());
    glUniform1i(glGetUniformLocation(pass1.program, "width"), pass1.width);
    glUniform1i(glGetUniformLocation(pass1.program, "height"), pass1.height);
    glUseProgram(0);
    //pass 大体和pass1 一致 但是只接收一个颜色附件
    pass2.program = getshaderProgram("../vshaders/vshader.vsh",
                                     "../fshaders/pass2.fsh");
    lastFrame = TextureLoader::getTextureRGB32F(pass2.width, pass2.height);
    pass2.colorAttachments.push_back(lastFrame);
    pass2.bindData();
    //三个着色器对象，前两个都是用来画颜色附件的，第三个用来输出颜色到屏幕
    pass3.program = getshaderProgram("../vshaders/vshader.vsh",
                                     "../fshaders/pass3.fsh");
    pass3.bindData(true);

//----------------------------------------------------------------------------------------//
    std::cout << "begin..." << std::endl << std::endl;
    //glEnable(GL_DEPTH_TEST); // 开启深度测试
    glClearColor(0.0, 0.0, 0.0, 1.0); // 背景颜色 -- 黑


    loop();

    int width=0;int height =0;
    std::cout<<"what"<<std::endl;

    //unsigned char* image = SOIL_load_image("T_image4.png", &width, &height, 0, SOIL_LOAD_RGBA);

    return 0;
}
