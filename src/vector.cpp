#include "vector.hpp"
#include "math.h"


float3 operator/(float3 v, float s) {
    float3 result;
    result.x = v.x / s;
    result.y = v.y / s;
    result.z = v.z / s;
    return result;
}

float3 operator*(float3 v, float s) {
    float3 result;
    result.x = v.x * s;
    result.y = v.y * s;
    result.z = v.z * s;
    return result;
}

float3 operator*(float s, float3 v) {
    float3 result;
    result.x = v.x * s;
    result.y = v.y * s;
    result.z = v.z * s;
    return result;
}

float3 operator+(float3 v1, float3 v2) {
    float3 result;
    result.x = v1.x + v2.x;
    result.y = v1.y + v2.y;
    result.z = v1.z + v2.z;
    return result;
}

float3 operator-(float3 v1) {
    float3 result;
    result.x = -v1.x;
    result.y = -v1.y;
    result.z = -v1.z;
    return result;
}

float3 operator-(float3 v1, float3 v2) {
    float3 result;
    result.x = v1.x - v2.x;
    result.y = v1.y - v2.y;
    result.z = v1.z - v2.z;
    return result;
}

float3 operator/(float3 v1, float3 v2) {
    float3 result;
    result.x = v1.x / v2.x;
    result.y = v1.y / v2.y;
    result.z = v1.z / v2.z;
    return result;
}

float3& operator*=(float3& v, float s) {
    v = v * s;
    return v;
}

float3& operator+=(float3& v1, float3 v2) {
    v1 = v1 + v2;
    return v1;
}

float3& operator-=(float3& v1, float3 v2) {
    v1 = v1 - v2;
    return v1;
}

float3& operator/=(float3& v1, float3 v2) {
    v1 = v1 / v2;
    return v1;
}

float3 Cross(float3 a, float3 b) {
    float3 result;

    result.x = a.y*b.z - a.z*b.y;
    result.y = a.z*b.x - a.x*b.z;
    result.z = a.x*b.y - a.y*b.x;

    return result;
}

float Dot(float3 a, float3 b) {
    return
        a.x * b.x +
        a.y * b.y +
        a.z * b.z;
}

float SquaredLength(float3 v) {
    return Dot(v, v);
}

float3 Normalize(float3 v) {
    float3 result { 0 };

    float squaredLength = SquaredLength(v);
    if(squaredLength > 0.000001f) {
        result = v * (1.0f / sqrt(squaredLength));
    }

    return result;
}

float3 Abs(float3 v) {
    float3 result;
    result.x = fabs(v.x);
    result.y = fabs(v.y);
    result.z = fabs(v.z);
    return result;
}

float2x3 Abs(float2x3 M) {
    float2x3 result;
    result.v1 = Abs(M.v1);
    result.v2 = Abs(M.v2);
    return result;
}

bool operator<=(float3 v, float s) {
    return
        v.x <= s &&
        v.y <= s &&
        v.z <= s;
}

bool operator<=(float2x3 M, float s) {
    return M.v1 <= s && M.v2 <= s;
}

float2x3 operator/(float2x3 M1, float2x3 M2) {
    float2x3 result;
    result.v1 = M1.v1 / M2.v1;
    result.v2 = M1.v2 / M2.v2;
    return result;
}
