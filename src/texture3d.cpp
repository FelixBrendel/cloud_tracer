#include <cstdio>
#include <cstdlib>
#include "texture3d.hpp"
#include "defer.hpp"


texture3D ReadXYZFile(const char* Path) {
    uint32_t Buffer[9];

    FILE* XYZFile = fopen(Path, "rb");

    if (!XYZFile) {
        fprintf(stderr, "ERROR: File %s could not be opened\n", Path);
        return {};
    }
    defer {
        fclose(XYZFile);
    };


    size_t Read = fread(Buffer, sizeof(uint32_t)*9, 1, XYZFile);

    if (!Read) {
        fprintf(stderr, "ERROR: Header could not be read\n");
        return {};
    }

    texture3D Result;
    Result.w = Buffer[0];
    Result.h = Buffer[1];
    Result.d = Buffer[2];

    printf("INFO: Reading density map %u x %u x %u\n",
           Result.w, Result.h, Result.d);

    uint32_t FloatsCount = Result.w * Result.h * Result.d;
    Result.Data = (float*)malloc(FloatsCount * sizeof(float));

    Read = fread(Result.Data, FloatsCount * sizeof(float), 1, XYZFile);

    if (!Read) {
        fprintf(stderr, "ERROR: Data could not be read\n");
        return {};
    }

    return Result;
}
