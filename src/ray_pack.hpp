#pragma once
#include "list.hpp"
#include "vector.hpp"

struct ray_pack {
    list<float3>   vertices;
    list<uint32_t> ray_starts;

    void alloc() {
        vertices.alloc();
        ray_starts.alloc();
    }

    void dealloc() {
        vertices.dealloc();
        ray_starts.dealloc();
    }

    void reset() {
        vertices.clear();
        ray_starts.clear();
    }

    void add_vertex(float3 vert) {
        vertices.append(vert);
    }

    void start_new_ray() {
        ray_starts.append(vertices.count);
    }

};
