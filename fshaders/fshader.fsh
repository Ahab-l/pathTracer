#version 430 core
in vec3 pix;
out vec4 fragColor;

// ----------------------------------------------------------------------------- //

uniform uint frameCounter;
uniform int nTriangles;
uniform int nNodes;
uniform int width;
uniform int height;

uniform sampler2DArray textureMapsArrayTex;
uniform samplerBuffer triangles;
uniform samplerBuffer nodes;
uniform samplerBuffer materials;

uniform sampler2D lastFrame;
uniform sampler2D hdrMap;



uniform vec3 eye;
uniform mat4 cameraRotate;

// ----------------------------------------------------------------------------- //
#define PI              3.1415926
#define INF             114514.0
#define SIZE_TRIANGLE   10
#define SIZE_BVHNODE    4
#define SIZE_Material   11
#define INF 114514.0

struct Triangle{
    vec3 n1,n2,n3;
    vec3 p1,p2,p3;
    vec2 uv1,uv2,uv3;
    int  materialId;
    float padding1;
    float padding2;
    vec3 padding3;
};
struct Ray {
    vec3 startPoint;
    vec3 direction;
};
// BVH 树节点
struct BVHNode {
    int left; // 左子树
    int right; // 右子树
    int n; // 包含三角形数目
    int index; // 三角形索引
    vec3 AA, BB; // 碰撞盒
};

struct Material {
    vec3 emissive;          // 作为光源时的发光颜色
    vec3 baseColor;

    float subsurface;
    float metallic;
    float specular;

    float specularTint;
    float roughness;
    float anisotropic;

    float sheen;
    float sheenTint;
    float clearcoat;

    float clearcoatGloss;
    float IOR;
    float transmission;


    float specTrans;
    float mediumType;
    float mediumDensity;

    vec3 mediumColor;

    float mediumAnisortropy;
    int baseColorTexId;
    int metallicRoughnessTexID;

    int normalmapTexID;
    int emissionmapTexID;
    float opacity;

    float alphaMode;
    float alphaCutoff;
    float padding;
};
struct HitResult{
    bool     isHit;
    bool     isInside;
    float    distance;
    vec3     hitPoint;
    vec3     normal;
    vec3     viewDir;
    vec3     baycentric_coord;
    vec3     tangent;
    vec3     bitangent;
    vec2     uv_coord;
    float    alpha;
    float    beta;
    float    gamma;

    Material material;
};
uint seed = uint(
uint((pix.x * 0.5 + 0.5) * width) * uint(1973) +
uint((pix.y * 0.5 + 0.5) * height) * uint(9277) +
uint(frameCounter) * uint(26699)) | uint(1);
uint wang_hash(inout uint seed) {
    seed = uint(seed ^ uint(61)) ^ uint(seed >> uint(16));
    seed *= uint(9);
    seed = seed ^ (seed >> 4);
    seed *= uint(0x27d4eb2d);
    seed = seed ^ (seed >> 15);
    return seed;
}

float rand(){
    return float(wang_hash(seed))/4294967296.0;
}
//留个空 如果使用与cosθ相关的采样方式，兰伯特模型中就应该不用除以cosθ了，
//因为由入射和法线相乘的贡献项为cosθ，均匀采样概率为处处相等，积分为1，
//使用概率为cos正相关的概率分布与其结果一致，因此我们直接可以使用把贡献的比值转换为概率分布的比值，在结果上没有变化的。
//单位球内均匀采样
vec3 randomVec3(){
    vec3 d;
    do{
        d = 2.f*vec3(rand(),rand(),rand())-vec3(1.f,1.f,1.f);
    }while (dot(d,d)>1.0);
    return normalize(d);
}
//sobel 生成矩阵
const uint V[256] = uint[](
2147483648u,1073741824u,536870912u,268435456u,134217728u,67108864u,33554432u,16777216u,8388608u,4194304u,2097152u,1048576u,524288u,262144u,131072u,65536u,32768u,16384u,8192u,4096u,2048u,1024u,512u,256u,128u,64u,32u,16u,8u,4u,2u,1u,2147483648u,3221225472u,2684354560u,4026531840u,2281701376u,3422552064u,2852126720u,4278190080u,2155872256u,3233808384u,2694840320u,4042260480u,2290614272u,3435921408u,2863267840u,4294901760u,2147516416u,3221274624u,2684395520u,4026593280u,2281736192u,3422604288u,2852170240u,4278255360u,2155905152u,3233857728u,2694881440u,4042322160u,2290649224u,3435973836u,2863311530u,4294967295u,2147483648u,3221225472u,1610612736u,2415919104u,3892314112u,1543503872u,2382364672u,3305111552u,1753219072u,2629828608u,3999268864u,1435500544u,2154299392u,3231449088u,1626210304u,2421489664u,3900735488u,1556135936u,2388680704u,3314585600u,1751705600u,2627492864u,4008611328u,1431684352u,2147543168u,3221249216u,1610649184u,2415969680u,3892340840u,1543543964u,2382425838u,3305133397u,2147483648u,3221225472u,536870912u,1342177280u,4160749568u,1946157056u,2717908992u,2466250752u,3632267264u,624951296u,1507852288u,3872391168u,2013790208u,3020685312u,2181169152u,3271884800u,546275328u,1363623936u,4226424832u,1977167872u,2693105664u,2437829632u,3689389568u,635137280u,1484783744u,3846176960u,2044723232u,3067084880u,2148008184u,3222012020u,537002146u,1342505107u,2147483648u,1073741824u,536870912u,2952790016u,4160749568u,3690987520u,2046820352u,2634022912u,1518338048u,801112064u,2707423232u,4038066176u,3666345984u,1875116032u,2170683392u,1085997056u,579305472u,3016343552u,4217741312u,3719483392u,2013407232u,2617981952u,1510979072u,755882752u,2726789248u,4090085440u,3680870432u,1840435376u,2147625208u,1074478300u,537900666u,2953698205u,2147483648u,1073741824u,1610612736u,805306368u,2818572288u,335544320u,2113929216u,3472883712u,2290089984u,3829399552u,3059744768u,1127219200u,3089629184u,4199809024u,3567124480u,1891565568u,394297344u,3988799488u,920674304u,4193267712u,2950604800u,3977188352u,3250028032u,129093376u,2231568512u,2963678272u,4281226848u,432124720u,803643432u,1633613396u,2672665246u,3170194367u,2147483648u,3221225472u,2684354560u,3489660928u,1476395008u,2483027968u,1040187392u,3808428032u,3196059648u,599785472u,505413632u,4077912064u,1182269440u,1736704000u,2017853440u,2221342720u,3329785856u,2810494976u,3628507136u,1416089600u,2658719744u,864310272u,3863387648u,3076993792u,553150080u,272922560u,4167467040u,1148698640u,1719673080u,2009075780u,2149644390u,3222291575u,2147483648u,1073741824u,2684354560u,1342177280u,2281701376u,1946157056u,436207616u,2566914048u,2625634304u,3208642560u,2720006144u,2098200576u,111673344u,2354315264u,3464626176u,4027383808u,2886631424u,3770826752u,1691164672u,3357462528u,1993345024u,3752330240u,873073152u,2870150400u,1700563072u,87021376u,1097028000u,1222351248u,1560027592u,2977959924u,23268898u,437609937u
);
float myArray[5] = float[](1.0, 2.0, 3.0, 4.0, 5.0);
// 格林码
uint grayCode(uint i) {
    return i ^ (i>>1);
}
// 生成第 d 维度的第 i 个 sobol 数
float sobol(uint d, uint i) {
    uint result = uint(0);
    uint offset = d * uint(32);
    for(uint j = uint(0); i!=uint(0); i >>= uint(1), j++)
    if((i & uint(1))!=uint(0))
    result ^= V[j+offset];

    return float(result) * (1.0f/float(0xFFFFFFFFU));
}
// 生成第 i 帧的第 b 次反弹需要的二维随机向量
vec2 sobolVec2(uint i, uint b) {
    float u = sobol(b*uint(2), grayCode(i));
    float v = sobol(b*uint(2)+uint(1), grayCode(i));
    return vec2(u, v);
}
vec2 CranleyPattersonRotation(vec2 p) {
    uint pseed = uint(
    uint((pix.x * 0.5 + 0.5) * width)  * uint(1973) +
    uint((pix.y * 0.5 + 0.5) * height) * uint(9277) +
    uint(114514/1919) * uint(26699)) | uint(1);

    float u = float(wang_hash(pseed)) / 4294967296.0;
    float v = float(wang_hash(pseed)) / 4294967296.0;

    p.x += u;
    if(p.x>1) p.x -= 1;
    if(p.x<0) p.x += 1;

    p.y += v;
    if(p.y>1) p.y -= 1;
    if(p.y<0) p.y += 1;

    return p;
}
// 半球均匀采样
vec3 SampleHemisphere() {
    float z = rand();
    float r = max(0, sqrt(1.0 - z*z));
    float phi = 2.0 * PI * rand();
    return vec3(r * cos(phi), r * sin(phi), z);
}
//半球均匀采样 索贝尔序列
vec3 SampleHemisphere(float xi_1,float xi_2){
    float z = xi_1;
    float r = max(0,sqrt(1.0-z*z));
    float phi = 2.0 * PI *xi_2;
    return vec3(r*cos(phi),r*sin(phi),z);
}
//与法线接近的半球采样
vec3 SampleHemisphere_Normal(vec3 n){
    return normalize((randomVec3()+n));
}
//获取hdr环境颜色

// 将向量 v 投影到 N 的法向半球
vec3 toNormalHemisphere(vec3 v, vec3 N) {
    vec3 helper = vec3(1, 0, 0);
    if(abs(N.x)>0.999) helper = vec3(0, 0, 1);
    vec3 tangent = normalize(cross(N, helper));
    vec3 bitangent = normalize(cross(N, tangent));
    return v.x * tangent + v.y * bitangent + v.z * N;
}
// 余弦加权的法向半球采样
vec3 SampleCosineHemisphere(float xi_1, float xi_2, vec3 N) {
    // 均匀采样 xy 圆盘然后投影到 z 半球
    float r = sqrt(xi_1);
    float theta = xi_2 * 2.0 * PI;
    float x = r * cos(theta);
    float y = r * sin(theta);
    float z = sqrt(1.0 - x*x - y*y);
    // 从 z 半球投影到法向半球
    vec3 L = toNormalHemisphere(vec3(x, y, z), N);
    return L;
}
// 获取第 i 下标的三角形
Triangle getTriangle(int i) {
    int offset = i * SIZE_TRIANGLE;
    Triangle t;
    // 顶点坐标
    t.p1 = texelFetch(triangles, offset + 0).xyz;
    t.p2 = texelFetch(triangles, offset + 1).xyz;
    t.p3 = texelFetch(triangles, offset + 2).xyz;
    // 法线
    t.n1 = texelFetch(triangles, offset + 3).xyz;
    t.n2 = texelFetch(triangles, offset + 4).xyz;
    t.n3 = texelFetch(triangles, offset + 5).xyz;
    //
    t.uv1   = texelFetch(triangles,offset  + 6).xy;
    t.uv2.x = texelFetch(triangles,offset  + 6).z;
    t.uv2.y = texelFetch(triangles,offset  + 7).x;
    t.uv3   = texelFetch(triangles,offset  + 7).yz;

    t.materialId = int(texelFetch(triangles,offset  + 8).x);
    t.padding1   = texelFetch(triangles,offset  + 8).y;
    t.padding2   = texelFetch(triangles,offset  + 8).z;
    t.padding3   = texelFetch(triangles,offset  + 9).xyz;


    return t;
}
//  获取第i 个下标的BVHNode 对象
BVHNode getBVHNode(int i){
    BVHNode node;

    //左右子树
    int offset     = i * SIZE_BVHNODE;
    ivec3 childs   = ivec3(texelFetch(nodes,offset+0).xyz);
    ivec3 leafInfo = ivec3(texelFetch(nodes,offset+1).xyz);
    node.left      = int(childs.x);  //左BVH节点的下标
    node.right     = int(childs.y);  //右BVH节点的下标
    node.n         = int(leafInfo.x);//该BVH节点中起始三角形的下标
    node.index     = int(leafInfo.y);//该BVH节点中三角形的个数

    //包围盒
    node.AA        = texelFetch(nodes,offset+2).xyz;
    node.BB        = texelFetch(nodes,offset+3).xyz;

    return node;
}
// 获取第 i 下标的三角形的材质
Material getMaterial_new(int i,inout HitResult res){
    Material m;
    int offset = i * SIZE_Material;
    vec3 param1 = texelFetch(materials, offset + 2).xyz;
    vec3 param2 = texelFetch(materials, offset + 3).xyz;
    vec3 param3 = texelFetch(materials, offset + 4).xyz;
    vec3 param4 = texelFetch(materials, offset + 5).xyz;
    vec3 param5 = texelFetch(materials, offset + 6).xyz;
    vec3 param6 = texelFetch(materials, offset + 7).xyz;
    vec3 param7 = texelFetch(materials, offset + 8).xyz;
    vec3 param8 = texelFetch(materials, offset + 9).xyz;
    vec3 param9 = texelFetch(materials, offset + 10).xyz;

    m.emissive = texelFetch(materials, offset + 0).xyz;
    m.baseColor = texelFetch(materials, offset + 1).xyz;

    m.subsurface             = param1.x;
    m.metallic               = param1.y;
    m.specular               = param1.z;

    m.specularTint           = param2.x;
    m.roughness              = param2.y;
    m.anisotropic            = param2.z;

    m.sheen                  = param3.x;
    m.sheenTint              = param3.y;
    m.clearcoat              = param3.z;

    m.clearcoatGloss         = param4.x;
    m.IOR                    = param4.y;
    m.transmission           = param4.z;

    m.specTrans              = param5.x;
    m.mediumType             = param5.y;
    m.mediumDensity          = param5.z;

    m.mediumColor            = param6.xyz;

    m.mediumAnisortropy      = param7.x;
    m.baseColorTexId         = int(param7.y);
    m.metallicRoughnessTexID = int(param7.z);

    m.normalmapTexID         = int(param8.x);
    m.emissionmapTexID       = int(param8.y);
    m.opacity                = param8.z;

    m.alphaMode              = param9.x;
    m.alphaCutoff            = param9.y;
    m.padding                = param9.z;

    if(m.baseColorTexId>=0)
    {
        vec4 col =texture(textureMapsArrayTex,vec3(res.uv_coord,m.baseColorTexId));
        //m.baseColor.rgb= res.baycentric_coord;
        //m.baseColor.rgb = col.rgb;
        //m.opacity *= col.a;
        //m.baseColor=texture(textureMapsArrayTex,vec3(0.5f,0.5f,0.0f)).xyz;
        m.baseColor.rgb*=pow(col.rgb,vec3(2.2));
        m.opacity*=col.a;
    }
    if(m.metallicRoughnessTexID>=0){
        vec2 matRgh = texture(textureMapsArrayTex,vec3(res.uv_coord,m.metallicRoughnessTexID)).bg;
        m.metallic  = matRgh.x;
        m.roughness =  max(matRgh.y, 0.001);
    }
    if(m.normalmapTexID>=0){
        vec3 texNormal = texture(textureMapsArrayTex,vec3(res.uv_coord,m.normalmapTexID)).rgb;
        texNormal = normalize(texNormal * 2.0 - 1.0);
        //#ifdef OPT_OPENGL_NORMALMAP
        //texNormal.y = 1.0 - texNormal.y;
        //#endif

        res.normal = normalize(res.tangent*texNormal.x+res.bitangent*texNormal.y+res.normal*texNormal.z);
        res.normal.x=clamp(res.normal.x,-1.0f,1.0f);
        res.normal.y=clamp(res.normal.y,-1.0f,1.0f);
        res.normal.z=clamp(res.normal.z,-1.0f,1.0f);
    }
    //if (m.normalmapTexID >= 0)
    //{
    //    vec3 texNormal = texture(textureMapsArrayTex, vec3(res.uv_coord, m.normalmapTexID)).rgb;
    //
    //    #ifdef OPT_OPENGL_NORMALMAP
    //    texNormal.y = 1.0 - texNormal.y;
    //    #endif
    //    texNormal = normalize(texNormal * 2.0 - 1.0);

    //    vec3 origNormal = res.normal;
    //    res.normal = texNormal;

    //}
    if (m.emissionmapTexID >= 0)
        m.emissive = pow(texture(textureMapsArrayTex, vec3(res.uv_coord, m.emissionmapTexID)).rgb, vec3(2.2));

    return m;
}
Material getMaterial(int i) {
    Material m;
    int offset = i * SIZE_TRIANGLE;
    vec3 param1 = texelFetch(triangles, offset + 8).xyz;
    vec3 param2 = texelFetch(triangles, offset + 9).xyz;
    vec3 param3 = texelFetch(triangles, offset + 10).xyz;
    vec3 param4 = texelFetch(triangles, offset + 11).xyz;
    vec3 param5 = texelFetch(triangles, offset + 12).xyz;
    vec3 param6 = texelFetch(triangles, offset + 13).xyz;
    vec3 param7 = texelFetch(triangles, offset + 14).xyz;
    vec3 param8 = texelFetch(triangles, offset + 15).xyz;
    vec3 param9 = texelFetch(triangles, offset + 16).xyz;

    m.emissive = texelFetch(triangles, offset + 6).xyz;
    m.baseColor = texelFetch(triangles, offset + 7).xyz;
    m.subsurface             = param1.x;
    m.metallic               = param1.y;
    m.specular               = param1.z;

    m.specularTint           = param2.x;
    m.roughness              = param2.y;
    m.anisotropic            = param2.z;

    m.sheen                  = param3.x;
    m.sheenTint              = param3.y;
    m.clearcoat              = param3.z;

    m.clearcoatGloss         = param4.x;
    m.IOR                    = param4.y;
    m.transmission           = param4.z;

    m.specTrans              = param5.x;
    m.mediumType             = param5.y;
    m.mediumDensity          = param5.z;

    m.mediumColor            = param6.xyz;

    m.mediumAnisortropy      = param7.x;
//    m.baseColorTexId         = param7.y;
//    m.metallicRoughnessTexID = param7.z;

    //m.normalmapTexID         = param8.x;
    //m.emissionmapTexID       = param8.y;
    m.opacity                = param8.z;

    m.alphaMode              = param9.x;
    m.alphaCutoff            = param9.y;
    m.padding                = param9.z;

    return m;
}

HitResult hitTriangle(Triangle triangle, Ray ray) {
    HitResult res;
    res.distance = INF;
    res.isHit = false;
    res.isInside = false;

    vec3 p1 = triangle.p1;
    vec3 p2 = triangle.p2;
    vec3 p3 = triangle.p3;

    vec2 uv1 =triangle.uv1;
    vec2 uv2 =triangle.uv2;
    vec2 uv3 =triangle.uv3;

    vec3 S  = ray.startPoint; // 射线起点
    vec3 d  = ray.direction; // 射线方向
    vec3 N  = normalize(cross(p2-p1, p3-p1)); // 法向量

    vec3 e0 = p2 - p1;
    vec3 e1 = p3 - p1;
    vec3 pv = cross(d,e1);
    float det = dot(e0,pv);

    vec3 tv = S-p1;
    vec3 qv = cross(tv,e0);

    vec4 uvt;
    uvt.x = dot(tv,pv);
    uvt.y = dot(d, qv);
    uvt.z = dot(e1, qv);
    uvt.xyz = uvt.xyz / det;
    uvt.w = 1.0 - uvt.x - uvt.y;

    // 从三角形背后（模型内部）击中
    if (dot(N, d) > 0.0f) {
        N = -N;
        res.isInside = true;
    }
    // 如果视线和三角形平行，视为没有击中，直接返回
    if (abs(dot(N, d)) < 0.00001f) return res;
    // 距离



    if (uvt.z < 0.0005f) return res; // 如果三角形在光线背面，或者三角形距离起点太近，避免三角形在面上来回弹射。
    // 交点计算
    vec3 P = S + d * uvt.z;
    // 判断交点是否在三角形中
    vec3 c1 = cross(p2 - p1, P - p1);
    vec3 c2 = cross(p3 - p2, P - p2);
    vec3 c3 = cross(p1 - p3, P - p3);
    //在之前我们使N总指向对cam的方向，因此在正对和背对情况下有乘积都大于0和都小于0的情况。
    bool r1 = (dot(c1, N) > 0 && dot(c2, N) > 0 && dot(c3, N) > 0);

    bool r2 = (dot(c1, N) < 0 && dot(c2, N) < 0 && dot(c3, N) < 0);
    // 命中，封装返回结果
    if (r1 || r2) {
        res.isHit = true;
        res.hitPoint = P;
        res.distance = uvt.z;
        res.normal   = N;
        res.viewDir  = d;
        // 根据交点位置插值顶点法线
        float alpha = (-(P.x-p2.x)*(p3.y-p2.y) + (P.y-p2.y)*(p3.x-p2.x)) / (-(p1.x-p2.x-0.00005)*(p3.y-p2.y+0.00005) + (p1.y-p2.y+0.00005)*(p3.x-p2.x+0.00005));
        float beta  = (-(P.x-p3.x)*(p1.y-p3.y) + (P.y-p3.y)*(p1.x-p3.x)) / (-(p2.x-p3.x-0.00005)*(p1.y-p3.y+0.00005) + (p2.y-p3.y+0.00005)*(p1.x-p3.x+0.00005));
        float gama  = 1.0 - alpha - beta;
        beta =uvt.x;
        gama  =uvt.y;
        alpha  = 1.0f-beta-gama;
        vec3 Nsmooth = alpha * triangle.n1 + beta * triangle.n2 + gama *triangle.n3;
        Nsmooth      = normalize(Nsmooth);
        res.normal   = (res.isInside) ? (-Nsmooth) : (Nsmooth);

        res.uv_coord=alpha*uv1+beta*uv2+gama*uv3;
        res.baycentric_coord.x=alpha;
        res.baycentric_coord.y=beta;
        res.baycentric_coord.z=gama;

        // Calculate tangent and bitangent
        vec3 deltaPos1 = p2 - p1;
        vec3 deltaPos2 = p3 - p1;

        vec2 deltaUV1  = uv2 - uv1;
        vec2 deltaUV2  = uv3 - uv1;

        float invdet   = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);

        res.tangent    = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * invdet;
        res.bitangent  = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * invdet;

        res.tangent    = normalize(res.tangent);
        res.bitangent  = normalize(res.bitangent);

    }
    return res;
}
float hitAABB(Ray r, vec3 AA, vec3 BB) {
    vec3 invdir = 1.0 / r.direction;
    vec3 f = (BB - r.startPoint) * invdir;
    vec3 n = (AA - r.startPoint) * invdir;
    vec3 tmax = max(f, n);
    vec3 tmin = min(f, n);
    float t1 = min(tmax.x, min(tmax.y, tmax.z));
    float t0 = max(tmin.x, max(tmin.y, tmin.z));
    return (t1 >= t0) ? ((t0 > 0.0) ? (t0) : (t1)) : (-1);
}
HitResult hitArray(Ray ray, int l, int r) {
    HitResult res;
    res.isHit = false;
    res.distance = INF;
    for(int i=l; i<=r; i++) {
        Triangle triangle = getTriangle(i);
        HitResult r = hitTriangle(triangle, ray);
        if(r.isHit && r.distance<res.distance) {
            res = r;
            res.material = getMaterial_new(triangle.materialId,res);
        }
    }
    return res;
}
HitResult hitBVH(Ray ray) {
    HitResult res;
    res.isHit = false;
    res.distance = INF;
    // 栈
    int stack[256];
    int sp = 0;

    stack[sp++] = 1;
    while(sp>0) {
        int top = stack[--sp];
        BVHNode node = getBVHNode(top);
        // 是叶子节点，遍历三角形，求最近交点
        if(node.n>0) {
            int L = node.index;
            int R = node.index + node.n - 1;
            HitResult r = hitArray(ray, L, R);
            if(r.isHit && r.distance<res.distance) res = r;
            continue;
        }
        // 和左右盒子 AABB 求交
        float d1 = INF; // 左盒子距离
        float d2 = INF; // 右盒子距离
        if(node.left>0) {
            BVHNode leftNode = getBVHNode(node.left);
            d1 = hitAABB(ray, leftNode.AA, leftNode.BB);
        }
        if(node.right>0) {
            BVHNode rightNode = getBVHNode(node.right);
            d2 = hitAABB(ray, rightNode.AA, rightNode.BB);
        }
        // 在最近的盒子中搜索
        if(d1>0 && d2>0) {
            if(d1<d2) { // d1<d2, 左边先
                stack[sp++] = node.right;
                stack[sp++] = node.left;
            } else { // d2<d1, 右边先
                stack[sp++] = node.left;
                stack[sp++] = node.right;
            }
        } else if(d1>0) { // 仅命中左边
            stack[sp++] = node.left;
        } else if(d2>0) { // 仅命中右边
            stack[sp++] = node.right;
        }
    }
    return res;
}
float GTR1(float NdotH, float a) {
    if (a >= 1) return 1/PI;
    float a2 = a*a;
    float t = 1 + (a2-1)*NdotH*NdotH;
    return (a2-1) / (PI*log(a2)*t);
}
float GTR2(float NdotH, float a) {
    float a2 = a*a;
    float t = 1 + (a2-1)*NdotH*NdotH;
    return a2 / (PI * t*t);
}
void getTangent(vec3 N, inout vec3 tangent, inout vec3 bitangent) {
    vec3 helper = vec3(1, 0, 0);
    if(abs(N.x)>0.999) helper = vec3(0, 0, 1);
    bitangent = normalize(cross(N, helper));
    tangent = normalize(cross(N, bitangent));
}
float sqr(float x) {
    return x*x;
}
float GTR2_aniso(float NdotH, float HdotX, float HdotY, float ax, float ay) {
    return 1 / (PI * ax*ay * sqr( sqr(HdotX/ax) + sqr(HdotY/ay) + NdotH*NdotH
    ));
}
float smithG_GGX_aniso(float NdotV, float VdotX, float VdotY, float ax, float ay) {
    return 1 / (NdotV + sqrt( sqr(VdotX*ax) + sqr(VdotY*ay) + sqr(NdotV) ));
}

float SchlickFresnel(float u) {
    float m = clamp(1-u, 0, 1);
    float m2 = m*m;
    return m2*m2*m; // pow(m,5)
}
float smithG_GGX(float NdotV, float alphaG) {
    float a = alphaG*alphaG;
    float b = NdotV*NdotV;
    return 1 / (NdotV + sqrt(a + b - a*b));
}

// 将三维向量 v 转为 HDR map 的纹理坐标 uv
vec2 SampleSphericalMap(vec3 v) {
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv /= vec2(2.0 * PI, PI);
    uv.x-=0.25f;
    uv.y += 0.5;
    uv.y = 1.0 - uv.y;
    return uv;
}
//hdr 采样
vec3 sampleHdr(vec3 v) {
    vec2 uv = SampleSphericalMap(normalize(v));
    vec3 color = texture2D(hdrMap, uv).rgb;
    color = min(color, vec3(10));
    return color;
}
vec3 hdrColor(vec3 L) {
    vec2 uv = SampleSphericalMap(normalize(L));
    vec3 color = texture2D(hdrMap, uv).rgb;
    return color;
}
// GTR2 重要性采样
vec3 SampleGTR2(float xi_1,float xi_2,vec3 V,vec3 N,float alpha){
    float phi_h = 2.0*PI*xi_1;
    float sin_phi_h = sin(phi_h);
    float cos_phi_h = cos(phi_h);

    float cos_theta_h = sqrt((1.0-xi_2)/(1.0+(alpha*alpha-1.0)*xi_2));
    float sin_theta_h = sqrt(max(0.0,1.0-cos_theta_h*cos_theta_h));

    //采样“微平面”的法向量 作为镜面反射的半角向量
    vec3 H = vec3(sin_theta_h*cos_phi_h,sin_theta_h*sin_phi_h,cos_theta_h);
    H      = toNormalHemisphere(H,N);//投影到真正的法向半球
    //根据“微法线”计算反射光方向
    vec3 L = reflect(-V,H);

    return L;
}
vec3 SampleGTR1(float xi_1,float xi_2,vec3 V,vec3 N,float alpha){
    float phi_h = 2.0*PI*xi_1;
    float sin_phi_h = sin(phi_h);
    float cos_phi_h = cos(phi_h);

    float cos_theta_h = sqrt((1.0-pow(alpha*alpha, 1.0-xi_2))/(1.0-alpha*alpha));
    float sin_theta_h = sqrt(max(0.0,1.0-cos_theta_h*cos_theta_h));

    //采样“微平面”的法向量 作为镜面反射的半角向量
    vec3 H = vec3(sin_theta_h*cos_phi_h,sin_theta_h*sin_phi_h,cos_theta_h);
    H      = toNormalHemisphere(H,N);//投影到真正的法向半球
    //根据“微法线”计算反射光方向
    vec3 L = reflect(-V,H);

    return L;
}
vec3 BRDF_Evaluate(vec3 V, vec3 N, vec3 L, in Material material) {
    float NdotL = dot(N, L);
    float NdotV = dot(N, V);
    if(NdotL < 0 || NdotV < 0) return vec3(0);

    vec3 H = normalize(L + V);
    float NdotH = dot(N, H);
    float LdotH = dot(L, H);

    // 各种颜色
    vec3 Cdlin = material.baseColor;
    float Cdlum = 0.3 * Cdlin.r + 0.6 * Cdlin.g  + 0.1 * Cdlin.b;
    vec3 Ctint = (Cdlum > 0) ? (Cdlin/Cdlum) : (vec3(1));
    vec3 Cspec = material.specular * mix(vec3(1), Ctint, material.specularTint);
    vec3 Cspec0 = mix(0.08*Cspec, Cdlin, material.metallic); // 0° 镜面反射颜色
    vec3 Csheen = mix(vec3(1), Ctint, material.sheenTint);   // 织物颜色

    // 漫反射
    float Fd90 = 0.5 + 2.0 * LdotH * LdotH * material.roughness;
    float FL = SchlickFresnel(NdotL);
    float FV = SchlickFresnel(NdotV);
    float Fd = mix(1.0, Fd90, FL) * mix(1.0, Fd90, FV);

    // 次表面散射
    float Fss90 = LdotH * LdotH * material.roughness;
    float Fss = mix(1.0, Fss90, FL) * mix(1.0, Fss90, FV);
    float ss = 1.25 * (Fss * (1.0 / (NdotL + NdotV) - 0.5) + 0.5);

    // 镜面反射 -- 各向同性
    float alpha = max(0.001, sqr(material.roughness));
    float Ds = GTR2(NdotH, alpha);
    float FH = SchlickFresnel(LdotH);
    vec3 Fs = mix(Cspec0, vec3(1), FH);
    float Gs = smithG_GGX(NdotL, material.roughness);
    Gs *= smithG_GGX(NdotV, material.roughness);

    // 清漆
    float Dr = GTR1(NdotH, mix(0.1, 0.001, material.clearcoatGloss));
    float Fr = mix(0.04, 1.0, FH);
    float Gr = smithG_GGX(NdotL, 0.25) * smithG_GGX(NdotV, 0.25);

    // sheen
    vec3 Fsheen = FH * material.sheen * Csheen;

    vec3 diffuse = (1.0/PI) * mix(Fd, ss, material.subsurface) * Cdlin + Fsheen;
    vec3 specular = Gs * Fs * Ds;
    vec3 clearcoat = vec3(0.25 * Gr * Fr * Dr * material.clearcoat);

    return diffuse * (1.0 - material.metallic) + specular + clearcoat;
}
vec3 BRDF_Evaluate_Disney(vec3 V,vec3 N,vec3 L,vec3 X,vec3 Y,in Material material){


    float NdotL = dot(N,L);
    float NdotV = dot(N,V);
    if(NdotL<0||NdotV<0)    return vec3(0);

    vec3 H = normalize(L+V);
    float NdotH = dot(N,H);
    float LdotH = dot(L,H);

    vec3  Cdlin = material.baseColor;
    float Cdlum = 0.3*Cdlin.r+0.6*Cdlin.g+0.1*Cdlin.b;
    vec3  Ctint = (Cdlum>0)?(Cdlin/Cdlum):(vec3(1));
    vec3  Cspec = material.specular * mix(vec3(1), Ctint, material.specularTint);
    vec3 Cspec0 = mix(0.08*Cspec, Cdlin, material.metallic); // 0° 镜面反射颜色

    // 镜面反射
    //float alpha = material.roughness * material.roughness;
    float aspect = sqrt(1.0 - material.anisotropic * 0.9);
    float ax = max(0.001, sqr(material.roughness)/aspect);
    float ay = max(0.001, sqr(material.roughness)*aspect);
    float Ds = GTR2_aniso(NdotH, dot(H, X), dot(H, Y), ax, ay);
    float FH = SchlickFresnel(LdotH);
    vec3 Fs = mix(Cspec0, vec3(1), FH);
    float Gs;
    Gs = smithG_GGX_aniso(NdotL, dot(L, X), dot(L, Y), ax, ay);
    Gs *= smithG_GGX_aniso(NdotV, dot(V, X), dot(V, Y), ax, ay);

    vec3 specular = Gs * Fs * Ds;

    // 漫反射
    float Fd90 = 0.5 + 2.0 * LdotH * LdotH * material.roughness;
    float FL = SchlickFresnel(NdotL);
    float FV = SchlickFresnel(NdotV);
    float Fd = mix(1.0, Fd90, FL) * mix(1.0, Fd90, FV);
    // 清漆
    float Dr = GTR1(NdotH, mix(0.1, 0.001, material.clearcoatGloss));
    float Fr = mix(0.04, 1.0, FH);
    float Gr = smithG_GGX(NdotL, 0.25) * smithG_GGX(NdotV, 0.25);
    vec3 clearcoat = vec3(0.25 * Gr * Fr * Dr * material.clearcoat);
    //织物
    vec3 Csheen = mix(vec3(1), Ctint, material.sheenTint); // 织物颜色
    vec3 Fsheen = FH * material.sheen * Csheen;

    //次表面散射
    float Fss90 = LdotH*LdotH*material.roughness;
    float Fss   = mix(1.0,Fss90,FL)*mix(1.0,Fss90,FV);
    float ss    = 1.25*(Fss*(1.0/(NdotL+NdotV)-0.5)+0.5);
    vec3 diffuse = mix(Fd,ss,material.subsurface) * Cdlin / (2*PI);
    return diffuse * (1.0 - material.metallic)+specular+clearcoat;

    return vec3(0.f);
}
//按照辐射度分布分别采样三种BRDF
vec3 SampleBRDF(float xi_1,float xi_2,float xi_3,vec3 V,vec3 N,in Material material ){
    float alpha_GTR1 = mix(0.1, 0.001, material.clearcoatGloss);
    float alpha_GTR2 = max(0.001, sqr(material.roughness));

    //根据辐射度计算概率
    // 辐射度统计
    float r_diffuse = (1.0 - material.metallic);
    float r_specular = 1.0;
    float r_clearcoat = 0.25 * material.clearcoat;
    float r_sum = r_diffuse + r_specular + r_clearcoat;
    //根据辐射度计算概率
    float p_diffuse  = r_diffuse  /r_sum;
    float p_specular = r_specular /r_sum;
    float p_clearcot = r_clearcoat/r_sum;

    //按照概率采样
    float rd =xi_3;
    // 漫反射
    if(rd <= p_diffuse) {
        return SampleCosineHemisphere(xi_1, xi_2, N);
    }
    // 镜面反射
    else if(p_diffuse < rd && rd <= p_diffuse + p_specular) {
        return SampleGTR2(xi_1, xi_2, V, N, alpha_GTR2);
    }
    // 清漆
    else if(p_diffuse + p_specular < rd) {
        return SampleGTR1(xi_1, xi_2, V, N, alpha_GTR1);
    }
    return vec3(0, 1, 0);
}
// 获取BRDF在L 方向上的概率密度
float BRDF_Pdf(vec3 V,vec3 N,vec3 L,in Material material){
    float NdotL = dot(N,L);
    float NdotV = dot(N,V);
    if(NdotL<0||NdotV<0)    return 0;

    vec3 H = normalize(L+V);
    float NdotH = dot(N,H);
    float LdotH = dot(L,H);

    // 镜面反射 -- 各向同性
    float alpha = max(0.001, sqr(material.roughness));
    float Ds = GTR2(NdotH, alpha);
    float Dr = GTR1(NdotH, mix(0.1, 0.001, material.clearcoatGloss)); // 清漆

    // 分别计算三种 BRDF 的概率密度
    float pdf_diffuse = NdotL / PI;
    float pdf_specular = Ds * NdotH / (4.0 * dot(L, H));
    float pdf_clearcoat = Dr * NdotH / (4.0 * dot(L, H));// 辐射度统计
    float r_diffuse = (1.0 - material.metallic);
    float r_specular = 1.0;
    float r_clearcoat = 0.25 * material.clearcoat;
    float r_sum = r_diffuse + r_specular + r_clearcoat;
    // 根据辐射度计算选择某种采样方式的概率
    float p_diffuse = r_diffuse / r_sum;
    float p_specular = r_specular / r_sum;
    float p_clearcoat = r_clearcoat / r_sum;
    // 根据概率混合 pdf
    float pdf = p_diffuse * pdf_diffuse
    + p_specular * pdf_specular
    + p_clearcoat * pdf_clearcoat;
    pdf = max(1e-10, pdf);
    return pdf;

}
// 路径追踪
vec3 pathTracing(HitResult hit, int maxBounce) {
    vec3 Lo = vec3(0); // 最终的颜色
    vec3 history = vec3(1); // 递归积累的颜色运行代码：
    for(int bounce=0; bounce<maxBounce; bounce++) {

        vec3 V = -hit.viewDir;
        vec3 N = hit.normal;

        vec2 uv = sobolVec2(frameCounter+uint(1),uint(bounce));
        uv = CranleyPattersonRotation(uv);
        //普通半球随机采样
        //vec3 L = SampleHemisphere();
        //使用索贝序列的半球随机采样
        vec3 L = SampleHemisphere(uv.x, uv.y);
        L = toNormalHemisphere(L, hit.normal);

        float pdf = 1.0 / ( 2*PI); // 半球均匀采样概率密度,如果使用与cos 正相关的分布就使用2PI
        float cosine_o = max(0, dot(-hit.viewDir, hit.normal)); // 入射光和法线夹角余弦
        float cosine_i = max(0, dot(L, hit.normal)); // 出射光和法线夹角余弦
        //vec3 f_r = hit.material.baseColor / (2*PI); //与法线正相关的采样
        vec3 tangent, bitangent;
        getTangent(N, tangent, bitangent);
        vec3 f_r = BRDF_Evaluate_Disney(V, N, L, tangent, bitangent, hit.material);
        // 漫反射: 随机发射光线
        Ray randomRay;
        randomRay.startPoint = hit.hitPoint;
        randomRay.direction = L;
        HitResult newHit = hitBVH(randomRay);

        // 未命中
        if(!newHit.isHit) {
            vec3 skyColor = sampleHdr(randomRay.direction);
            Lo += history*skyColor*f_r*cosine_i/pdf;
            break;
        }
        // 命中光源积累颜色
        vec3 Le = newHit.material.emissive;
        Lo += history * Le * f_r * cosine_i / pdf;
        // 递归(步进)
        hit = newHit;
        history *= f_r * cosine_i / pdf; // 累积颜色


    }
    return Lo;
}
// 路径追踪
vec3 pathTracing_1(HitResult hit, int maxBounce) {
    vec3 Lo = vec3(0); // 最终的颜色
    vec3 history = vec3(1); // 递归积累的颜色运行代码：
    for(int bounce=0; bounce<maxBounce; bounce++) {

        vec3 V = -hit.viewDir;
        vec3 N = hit.normal;

        vec2 uv = sobolVec2(frameCounter+uint(1),uint(bounce));
        uv = CranleyPattersonRotation(uv);
        //普通半球随机采样
        //vec3 L = SampleHemisphere();
        //使用索贝序列的半球随机采样
        vec3 L = SampleCosineHemisphere(uv.x,uv.y,N);

        //以入射角与法线夹角余弦值为权重的采样
        //L = normalize(L+hit.normal);


        float pdf = dot(N,normalize(L+V))/PI; // 半球均匀采样概率密度,如果使用与cos 正相关的分布就使用2PI
        float cosine_o = max(0, dot(-hit.viewDir, hit.normal)); // 入射光和法线夹角余弦
        float cosine_i = max(0, dot(L, hit.normal)); // 出射光和法线夹角余弦
        //vec3 f_r = hit.material.baseColor / (2*PI); //与法线正相关的采样
        vec3 tangent, bitangent;
        getTangent(N, tangent, bitangent);
        vec3 f_r = BRDF_Evaluate_Disney(V, N, L, tangent, bitangent, hit.material);
        // 漫反射: 随机发射光线
        Ray randomRay;
        randomRay.startPoint = hit.hitPoint;
        randomRay.direction = L;
        HitResult newHit = hitBVH(randomRay);

        // 未命中
        if(!newHit.isHit) {
            vec3 skyColor = sampleHdr(randomRay.direction);
            Lo += history*skyColor*f_r*cosine_i/pdf;
            break;
        }
        // 命中光源积累颜色
        vec3 Le = newHit.material.emissive;
        Lo += history * Le * f_r * cosine_i / pdf;
        // 递归(步进)
        hit = newHit;
        history *= f_r * cosine_i / pdf; // 累积颜色


    }
    return Lo;
}
vec3 pathTracing_IS(HitResult hit, int maxBounce) {
    vec3 Lo = vec3(0); // 最终的颜色
    vec3 history = vec3(1); // 递归积累的颜色运行代码：
    for(int bounce=0; bounce<maxBounce; bounce++) {

        vec3 V = -hit.viewDir;
        vec3 N = hit.normal;
        // 获取 3 个随机数
        vec2 uv = sobolVec2(frameCounter+1u, uint(bounce));
        uv = CranleyPattersonRotation(uv);
        float xi_1 = uv.x;
        float xi_2 = uv.y;
        float xi_3 = rand();    // xi_3 是决定采样的随机数, 朴素 rand 就好

        vec3 L = SampleBRDF(xi_1,xi_2,xi_3,V,N,hit.material);
        float NdotL = dot(N, L);
        if(NdotL <= 0.0) break;
        // 发射光线
        Ray randomRay;
        randomRay.startPoint = hit.hitPoint;
        randomRay.direction = L;
        HitResult newHit = hitBVH(randomRay);
        // 获取 L 方向上的 BRDF 值和概率密度
        vec3 f_r = BRDF_Evaluate(V, N, L, hit.material);
        float pdf_brdf = BRDF_Pdf(V, N, L, hit.material);
        if(pdf_brdf <= 0.0) break;
        // 未命中
        if(!newHit.isHit) {
            vec3 color = sampleHdr(L);
            Lo += history * color * f_r * NdotL / pdf_brdf;
            break;
        }
        // 命中光源积累颜色
        vec3 Le = newHit.material.emissive;
        Lo += history * Le * f_r * NdotL / pdf_brdf;
        // 递归(步进)
        hit = newHit;
        history *= f_r * NdotL / pdf_brdf; // 累积颜色
    }
    return Lo;
}
void main(){
    Ray ray;

    //鼠标事件的逻辑是摄像机始终在一个固定圆心的球面上运动，鼠标拖拽改变的极坐标，滚轮改变的是半径。这个cameraRotate则是使标准的视线变为
    //经过变换的坐标系上的方向
    ray.startPoint = eye;
    float maxx=INF;
    int spp=2;
    vec3 color;
    for(int i=0;i<spp;i++){
        vec2 AA        = vec2((rand()-0.5)/float(width), (rand()-0.5)/float(height));
        vec4 dir       = cameraRotate*vec4(pix.xy+AA,-1.5,0.0);//(pix.xy, 2) - ray.startPoint;
        ray.direction  = normalize(dir.xyz);
        HitResult firstHit = hitBVH(ray);
        if(!firstHit.isHit) {
            color += sampleHdr(ray.direction)/spp;
        } else {
            int maxBounce = 3;
            vec3 Le = firstHit.material.emissive;
            vec3 Li = pathTracing_IS(firstHit, maxBounce);
            color += (Le + Li)/spp;
        }
    }





    vec3 lastColor = texture2D(lastFrame, pix.xy*0.5+0.5).rgb;

    color = mix(lastColor, color, 1.0/float(frameCounter+1.f));


    //gl_FragDate[0]是一个直接的颜色缓冲区



    fragColor = vec4(color, 1.0);


    //ragColor=vec4(color,1.0f);
    //HitResult res  = hitArray(ray,0,nTriangles-1);
    //if(res.isHit) fragColor = vec4(res.material.baseColor, 1);
    //fragColor=vec4(pix.xyz,1.0f);
}