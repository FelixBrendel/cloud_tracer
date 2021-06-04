#pragma once

#include "dt_pathtrace.hpp"

struct scene {
    const char* SceneFile;

    volume_info VolInfo;
    uint32_t Seed;

    float3 CameraPos;
    float3 CameraLookAt;
    float  CameraFOV;
    uint32_t ResX;
    uint32_t ResY;
    uint32_t SamplesPerPixel;
};

scene LoadSceneFromFile(const char* FilePath);
