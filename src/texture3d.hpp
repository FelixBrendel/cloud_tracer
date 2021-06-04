#pragma once
#include "stdint.h"

struct texture3D {
    uint32_t w;
    uint32_t h;
    uint32_t d;
    float* Data;
};

texture3D ReadXYZFile(const char* Path);
