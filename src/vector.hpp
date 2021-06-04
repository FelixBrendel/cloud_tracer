#pragma once

typedef union {
    struct {
        float x, y, z;
    };
    struct {
        float r, g, b;
    };
    float v[3];
} float3;

typedef struct {
    float3 v1;
    float3 v2;
} float2x3;


float3 Cross(float3, float3);
float3 Normalize(float3);


float3   Abs(float3);
float2x3 Abs(float2x3);

float2x3 operator/(float2x3, float2x3);
bool operator<=(float2x3, float);

float3 operator-(float3);
float3 operator*(float3, float);
float3 operator*(float, float3);
float3 operator/(float3, float);
float3 operator+(float3, float3);
float3 operator-(float3, float3);
float3 operator/(float3, float3);

float3& operator*=(float3&, float);
float3& operator/=(float3&, float3);
float3& operator+=(float3&, float3);
float3& operator-=(float3&, float3);
