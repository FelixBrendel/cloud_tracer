#include <cstdio>
#include <cmath>
#include <cstring>

#include "vector.hpp"
#include "texture3d.hpp"
#include "list.hpp"
#include "defer.hpp"
#include "dt_pathtrace.hpp"
#include "ray_pack.hpp"
#include "scene.hpp"

void WriteDomainToOBJFile(texture3D Grid) {
    FILE* out = fopen("output/Domain.obj", "w");
    if (!out) {
        fprintf(stderr, "ERROR: file %s could not be opened\n", "output/Domain.obj");
        return;
    }
    defer {
        fclose(out);
    };

    // Domain Box
    float3 maxim;
    float3 minim;
    GetGridBox(Grid, minim, maxim);
    fprintf(out, "o Domain\n");

    fprintf(out, "v %f %f %f\n",  maxim.x,  maxim.y,  maxim.z);
    fprintf(out, "v %f %f %f\n",  maxim.x,  maxim.y, -maxim.z);
    fprintf(out, "v %f %f %f\n",  maxim.x, -maxim.y,  maxim.z);
    fprintf(out, "v %f %f %f\n",  maxim.x, -maxim.y, -maxim.z);
    fprintf(out, "v %f %f %f\n", -maxim.x,  maxim.y,  maxim.z);
    fprintf(out, "v %f %f %f\n", -maxim.x,  maxim.y, -maxim.z);
    fprintf(out, "v %f %f %f\n", -maxim.x, -maxim.y,  maxim.z);
    fprintf(out, "v %f %f %f\n", -maxim.x, -maxim.y, -maxim.z);


    fprintf(out, "l %u %u\n", 1, 2);
    fprintf(out, "l %u %u\n", 3, 4);
    fprintf(out, "l %u %u\n", 5, 6);
    fprintf(out, "l %u %u\n", 7, 8);

    fprintf(out, "l %u %u\n", 1, 3);
    fprintf(out, "l %u %u\n", 2, 4);
    fprintf(out, "l %u %u\n", 5, 7);
    fprintf(out, "l %u %u\n", 6, 8);

    fprintf(out, "l %u %u\n", 1, 5);
    fprintf(out, "l %u %u\n", 2, 6);
    fprintf(out, "l %u %u\n", 3, 7);
    fprintf(out, "l %u %u\n", 4, 8);
}

void WriteRayPackToOBJFile(int x, int y, ray_pack* RayPack) {
    if (RayPack->ray_starts.count == 0) {
        return;
    }

    char OBJFileName[1024];
    sprintf(OBJFileName, "output/Ray_Pack_%d_%d.obj", x, y);

    FILE* out = fopen(OBJFileName, "w");
    // FILE* out = stdout;
    if (!out) {
        fprintf(stderr, "ERROR: file %s could not be opened\n", OBJFileName);
        return;
    }
    defer {
        fclose(out);
    };

    for (float3 v : RayPack->vertices) {
        fprintf(out, "v %f %f %f\n", v.x, v.y, v.z);
    }

    fprintf(out, "g Rays\n");
    int start = 0;
    int end;
    // all but the last sample:
    int i = 0;
    for (; i < (int)(RayPack->ray_starts.count)-1; ++i) {
        end = RayPack->ray_starts[i+1];
        fprintf(out, "o Ray_%u\n", i);

        for (int j = start; j < end-1; ++j) {
            // NOTE(Felix): +1 because obj vertex ids start counting at 1
            fprintf(out, "l %u %u\n", j+1, j+2);
        }
        
        start = end;
    }
    // NOTE(Felix): the last sample;
    end = RayPack->vertices.count;
    fprintf(out, "o Ray_%u\n", i);
    for (int j = start; j < end-1; ++j) {
        // NOTE(Felix): +1 because obj vertex ids start counting at 1
        fprintf(out, "l %u %u\n", j+1, j+2);
    }
}

void WritePixelGridToOBJFile(float GridWidth, float GridHeight, int ResX, int ResY,
                             float D, float3 P0, float3 X, float3 Y)
{
    FILE* out = fopen("output/Camera.obj", "w");
    if (!out) {
        fprintf(stderr, "ERROR: file %s could not be opened\n", "output/Domain.obj");
        return;
    }
    defer {
        fclose(out);
    };

    float PixelLength = GridWidth / ResX;

    P0 = P0 -
        PixelLength / 2 * X -
        PixelLength / 2 * Y;
    GridWidth += PixelLength;
    GridHeight += PixelLength;

    fprintf(out, "o Camera\n");

    int VertIndex = 1;

    // horizontal lines
    for (int i = 0; i <= ResY; ++i) {
        float3 left = P0 + (1.0*i/ResY) * Y * GridHeight;
        float3 right= left + X * GridWidth;

        fprintf(out, "v %f %f %f\n", left.x, left.y, left.z);
        fprintf(out, "v %f %f %f\n", right.x, right.y, right.z);
        fprintf(out, "l %d %d\n", VertIndex, VertIndex+1);

        VertIndex += 2;

    }

    // Vertical lines
    for (int i = 0; i <= ResX; ++i) {
        float3 top = P0 + (1.0*i/ResX) * X * GridWidth;
        float3 bottom = top + Y * GridWidth;

        fprintf(out, "v %f %f %f\n", top.x, top.y, top.z);
        fprintf(out, "v %f %f %f\n", bottom.x, bottom.y, bottom.z);
        fprintf(out, "l %d %d\n", VertIndex, VertIndex+1);

        VertIndex += 2;

    }

}

void RunSimulationAndOutputScene(scene S) {
    ray_pack RayPack;
    RayPack.alloc();
    defer {
        RayPack.dealloc();
    };

    Random::InitRNG(S.Seed);

    float3 CameraDirection = Normalize(S.CameraLookAt - S.CameraPos);

    float3 Y = { 0, 0, -1 };
    float3 X = Cross(CameraDirection, Y);
    Y = Cross(X, CameraDirection);

    // printf("X: %f %f %f\n", X.x, X.y, X.z);
    // printf("Y: %f %f %f\n", Y.x, Y.y, Y.z);

    float D = 0.5f; // how far away from the camera the grid will be
    float GridWidth = tan(S.CameraFOV / 2) * 2 * D;
    float GridHeight = S.ResY * 1.0 / S.ResX * GridWidth;

    float3 P0 =
        S.CameraPos +
        CameraDirection * D -
        0.5f * Y * GridHeight -
        0.5f * X * GridWidth;

    // printf("P0: %f %f %f\n", P0.x, P0.y,P0.z);

    path_info PI = { };
    PI.x = S.CameraPos;

    // Pixel loop
    for (int y = 0; y < S.ResY; ++y) {
        for (int x = 0; x < S.ResX; ++x) {
            // printf("INFO: %d %d\n", x, y);
            float3 P =
                P0 +
                X * (x*1.0/(S.ResX-1)) * GridWidth +
                Y * (y*1.0/(S.ResY-1)) * GridHeight;
            // printf("P: %f %f %f\n", P.x, P.y, P.z);

            P = Normalize(P - S.CameraPos);
            // printf("dir: %f %f %f\n", P.x, P.y, P.z);

            PI.w = P;
            // Sample loop
            for (int i = 0; i < S.SamplesPerPixel; ++i) {
                PI.PassNumber = i;
                DTPathtrace(PI, S.VolInfo, &RayPack);
            }

            WriteRayPackToOBJFile(x, y, &RayPack);
            RayPack.reset();
        }
    }

    WriteDomainToOBJFile(S.VolInfo.Grid);
    WritePixelGridToOBJFile(GridWidth, GridHeight, S.ResX, S.ResY,
                            D, P0, X, Y);
}

int main(int ArgCount, char** Args) {
    if (ArgCount != 2) {
        fprintf(stderr, "Usage: cloud_tracer <scene_file>\n");
        return 1;
    }

    char* SceneFile = Args[1];
    scene Scene = LoadSceneFromFile(SceneFile);
    RunSimulationAndOutputScene(Scene);

    return 0;
}
