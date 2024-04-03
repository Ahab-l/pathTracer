//
// Created by Ahab on 2024/3/30.
//

#ifndef RT_3_ENVIRONMENTMAP_H
#define RT_3_ENVIRONMENTMAP_H
#include "hdrloader.h"
class EnvironmentMap
{
    public:
    EnvironmentMap():cdf(nullptr){};


    HDRLoaderResult* hdres;
    float totalSum;
    float* img;
    float* cdf;
};
#endif //RT_3_ENVIRONMENTMAP_H
