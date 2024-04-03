cmake_minimum_required(VERSION 3.27)
project(RT_3)

include_directories(include)
link_directories(../RT_3/lib)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
include_directories(${PROJECT_SOURCE_DIR}/includes)
include_directories(${PROJECT_SOURCE_DIR}/includes/glew/include/GL)
include_directories(${PROJECT_SOURCE_DIR}/includes/freeglut/include/GL)
include_directories(${PROJECT_SOURCE_DIR}/SOIL2tool/include/SOIL2)
# 生成可执行文件
link_libraries(opengl32)
add_executable(RT_3 hdrloader.cpp main.cpp glad.c
        includes/stb/stb_image.h
        includes/stb/stb_image_resize.h
        includes/stb/stb_image_write.h
        includes/Triangle.h
        includes/Material.h
        includes/BVHNode.h
        includes/Math/MathUtils.h
        includes/Math/Mat4.h
        includes/Loader/Loader.h
        Tests/ObjTestScene.h
        includes/tinygltf/tiny_gltf.h
        includes/RenderPass.h
        includes/RenderPass.cpp
        includes/Texture.h
        includes/Texture.cpp
        includes/Scene.h
        includes/EnvironmentMap.h
        includes/Scene.cpp
        includes/bbox.h
        includes/bbox.cpp
        includes/MathUtils.h
        includes/Triangle.cpp
        includes/split_bvh.h
        includes/bvh.h

)

target_link_libraries(${PROJECT_NAME}
        PUBLIC
        ${PROJECT_SOURCE_DIR}/includes/glew/bin/Release/x64/glew32.dll
        ${PROJECT_SOURCE_DIR}/includes/freeglut/bin/x64/freeglut.dll
        ${PROJECT_SOURCE_DIR}/SOIL2tool/lib/soil2-debug.lib
        ${PROJECT_SOURCE_DIR}/SOIL2tool/lib/soil2-debug.dll
        glfw3
)

