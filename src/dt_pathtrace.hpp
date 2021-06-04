#pragma once
#include "stdint.h"
#include "vector.hpp"
#include "texture3d.hpp"
#include "ray_pack.hpp"

struct volume_info {
    texture3D Grid;
    float3 Extinction;
    float3 ScatteringAlbedo;
    float3 G;
};

struct path_info {
    float3 x;
    float3 w;
    int PassNumber;
};

void GetGridBox(texture3D grid, float3& minim, float3& maxim);
void DTPathtrace(path_info PathInfo, volume_info VolumeInfo, ray_pack* RayPack);

namespace Random {
    void InitRNG(uint32_t seed);
}
