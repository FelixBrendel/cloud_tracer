#include "dt_pathtrace.hpp"
#include <cstdio>
#include <cmath>

// #define INFO(...) printf("INFO: "  __VA_ARGS__);
#define INFO(...)

#ifndef pi
#  define pi 3.1415926535897932384626433832795
#  define piOverTwo 1.5707963267948966192313216916398
#  define inverseOfPi 0.31830988618379067153776752674503
#  define inverseOfTwoPi 0.15915494309189533576888376337251
#  define two_pi 6.283185307179586476925286766559
#endif
namespace Random {
    struct rng_state {
        uint32_t x;
        uint32_t y;
        uint32_t z;
        uint32_t w;
    };

    static rng_state RNG_STATE;

    uint TausStep(uint z, int S1, int S2, int S3, uint M) {
        uint b = (((z << S1) ^ z) >> S2);
        return ((z & M) << S3) ^ b;
    }
    uint LCGStep(uint z, uint A, uint C) {
        return A * z + C;
    }

    float HybridTaus() {
        RNG_STATE.x = TausStep(RNG_STATE.x, 13, 19, 12, 4294967294);
        RNG_STATE.y = TausStep(RNG_STATE.y, 2, 25, 4, 4294967288);
        RNG_STATE.z = TausStep(RNG_STATE.z, 3, 11, 17, 4294967280);
        RNG_STATE.w = LCGStep(RNG_STATE.w, 1664525, 1013904223);

        return 2.3283064365387e-10 * (RNG_STATE.x ^ RNG_STATE.y ^ RNG_STATE.z ^ RNG_STATE.w);
    }

    float random() {
        return HybridTaus();
    }

    void InitRNG(uint32_t seed) {

        RNG_STATE.x = seed;
        RNG_STATE.y = seed;
        RNG_STATE.z = seed;
        RNG_STATE.w = seed;

        for (int i = 0; i < 23 + seed % 13; i++)
            random();
    }

}
inline double max(double a, double b) {
    return a > b ? a : b;
}

inline double min(double a, double b) {
    return a < b ? a : b;
}

inline float max(float a, float b) {
    return a > b ? a : b;
}

inline float min(float a, float b) {
    return a < b ? a : b;
}

inline float max(float a, float b, float c) {
    return max(a, max(b, c));
}

inline float min(float a, float b, float c) {
    return min(a, min(b, c));
}

inline uint32_t max(uint32_t a, uint32_t b) {
    return a > b ? a : b;
}

inline uint32_t min(uint32_t a, uint32_t b) {
    return a < b ? a : b;
}

inline float Lerp(float a, float t, float b) {
    return a + t * (b - a);
}

float Sample3DTexture(texture3D Grid, float3 P) {
    // cubic interpolation

#define IDX(x, y, z) ((x * Grid.h * Grid.d) + (y * Grid.d) + z)

    float fw = Grid.w * P.x;
    float fh = Grid.h * P.y;
    float fd = Grid.d * P.z;

    float tw = fw - (int)fw;
    float th = fh - (int)fh;
    float td = fd - (int)fd;

    uint32_t idx_w_left = floor(fw);
    uint32_t idx_h_low  = floor(fh);
    uint32_t idx_d_near = floor(fd);

    uint32_t idx_w_right = ceil(fw);
    uint32_t idx_h_high  = ceil(fh);
    uint32_t idx_d_far   = ceil(fd);

    float left_low_near   = Grid.Data[IDX(idx_w_left,  idx_h_low,  idx_d_near)];
    float left_low_far    = Grid.Data[IDX(idx_w_left,  idx_h_low,  idx_d_far)];
    float left_high_near  = Grid.Data[IDX(idx_w_left,  idx_h_high, idx_d_near)];
    float left_high_far   = Grid.Data[IDX(idx_w_left,  idx_h_high, idx_d_far)];
    float right_low_near  = Grid.Data[IDX(idx_w_right, idx_h_low,  idx_d_near)];
    float right_low_far   = Grid.Data[IDX(idx_w_right, idx_h_low,  idx_d_far)];
    float right_high_near = Grid.Data[IDX(idx_w_right, idx_h_high, idx_d_near)];
    float right_high_far  = Grid.Data[IDX(idx_w_right, idx_h_high, idx_d_far)];

    float low_near = Lerp(left_low_near, tw, right_low_near);
    float low_far  = Lerp(left_low_far,  tw, right_low_far);
    float low      = Lerp(low_near,      td, low_far);

    float high_near = Lerp(left_high_near, tw, right_high_near);
    float high_far  = Lerp(left_high_far,  tw, right_high_far);
    float high      = Lerp(high_near,      td, high_far);

    return Lerp(low, th, high);

#undef IDX
}

void CreateOrthonormalBasis(float3 D, float3& B, float3& T) {
    float3 other = fabs(D.z) >= 0.999 ? float3{1, 0, 0} : float3{0, 0, 1};
    B = Normalize(Cross(other, D));
    T = Normalize(Cross(D, B));
}

void CreateOrthonormalBasis2(float3 D, float3& B, float3& T) {
    float3 other = fabs(D.z) >= 0.999 ? float3{1, 0, 0} : float3{0, 0, 1};
    B = Normalize(Cross(other, D));
    T = Normalize(Cross(D, B));
}

float3 RandomDirection(float3 D) {
    float r1 = Random::random();
    float r2 = Random::random() * 2 - 1;
    float sqrR2 = r2 * r2;
    float two_pi_by_r1 = two_pi * r1;
    float sqrt_of_one_minus_sqrR2 = sqrt(1.0 - sqrR2);
    float x = cos(two_pi_by_r1) * sqrt_of_one_minus_sqrR2;
    float y = sin(two_pi_by_r1) * sqrt_of_one_minus_sqrR2;
    float z = r2;

    float3 t0, t1;
    CreateOrthonormalBasis2(D, t0, t1);

    return t0 * x + t1 * y + D * z;
}

float invertcdf(float GFactor, float xi) {
#define one_minus_g2 (1.0 - (GFactor) * (GFactor))
#define one_plus_g2 (1.0 + (GFactor) * (GFactor))
#define one_over_2g (0.5 / (GFactor))

    float t = (one_minus_g2) / (1.0f - GFactor + 2.0f * GFactor * xi);
    return one_over_2g * (one_plus_g2 - t * t);

#undef one_minus_g2
#undef one_plus_g2
#undef one_over_2g
}

float3 ImportanceSamplePhase(float GFactor, float3 D) {
    if (fabs(GFactor) < 0.001) {
        return RandomDirection(-D);
    }

    float phi = Random::random() * 2 * pi;
    float cosTheta = invertcdf(GFactor, Random::random());
    float sinTheta = sqrt(max(0, 1.0f - cosTheta * cosTheta));

    float3 t0, t1;
    CreateOrthonormalBasis(D, t0, t1);

    return sinTheta * sin(phi) * t0 + sinTheta * cos(phi) * t1 +
        cosTheta * D;
}

void GetGridBox(texture3D grid, float3& minim, float3& maxim) {
    float maxDim = max(grid.w, grid.h, grid.d);
    maxim = float3{(float)grid.w, (float)grid.h, (float)grid.d} / maxDim * 0.5;
    minim = -maxim;
}

bool BoxIntersect(float3 bMin, float3 bMax, float3 P, float3 D, float& tMin, float& tMax) {
    float2x3 C  = float2x3{ bMin - P, bMax - P };
    float2x3 D2 = float2x3{ D, D };

    float2x3 T =
        Abs(D2) <= 0.000001
        ? float2x3{float3{-1000, -1000, -1000}, float3{1000, 1000, 1000}}
        : C / D2;

    tMin = max(min(T.v1.v[0], T.v2.v[0]),
               min(T.v1.v[1], T.v2.v[1]),
               min(T.v1.v[2], T.v2.v[2]));

    tMin = max(0.0f, tMin);

    tMax = min(max(T.v1.v[0], T.v2.v[0]),
               max(T.v1.v[1], T.v2.v[1]),
               max(T.v1.v[2], T.v2.v[2]));

    if (tMax < tMin || tMax < 0) {
        return false;
    }
    return true;
}

void DTPathtrace(path_info PathInfo, volume_info VolumeInfo, ray_pack* RayPack) {

    int PassNumber = PathInfo.PassNumber;
    float3 x = PathInfo.x;
    float3 w = PathInfo.w;

    float density = VolumeInfo.Extinction.v[PassNumber % 3]; // extinction coefficient multiplier.
    INFO("  density: %f\n", density);

    float3 bMin, bMax;
    GetGridBox(VolumeInfo.Grid, bMin, bMax);

    float3 W = { 0 }; // weight
    W.v[PassNumber % 3] = 3; // Wavelenght dependent importance multiplier

    float tMin, tMax;
    if (!BoxIntersect(bMin, bMax, x, w, tMin, tMax))
        return;

    RayPack->start_new_ray();
    RayPack->add_vertex(x);


    float d = tMax - tMin;
    x += w * tMin;

    RayPack->add_vertex(x);

    while (true) {

        float t =
            density <= 0.00001
            ? 10000000
            : -log(max(0.00000000001, 1.0 - Random::random())) / density; // majorant of all volume data

        INFO("  t: %f\n", t);
        INFO("  d: %f\n", d);

        if (t >= d) {
            INFO("->Ray left the volume\n");
            RayPack->add_vertex(x + (w * t));
            return;
        }

        x += w * t; // move to next event position or border
        RayPack->add_vertex(x);

        float3 tSamplePosition = (x - bMin) / (bMax - bMin);
        float probExt = Sample3DTexture(VolumeInfo.Grid, tSamplePosition);
        INFO("  sample pos: %f %f %f\n", tSamplePosition.x, tSamplePosition.y, tSamplePosition.z);
        INFO("  density there: %f\n", probExt);

        float m_t = probExt * density; // extinction coef
        float m_s = m_t * VolumeInfo.ScatteringAlbedo.v[PassNumber % 3]; // scattering coef
        float m_a = m_t - m_s; // absorption coef
        float m_n = density - m_t; // null coef

        float xi = Random::random();

        float Pa = m_a / density;
        float Ps = m_s / density;
        float Pn = m_n / density;

        if (xi < Pa) { // absorption
            INFO("->absorbtion\n");
            return;
        }

        if (xi < 1 - Pn) { // scattering
            INFO("->scatter\n");
            w = ImportanceSamplePhase(VolumeInfo.G.v[PassNumber % 3], w); // scattering event...

            if (!BoxIntersect(bMin, bMax, x, w, tMin, tMax))
                return;

            d = tMax - tMin;
            x += w * tMin;
        } else {
            INFO("->null collision\n");
            // if no absorption and no scattering null collision occurred
            d -= t;
        }
    }
}
