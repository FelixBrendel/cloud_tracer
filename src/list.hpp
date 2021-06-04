#pragma once
#include <cstdlib>
#include "stdint.h"

template <typename type>
struct list {
    uint32_t count;
    uint32_t length;
    type* data;

    inline void alloc(uint32_t initial_length = 16) {
        count = 0;
        length = initial_length;
        data = (type*)malloc(sizeof(type) * length);
    }

    inline void dealloc() {
        free(data);
        data = nullptr;
    }

    inline void clear() {
        count = 0;
    }

    void append(type element) {
        if (count == length) {
            length *= 2;
            data = (type*)realloc(data, length * sizeof(type));
        }
        data[count] = element;
        count++;
    }

    inline type* begin() {
        return data;
    }

    inline type* end() {
        return data+(count);
    }

     type& operator[](uint32_t index) {
        return data[index];
    }
};
