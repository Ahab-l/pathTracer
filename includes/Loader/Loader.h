//
// Created by Ahab on 2024/3/16.
//
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#ifndef RT_3_LOADER_H
#define RT_3_LOADER_H
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>
#include <cstdint>
#include "../tinygltf/tiny_gltf.h"
#include "../includes/Texture.h"
#include "../includes/Scene.h"


struct ShaderLoader{
    public:
        inline static std::string readShaderFile(std::string filepath){
            std::string res,line;
            std::ifstream  fin(filepath);
            if(!fin.is_open()){
                std::cout<<"file"<<filepath<<"open failed"<<std::endl;
                exit(-1);
            }
            while(std::getline(fin,line)){
                res+=line+'\n';
            }
            fin.close();
            return res;
        }

};
struct GLTFLoader{
    struct Primitive{
        int primitiveId;
        int materialId;
    };
    public:
        //https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html
        static inline void LoadMeshes(std::vector<Triangle>&triangles,tinygltf::Model& model,std::map<int,std::vector<Primitive>>& meshPrimMap){
            for(int gltfMeshIdx = 0;gltfMeshIdx<model.meshes.size();gltfMeshIdx++){
                tinygltf::Mesh gltfMesh = model.meshes[gltfMeshIdx];
                for(int gltfPrimIdx =0;gltfPrimIdx<gltfMesh.primitives.size();gltfPrimIdx++)
                {
                    tinygltf::Primitive prim = gltfMesh.primitives[gltfPrimIdx];

                    //Skip points and lines
                    if(prim.mode !=TINYGLTF_MODE_TRIANGLES)
                        continue;

                    int indicesIndex  = prim.indices;
                    int positionIndex = -1;
                    int normalIndex   = -1;
                    int uv0Index      = -1;

                    //检查primitive 是否带有这些属性如果没有，通过map.count()检查
                    if(prim.attributes.count("POSITION")>0)
                    {
                        positionIndex = prim.attributes["POSITION"];
                    }

                    if (prim.attributes.count("NORMAL") > 0)
                    {
                        normalIndex = prim.attributes["NORMAL"];
                    }

                    if (prim.attributes.count("TEXCOORD_0") > 0)
                    {
                        uv0Index = prim.attributes["TEXCOORD_0"];
                    }

                    //Vertex positions
                    //通过层层索引找到buffer，access 能索引到一个buffer view，bufferview 可以索引到一个buffer，不过具体怎么读bufferview内offset是多少还是在accessor内
                    tinygltf::Accessor     positionAccessor = model.accessors[positionIndex];
                    tinygltf::BufferView positionBufferView = model.bufferViews[positionAccessor.bufferView];
                    const tinygltf::Buffer&  positionBuffer = model.buffers[positionBufferView.buffer];
                    const uint8_t*    positionBufferAddress = positionBuffer.data.data();
                    //Element size, in bytes, is (size in bytes of the 'componentType') * (number of components defined by 'type').
                    int                      positionStride = tinygltf::GetComponentSizeInBytes(positionAccessor.componentType)*tinygltf::GetNumComponentsInType(positionAccessor.type);

                    //Recheck
                    if(positionBufferView.byteStride>0)
                        positionStride = positionBufferView.byteStride;

                    //FIXME: Some GLTF files like TriangleWithoutIndices gltf have no indices
                    //Vertex indice
                    tinygltf::Accessor     indexAccessor = model.accessors[indicesIndex];
                    tinygltf::BufferView indexBufferView = model.bufferViews[indexAccessor.bufferView];
                    const tinygltf::Buffer&  indexBuffer = model.buffers[indexBufferView.buffer];
                    const uint8_t*    indexBufferAddress = indexBuffer.data.data();
                    int                      indexStride = tinygltf::GetComponentSizeInBytes(indexAccessor.componentType) * tinygltf::GetNumComponentsInType(indexAccessor.type);

                    //Normals
                    tinygltf::Accessor     normalAccessor;
                    tinygltf::BufferView normalBufferView;
                    const uint8_t * normalBufferAddress = nullptr;
                    int normalStride = -1;
                    if(normalIndex>-1){
                        normalAccessor                       = model.accessors[normalIndex];
                        normalBufferView                     = model.bufferViews[normalAccessor.bufferView];
                        const tinygltf::Buffer& normalBuffer = model.buffers[normalBufferView.buffer];
                        normalBufferAddress                  = normalBuffer.data.data();
                        normalStride                         = tinygltf::GetComponentSizeInBytes(normalAccessor.componentType) * tinygltf::GetNumComponentsInType(normalAccessor.type);
                        if(normalBufferView.byteStride>0)
                            normalStride = normalBufferView.byteStride;
                    }

                    //Texture coordinates
                    tinygltf::Accessor     uv0Accessor;
                    tinygltf::BufferView uv0BufferView;
                    const uint8_t* uv0BufferAddress = nullptr;
                    int uv0Stride = -1;
                    if(uv0Index>-1){
                        uv0Accessor                       = model.accessors[uv0Index];
                        uv0BufferView                     = model.bufferViews[uv0Accessor.bufferView];
                        const tinygltf::Buffer& uv0Buffer = model.buffers[uv0BufferView.buffer];
                        uv0BufferAddress                  = uv0Buffer.data.data();
                        uv0Stride                         = tinygltf::GetComponentSizeInBytes(uv0Accessor.componentType)*tinygltf::GetNumComponentsInType(uv0Accessor.type);
                        if(uv0BufferView.byteStride>0)
                            uv0Stride = uv0BufferView.byteStride;
                    }

                    std::vector<vec3> vertices;
                    std::vector<vec3> normals;
                    std::vector<vec2> uvs;

                    //get vertex data

                    for(size_t vertexIndex =0;vertexIndex<positionAccessor.count;vertexIndex++)
                    {
                        vec3 vertex,normal;
                        vec2 uv;

                        {
                            const uint8_t* address = positionBufferAddress + positionBufferView.byteOffset + positionAccessor.byteOffset + (vertexIndex * positionStride);
                            memcpy(&vertex, address, 12);
                        }

                        if (normalIndex > -1)
                        {
                            const uint8_t* address = normalBufferAddress + normalBufferView.byteOffset + normalAccessor.byteOffset + (vertexIndex * normalStride);
                            memcpy(&normal, address, 12);
                        }

                        if (uv0Index > -1)
                        {
                            const uint8_t* address = uv0BufferAddress + uv0BufferView.byteOffset + uv0Accessor.byteOffset + (vertexIndex * uv0Stride);
                            memcpy(&uv, address, 8);
                        }

                        vertices.push_back(vertex);
                        normals.push_back(normal);
                        uvs.push_back(uv);
                    }


                    // Get index data
                    std::vector<int> indices(indexAccessor.count);
                    const uint8_t* baseAddress = indexBufferAddress + indexBufferView.byteOffset + indexAccessor.byteOffset;
                    if (indexStride == 1)
                    {
                        std::vector<uint8_t> quarter;
                        quarter.resize(indexAccessor.count);

                        memcpy(quarter.data(), baseAddress, (indexAccessor.count * indexStride));

                        // Convert quarter precision indices to full precision
                        for (size_t i = 0; i < indexAccessor.count; i++)
                        {
                            indices[i] = quarter[i];
                        }
                    }
                    else if (indexStride == 2)
                    {
                        std::vector<uint16_t> half;
                        half.resize(indexAccessor.count);

                        memcpy(half.data(), baseAddress, (indexAccessor.count * indexStride));

                        // Convert half precision indices to full precision
                        for (size_t i = 0; i < indexAccessor.count; i++)
                        {
                            indices[i] = half[i];
                        }
                    }
                    else
                    {
                        memcpy(indices.data(), baseAddress, (indexAccessor.count * indexStride));
                    }

                    int offset = triangles.size();
                    triangles.resize(offset+indices.size()/3);

                    // Get triangles from vertex indices

                    for (int v = 0; v < indices.size(); v+=3)
                    {
                        Triangle tri;


                        triangles[offset+v/3].p1 = vertices[indices[v]];
                        triangles[offset+v/3].p2 = vertices[indices[v+1]];
                        triangles[offset+v/3].p3 = vertices[indices[v+2]];
                        triangles[offset+v/3].n1 = normals[indices[v]];
                        triangles[offset+v/3].n2 = normals[indices[v+1]];
                        triangles[offset+v/3].n3 = normals[indices[v+2]];
                        triangles[offset+v/3].uv1= uvs[indices[v]];
                        triangles[offset+v/3].uv2= uvs[indices[v+1]];
                        triangles[offset+v/3].uv3= uvs[indices[v+2]];
                        triangles[offset+v/3].materialId = prim.material;

                    }

                }
                int x =1;
            }
            //Skip points and lines
            return;


    }
        static inline void LoadMaterials(std::vector<Material>&materials,tinygltf::Model&model){
            for (size_t i=0;i<model.materials.size();i++){
                const tinygltf::Material gltfMaterial = model.materials[i];
                const tinygltf::PbrMetallicRoughness pbr = gltfMaterial.pbrMetallicRoughness;
                //Convert glTF material
                Material material;

                //Albedo
                material.baseColor = vec3((float)pbr.baseColorFactor[0], (float)pbr.baseColorFactor[1], (float)pbr.baseColorFactor[2]);
                if (pbr.baseColorTexture.index > -1)
                    material.baseColorTexId = pbr.baseColorTexture.index ;

                //Opacity
                material.opacity = (float)pbr.baseColorFactor[3];


                //Alpha
                material.alphaCutoff = static_cast<float >(gltfMaterial.alphaCutoff);
                if(strcmp(gltfMaterial.alphaMode.c_str(),"OPAQUE")==0) material.alphaMode     = AlphaMode::Opaque;
                else if(strcmp(gltfMaterial.alphaMode.c_str(),"BLEND")==0) material.alphaMode = AlphaMode::Blend;
                else if(strcmp(gltfMaterial.alphaMode.c_str(),"MASK")==0)  material.alphaMode = AlphaMode::Mask;


                //Roughness and Metallic
                material.roughness = sqrtf((float)pbr.roughnessFactor);
                material.metallic  = (float )pbr.metallicFactor;
                if(pbr.metallicRoughnessTexture.index>-1)
                    material.metallicRoughnessTexID = pbr.metallicRoughnessTexture.index ;

                //Normal Map
                material.normalmapTexID = gltfMaterial.normalTexture.index;

                //Emission
                material.emissive =vec3 ((float )gltfMaterial.emissiveFactor[0],(float)gltfMaterial.emissiveFactor[1],(float )gltfMaterial.emissiveFactor[2]);
                if(gltfMaterial.emissiveTexture.index>-1)
                    material.emissionmapTexID = gltfMaterial.emissiveTexture.index ;

                //KHR_materials_transmission
                if(gltfMaterial.extensions.find("KHR_materials_transmission")!=gltfMaterial.extensions.end()){
                    const auto& ext = gltfMaterial.extensions.find("KHR_materials_transmission")->second;
                    if(ext.Has("transmissionFactor"))
                        material.specTrans = (float)(ext.Get("transmissionFactor").Get<double>());
                }
                materials.push_back(material);

            }
        }
        static inline void LoadTextures(std::vector<Texture*>&textures,tinygltf::Model&model){
            for(size_t i=0;i<model.textures.size();i++){
                tinygltf::Texture& gltfTex = model.textures[i];
                tinygltf::Image  & image   = model.images[gltfTex.source];
                std::string texName        = gltfTex.name;
                if(strcmp(gltfTex.name.c_str(),"")==0)
                        texName=image.uri;
                Texture* texture = new Texture(texName,image.image.data(),image.width, image.height,image.component);
                textures.push_back(texture);
            }
        }
        static inline void encodeTextures(std::vector<Texture*>textures,std::vector<unsigned char>&textureMapsArray){
            int reqWidth  = 2048;
            int reqHeight = 2048;
            int texBytes  = reqHeight*reqWidth*4;
            textureMapsArray.resize(texBytes*textures.size());
#pragma omp parallel for
            for(int i=0;i<textures.size();i++){
                int texWidth  = textures[i]->width;
                int texHeight = textures[i]->height;

                if(texWidth!=reqWidth||texHeight!=reqHeight){
                    unsigned char* resizedTex = new unsigned char [texBytes];
                    stbir_resize_uint8(&textures[i]->texData[0],texWidth,texHeight,0,resizedTex,reqWidth,reqHeight,0,4);
                    std::copy(resizedTex,resizedTex+texBytes,&textureMapsArray[i*texBytes]);
                    delete[] resizedTex;
                }
                else
                    std::copy(textures[i]->texData.begin(),textures[i]->texData.end(),&textureMapsArray[i*texBytes]);
            }
        }
        inline static bool readGLTF(std::string filepath,std::vector<Triangle>&triangles,std::vector<Material>&materials,std::vector<unsigned char>&textureMapsArray){
            tinygltf::Model     Model;
            tinygltf::TinyGLTF loader;
            std::string           err;
            std::string          warn;

            printf("Loading GLTF%s\n",filepath.c_str());
            bool ret;

            ret = loader.LoadASCIIFromFile(&Model, &err, &warn, filepath);
            if (!ret)
            {
                printf("Unable to load file %s. Error: %s\n", filepath.c_str(), err.c_str());
                return false;
            }

            std::map<int,std::vector<Primitive>> meshPrimMap;


            std::vector<Texture*> textures;
            LoadMeshes(triangles,Model,meshPrimMap);
            LoadMaterials(materials,Model);
            LoadTextures(textures, Model);
            encodeTextures(textures,textureMapsArray);
            return true;
        }


};
struct GLTF_SceneLoader{
    struct Primitive{
        int primitiveId;
        int materialId;
    };
public:
    //https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html
    static inline void LoadMeshes(Scene* scene ,tinygltf::Model& model,std::map<int,std::vector<Primitive>>& meshPrimMap){
        for(int gltfMeshIdx = 0;gltfMeshIdx<model.meshes.size();gltfMeshIdx++){
            tinygltf::Mesh gltfMesh = model.meshes[gltfMeshIdx];
            for(int gltfPrimIdx =0;gltfPrimIdx<gltfMesh.primitives.size();gltfPrimIdx++)
            {
                tinygltf::Primitive prim = gltfMesh.primitives[gltfPrimIdx];

                //Skip points and lines
                if(prim.mode !=TINYGLTF_MODE_TRIANGLES)
                    continue;

                int indicesIndex  = prim.indices;
                int positionIndex = -1;
                int normalIndex   = -1;
                int uv0Index      = -1;

                //检查primitive 是否带有这些属性如果没有，通过map.count()检查
                if(prim.attributes.count("POSITION")>0)
                {
                    positionIndex = prim.attributes["POSITION"];
                }

                if (prim.attributes.count("NORMAL") > 0)
                {
                    normalIndex = prim.attributes["NORMAL"];
                }

                if (prim.attributes.count("TEXCOORD_0") > 0)
                {
                    uv0Index = prim.attributes["TEXCOORD_0"];
                }

                //Vertex positions
                //通过层层索引找到buffer，access 能索引到一个buffer view，bufferview 可以索引到一个buffer，不过具体怎么读bufferview内offset是多少还是在accessor内
                tinygltf::Accessor     positionAccessor = model.accessors[positionIndex];
                tinygltf::BufferView positionBufferView = model.bufferViews[positionAccessor.bufferView];
                const tinygltf::Buffer&  positionBuffer = model.buffers[positionBufferView.buffer];
                const uint8_t*    positionBufferAddress = positionBuffer.data.data();
                //Element size, in bytes, is (size in bytes of the 'componentType') * (number of components defined by 'type').
                int                      positionStride = tinygltf::GetComponentSizeInBytes(positionAccessor.componentType)*tinygltf::GetNumComponentsInType(positionAccessor.type);

                //Recheck
                if(positionBufferView.byteStride>0)
                    positionStride = positionBufferView.byteStride;

                //FIXME: Some GLTF files like TriangleWithoutIndices gltf have no indices
                //Vertex indice
                tinygltf::Accessor     indexAccessor = model.accessors[indicesIndex];
                tinygltf::BufferView indexBufferView = model.bufferViews[indexAccessor.bufferView];
                const tinygltf::Buffer&  indexBuffer = model.buffers[indexBufferView.buffer];
                const uint8_t*    indexBufferAddress = indexBuffer.data.data();
                int                      indexStride = tinygltf::GetComponentSizeInBytes(indexAccessor.componentType) * tinygltf::GetNumComponentsInType(indexAccessor.type);

                //Normals
                tinygltf::Accessor     normalAccessor;
                tinygltf::BufferView normalBufferView;
                const uint8_t * normalBufferAddress = nullptr;
                int normalStride = -1;
                if(normalIndex>-1){
                    normalAccessor                       = model.accessors[normalIndex];
                    normalBufferView                     = model.bufferViews[normalAccessor.bufferView];
                    const tinygltf::Buffer& normalBuffer = model.buffers[normalBufferView.buffer];
                    normalBufferAddress                  = normalBuffer.data.data();
                    normalStride                         = tinygltf::GetComponentSizeInBytes(normalAccessor.componentType) * tinygltf::GetNumComponentsInType(normalAccessor.type);
                    if(normalBufferView.byteStride>0)
                        normalStride = normalBufferView.byteStride;
                }

                //Texture coordinates
                tinygltf::Accessor     uv0Accessor;
                tinygltf::BufferView uv0BufferView;
                const uint8_t* uv0BufferAddress = nullptr;
                int uv0Stride = -1;
                if(uv0Index>-1){
                    uv0Accessor                       = model.accessors[uv0Index];
                    uv0BufferView                     = model.bufferViews[uv0Accessor.bufferView];
                    const tinygltf::Buffer& uv0Buffer = model.buffers[uv0BufferView.buffer];
                    uv0BufferAddress                  = uv0Buffer.data.data();
                    uv0Stride                         = tinygltf::GetComponentSizeInBytes(uv0Accessor.componentType)*tinygltf::GetNumComponentsInType(uv0Accessor.type);
                    if(uv0BufferView.byteStride>0)
                        uv0Stride = uv0BufferView.byteStride;
                }

                std::vector<vec3> vertices;
                std::vector<vec3> normals;
                std::vector<vec2> uvs;

                //get vertex data

                for(size_t vertexIndex =0;vertexIndex<positionAccessor.count;vertexIndex++)
                {
                    vec3 vertex,normal;
                    vec2 uv;

                    {
                        const uint8_t* address = positionBufferAddress + positionBufferView.byteOffset + positionAccessor.byteOffset + (vertexIndex * positionStride);
                        memcpy(&vertex, address, 12);
                    }

                    if (normalIndex > -1)
                    {
                        const uint8_t* address = normalBufferAddress + normalBufferView.byteOffset + normalAccessor.byteOffset + (vertexIndex * normalStride);
                        memcpy(&normal, address, 12);
                    }

                    if (uv0Index > -1)
                    {
                        const uint8_t* address = uv0BufferAddress + uv0BufferView.byteOffset + uv0Accessor.byteOffset + (vertexIndex * uv0Stride);
                        memcpy(&uv, address, 8);
                    }

                    vertices.push_back(vertex);
                    normals.push_back(normal);
                    uvs.push_back(uv);
                }


                // Get index data
                std::vector<int> indices(indexAccessor.count);
                const uint8_t* baseAddress = indexBufferAddress + indexBufferView.byteOffset + indexAccessor.byteOffset;
                if (indexStride == 1)
                {
                    std::vector<uint8_t> quarter;
                    quarter.resize(indexAccessor.count);

                    memcpy(quarter.data(), baseAddress, (indexAccessor.count * indexStride));

                    // Convert quarter precision indices to full precision
                    for (size_t i = 0; i < indexAccessor.count; i++)
                    {
                        indices[i] = quarter[i];
                    }
                }
                else if (indexStride == 2)
                {
                    std::vector<uint16_t> half;
                    half.resize(indexAccessor.count);

                    memcpy(half.data(), baseAddress, (indexAccessor.count * indexStride));

                    // Convert half precision indices to full precision
                    for (size_t i = 0; i < indexAccessor.count; i++)
                    {
                        indices[i] = half[i];
                    }
                }
                else
                {
                    memcpy(indices.data(), baseAddress, (indexAccessor.count * indexStride));
                }

                int offset = scene->triangles.size();
                scene->triangles.resize(offset+indices.size()/3);

                // Get triangles from vertex indices
                Mesh* mesh = new Mesh();

                for (int v = 0; v < indices.size(); v+=3)
                {
                    Triangle tri;

                    mesh->vertices.push_back(vertices[indices[v]]);
                    mesh->vertices.push_back(vertices[indices[v+1]]);
                    mesh->vertices.push_back(vertices[indices[v+2]]);
                    mesh->normals.push_back(normals[indices[v]]);
                    mesh->normals.push_back(normals[indices[v+1]]);
                    mesh->normals.push_back(normals[indices[v+2]]);
                    mesh->uvs.push_back(uvs[indices[v]]);
                    mesh->uvs.push_back(uvs[indices[v+1]]);
                    mesh->uvs.push_back(uvs[indices[v+2]]);

                    scene->triangles[offset+v/3].p1 = vertices[indices[v]];
                    scene->triangles[offset+v/3].p2 = vertices[indices[v+1]];
                    scene->triangles[offset+v/3].p3 = vertices[indices[v+2]];
                    scene->triangles[offset+v/3].n1 = normals[indices[v]];
                    scene->triangles[offset+v/3].n2 = normals[indices[v+1]];
                    scene->triangles[offset+v/3].n3 = normals[indices[v+2]];
                    scene->triangles[offset+v/3].uv1= uvs[indices[v]];
                    scene->triangles[offset+v/3].uv2= uvs[indices[v+1]];
                    scene->triangles[offset+v/3].uv3= uvs[indices[v+2]];
                    scene->triangles[offset+v/3].materialId = prim.material;

                }
                mesh->name = gltfMesh.name;
                int sceneMeshId = scene->meshes.size();
                scene->meshes.push_back(mesh);
                int sceneMatIdx = prim.material+scene->materials.size();
                meshPrimMap[gltfMeshIdx].push_back(Primitive{sceneMeshId,sceneMatIdx});

            }
            int x =1;
        }
        //Skip points and lines
        return;


    }
    static inline void LoadMaterials(Scene* scene,tinygltf::Model&model){
        for (size_t i=0;i<model.materials.size();i++){
            const tinygltf::Material gltfMaterial = model.materials[i];
            const tinygltf::PbrMetallicRoughness pbr = gltfMaterial.pbrMetallicRoughness;
            //Convert glTF material
            Material material;

            //Albedo
            material.baseColor = vec3((float)pbr.baseColorFactor[0], (float)pbr.baseColorFactor[1], (float)pbr.baseColorFactor[2]);
            if (pbr.baseColorTexture.index > -1)
                material.baseColorTexId = pbr.baseColorTexture.index ;

            //Opacity
            material.opacity = (float)pbr.baseColorFactor[3];


            //Alpha
            material.alphaCutoff = static_cast<float >(gltfMaterial.alphaCutoff);
            if(strcmp(gltfMaterial.alphaMode.c_str(),"OPAQUE")==0) material.alphaMode     = AlphaMode::Opaque;
            else if(strcmp(gltfMaterial.alphaMode.c_str(),"BLEND")==0) material.alphaMode = AlphaMode::Blend;
            else if(strcmp(gltfMaterial.alphaMode.c_str(),"MASK")==0)  material.alphaMode = AlphaMode::Mask;


            //Roughness and Metallic
            material.roughness = sqrtf((float)pbr.roughnessFactor);
            material.metallic  = (float )pbr.metallicFactor;
            if(pbr.metallicRoughnessTexture.index>-1)
                material.metallicRoughnessTexID = pbr.metallicRoughnessTexture.index ;

            //Normal Map
            material.normalmapTexID = gltfMaterial.normalTexture.index;

            //Emission
            material.emissive =vec3 ((float )gltfMaterial.emissiveFactor[0],(float)gltfMaterial.emissiveFactor[1],(float )gltfMaterial.emissiveFactor[2]);
            if(gltfMaterial.emissiveTexture.index>-1)
                material.emissionmapTexID = gltfMaterial.emissiveTexture.index ;

            //KHR_materials_transmission
            if(gltfMaterial.extensions.find("KHR_materials_transmission")!=gltfMaterial.extensions.end()){
                const auto& ext = gltfMaterial.extensions.find("KHR_materials_transmission")->second;
                if(ext.Has("transmissionFactor"))
                    material.specTrans = (float)(ext.Get("transmissionFactor").Get<double>());
            }
            scene->materials.push_back(material);

        }
    }
    static inline void LoadTextures(Scene* scene,tinygltf::Model&model){
        for(size_t i=0;i<model.textures.size();i++){
            tinygltf::Texture& gltfTex = model.textures[i];
            tinygltf::Image  & image   = model.images[gltfTex.source];
            std::string texName        = gltfTex.name;
            if(strcmp(gltfTex.name.c_str(),"")==0)
                texName=image.uri;
            Texture* texture = new Texture(texName,image.image.data(),image.width, image.height,image.component);
            scene->textures.push_back(texture);
        }
        int reqWidth  = 2048;
        int reqHeight = 2048;
        int texBytes  = reqHeight*reqWidth*4;
        scene->textureMapsArray.resize(texBytes*scene->textures.size());
#pragma omp parallel for
        for(int i=0;i<scene->textures.size();i++){
            int texWidth  = scene->textures[i]->width;
            int texHeight = scene->textures[i]->height;

            if(texWidth!=reqWidth||texHeight!=reqHeight){
                unsigned char* resizedTex = new unsigned char [texBytes];
                stbir_resize_uint8(&scene->textures[i]->texData[0],texWidth,texHeight,0,resizedTex,reqWidth,reqHeight,0,4);
                std::copy(resizedTex,resizedTex+texBytes,&scene->textureMapsArray[i*texBytes]);
                delete[] resizedTex;
            }
            else
                std::copy(scene->textures[i]->texData.begin(),scene->textures[i]->texData.end(),&scene->textureMapsArray[i*texBytes]);
        }
    }
    static void TraverseNodes(Scene* scene, tinygltf::Model& gltfModel, int nodeIdx, mat4& parentMat, std::map<int, std::vector<Primitive>>& meshPrimMap)
    {
        tinygltf::Node gltfNode = gltfModel.nodes[nodeIdx];

        mat4 localMat=mat4(1.0f);

        if (gltfNode.matrix.size() > 0)
        {
            localMat[0][0] = gltfNode.matrix[0];
            localMat[0][1] = gltfNode.matrix[1];
            localMat[0][2] = gltfNode.matrix[2];
            localMat[0][3] = gltfNode.matrix[3];

            localMat[1][0] = gltfNode.matrix[4];
            localMat[1][1] = gltfNode.matrix[5];
            localMat[1][2] = gltfNode.matrix[6];
            localMat[1][3] = gltfNode.matrix[7];

            localMat[2][0] = gltfNode.matrix[8];
            localMat[2][1] = gltfNode.matrix[9];
            localMat[2][2] = gltfNode.matrix[10];
            localMat[2][3] = gltfNode.matrix[11];

            localMat[3][0] = gltfNode.matrix[12];
            localMat[3][1] = gltfNode.matrix[13];
            localMat[3][2] = gltfNode.matrix[14];
            localMat[3][3] = gltfNode.matrix[15];
        }
        else
        {
            mat4 translate = mat4 (1.0f);
            mat4 rot       = mat4 (1.0f);
            mat4 scale     = mat4 (1.0f);

            if (gltfNode.translation.size() > 0)
            {
                translate[3][0] = gltfNode.translation[0];
                translate[3][1] = gltfNode.translation[1];
                translate[3][2] = gltfNode.translation[2];
            }

            if (gltfNode.rotation.size() > 0)
            {
                glm::quat glmquat = glm::quat (gltfNode.rotation[0], gltfNode.rotation[1], gltfNode.rotation[2], gltfNode.rotation[3]);
                rot = glm::mat4_cast(glmquat);
            }

            if (gltfNode.scale.size() > 0)
            {
                scale[0][0] = gltfNode.scale[0];
                scale[1][1] = gltfNode.scale[1];
                scale[2][2] = gltfNode.scale[2];
            }

            localMat = translate*rot*scale;
        }

        mat4 xform = parentMat* localMat;

        // When at a leaf node, add an instance to the scene (if a mesh exists for it)
        if (gltfNode.children.size() == 0 && gltfNode.mesh != -1)
        {
            std::vector<Primitive> prims = meshPrimMap[gltfNode.mesh];

            // Write the instance data
            for (int i = 0; i < prims.size(); i++)
            {
                std::string name = gltfNode.name;
                // TODO: Better naming

                name = "Mesh " + std::to_string(gltfNode.mesh) + " Prim" + std::to_string(prims[i].primitiveId);

                MeshInstance instance(name, prims[i].primitiveId, xform, prims[i].materialId < 0 ? 0 : prims[i].materialId);
                scene->AddMeshInstance(instance);
            }
        }

        for (size_t i = 0; i < gltfNode.children.size(); i++)
        {
            TraverseNodes(scene, gltfModel, gltfNode.children[i], xform, meshPrimMap);
        }
    }
    static void LoadInstances(Scene* scene, tinygltf::Model& gltfModel, mat4 xform, std::map<int, std::vector<Primitive>>& meshPrimMap)
    {
        const tinygltf::Scene gltfScene = gltfModel.scenes[gltfModel.defaultScene];

        for (int rootIdx = 0; rootIdx < gltfScene.nodes.size(); rootIdx++)
        {
            TraverseNodes(scene,gltfModel,gltfScene.nodes[rootIdx],xform,meshPrimMap);
        }
    }
    static bool readGLTF(Scene*& scene,std::string filepath){
        delete scene;

        scene = new Scene();
        tinygltf::Model     Model;
        tinygltf::TinyGLTF loader;
        std::string           err;
        std::string          warn;

        printf("Loading GLTF%s\n",filepath.c_str());
        bool ret;

        ret = loader.LoadASCIIFromFile(&Model, &err, &warn, filepath);
        if (!ret)
        {
            printf("Unable to load file %s. Error: %s\n", filepath.c_str(), err.c_str());
            return false;
        }

        std::map<int,std::vector<Primitive>> meshPrimMap;

        mat4 xform;

        LoadMeshes(scene,Model,meshPrimMap);
        LoadMaterials(scene,Model);
        LoadTextures(scene, Model);
        LoadInstances(scene,Model,xform,meshPrimMap);
        return true;
    }


};
struct ObjLoader{
    public:
        inline static void readObj(std::string filepath, std::vector<Triangle>& triangles, std::vector<Material>&materials, mat4 trans, bool smoothNormal) {
// 顶点位置，索引
            std::vector<vec3> vertices;
            std::vector<GLuint> indices;
// 打开文件流
            std::ifstream fin(filepath);
            std::string line;
            if (!fin.is_open()) {
                std::cout << "文件 " << filepath << " 打开失败" << std::endl;
                exit(-1);
            }
// 计算 AABB 盒，归一化模型大小
            float maxx = -11451419.19;
            float maxy = -11451419.19;
            float maxz = -11451419.19;
            float minx = 11451419.19;
            float miny = 11451419.19;
            float minz = 11451419.19;
// 按行读取
            while (std::getline(fin, line)) {
                std::istringstream sin(line); // 以一行的数据作为 string stream 解析并且读取
                std::string type;
                GLfloat x, y, z;
                int v0, v1, v2;
                int vn0, vn1, vn2;
                int vt0, vt1, vt2;
                char slash;
// 统计斜杆数目，用不同格式读取
                int slashCnt = 0;
                for (int i = 0; i < line.length(); i++) {
                    if (line[i] == '/') slashCnt++;
                }
// 读取obj文件
                sin >> type;
                if (type == "v") {
                    sin >> x >> y >> z;
                    vertices.push_back(vec3(x, y, z));
                    maxx = max(maxx, x); maxy = max(maxx, y); maxz = max(maxx, z);
                    minx = min(minx, x); miny = min(minx, y); minz = min(minx, z);
                }
                if (type == "f") {
                    if (slashCnt == 6) {
                        sin >> v0 >> slash >> vt0 >> slash >> vn0;
                        sin >> v1 >> slash >> vt1 >> slash >> vn1;
                        sin >> v2 >> slash >> vt2 >> slash >> vn2;
                    }
                    else if (slashCnt == 3) {
                        sin >> v0 >> slash >> vt0;
                        sin >> v1 >> slash >> vt1;
                        sin >> v2 >> slash >> vt2;
                    }
                    else {
                        sin >> v0 >> v1 >> v2;
                    }
                    indices.push_back(v0 - 1);
                    indices.push_back(v1 - 1);
                    indices.push_back(v2 - 1);
                }
            }
// 模型大小归一化
            float lenx = maxx - minx;
            float leny = maxy - miny;
            float lenz = maxz - minz;
            float maxaxis = max(lenx, max(leny, lenz));
            for (auto& v : vertices) {
                v.x /= maxaxis;
                v.y /= maxaxis;
                v.z /= maxaxis;
            }
// 通过矩阵进行坐标变换
            for (auto& v : vertices) {
                vec4 vv = vec4(v.x, v.y, v.z, 1);
                vv = trans * vv;
                v = vec3(vv.x, vv.y, vv.z);
            }
// 生成法线
            std::vector<vec3> normals(vertices.size(), vec3(0, 0, 0));
            for (int i = 0; i < indices.size(); i += 3) {
                vec3 p1 = vertices[indices[i]];
                vec3 p2 = vertices[indices[i + 1]];
                vec3 p3 = vertices[indices[i + 2]];
                vec3 n = normalize(cross(p2 - p1, p3 - p1));
                normals[indices[i]] += n;
                normals[indices[i + 1]] += n;
                normals[indices[i + 2]] += n;
            }
// 构建 Triangle 对象数组
            int offset = triangles.size(); // 增量更新
            triangles.resize(offset + indices.size() / 3);
            for (int i = 0; i < indices.size(); i += 3) {
                Triangle& t = triangles[offset + i / 3];
// 传顶点属性
                t.p1 = vertices[indices[i]];
                t.p2 = vertices[indices[i + 1]];
                t.p3 = vertices[indices[i + 2]];
                if (!smoothNormal) {
                    vec3 n = normalize(cross(t.p2 - t.p1, t.p3 - t.p1));
                    t.n1 = n; t.n2 = n; t.n3 = n;
                }
                else {
                    t.n1 = normalize(normals[indices[i]]);
                    t.n2 = normalize(normals[indices[i + 1]]);
                    t.n3 = normalize(normals[indices[i + 2]]);
                }
// 传材质
                t.materialId = materials.size()-1;
            }
        }

};
struct TextureLoader{
public:
    inline static GLuint getTextureRGB32F(int width, int height) {
        //width 宽，height 高
        //生成一个纹理对象，实际的创建内存了，然后给了一堆这个纹理的参数

        GLuint tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA,
                     GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        return tex;
    }

};

//GltfLoader 用于
//bufferview 用于对buffer分块，确定每个buffer块在一个buffer中的起始位置和大小，以及对应的buffer_index
//每个buffer_view对应一个buffer块
//accessor 用于确定buffer_view中的数据类型，数据大小，数据个数，以及数据在buffer_view中的偏移,是对buffer块中数据的描述
//accessor和buffer view 是1对1或多对一的关系
//accessor对象的byteOffset属性值需要能够整除componentType属性所指定的类型所占的字节大小。
//accessor对象的byteOffset属性值和bufferView对象的byteOffset属性值之和需要能够整除componentType属性所指定的类型所占的字节大小。
//sparse access 定义了要替换的数据的数量，以及要替换的数据的索引和值
//这些索引和值会被存到不同的bufferview之中
//mesh 的 attribute 属性中的position & index 的值是accessors的索引
//动画系统
//sampler 负责采集和插值，所以他要对应一个input和一个output，从这两者的对应数值中根据插值方式采样
//channel 就像是一根管道，一头是sampler 一头是node，还要具体到node的某个属性，sampler把采集到的数据通过channel 传送给node
//primitive 可以使用不同的模式显示指定几何数据描述的对象

//images 用于存储图片的uri，以及图片的格式
//texture 用于存储纹理的sampler和source，用source 指定使用的sampler
//纹理的sampler 和 动画的sampler 是不一样的
//Material 通过 index 引用texture 纹理
//所有texture 默认使用mesh-primitive 中定义的纹理，一般是 TEXCOORD_0
//如果纹理想要单独指定纹理坐标，可以在material basecolorTexture中指定特定的纹理坐标
#endif //RT_3_LOADER_H
