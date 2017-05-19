#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "file.h"

uint64_t shrink(uint64_t x) {
    x &= 0x1249249249249249;
    x = (x | x >>  2) & 0x10c30c30c30c30c3;
    x = (x | x >>  4) & 0x100f00f00f00f00f;
    x = (x | x >>  8) & 0x001f0000ff0000ff;
    x = (x | x >> 16) & 0x001f00000000ffff;
    x = (x | x >> 32) & 0x1fffff;
    return x;
}

uint32_t density(uint64_t m) {
    float x = -2 + 4 * (float)shrink(m >> 0) / 1024;
    float y = -2 + 4 * (float)shrink(m >> 1) / 1024;
    float z = -2 + 4 * (float)shrink(m >> 2) / 1024;
    float ff = 0.075;
    float bb = 3;
    float p = (pow(x*x + y*y - 1, 2) + z*z)*(pow(y*y + z*z - 1, 2) + x*x)*(pow(z*z + x*x - 1, 2) + y*y) - ff*ff*(1 + bb*(x*x + y*y + z*z));
    return p < 0;
}

int main() {
    struct __attribute__((packed)) node {
        uint32_t data;
        uint32_t offset;
    };
    struct node* array = malloc(1024 * 1024 * 512 * sizeof(struct node));
    struct node stack[20][8] = {{{0}}};
    uint64_t pointer[20] = {0};
    uint64_t index = 1;
    uint64_t layer;

    uint64_t start;
    uint64_t morton = 0;
    uint64_t length;
    uint64_t limit;
    uint32_t ds;
    uint32_t maxlevel = 10;

    while (morton < exp2(3 * maxlevel)) {
        start = morton;
        morton += 1;
        layer = ((int)log2(start ^ (start - 1)) / 3) * 3;
        limit = (start & ~((uint64_t)(7) << layer)) + ((uint64_t)(8) << layer);
        if (start == 0)
            limit = -1;

        ds = density(start);
        while (density(morton) == ds && morton < limit && morton < exp2(3 * maxlevel)) {
            morton += 1;
        }

        layer = 0;
        length = morton - start;
        while (length > 0) {
            while ((length & 7) > 0) {
                stack[layer][pointer[layer]] = (struct node){.data = ds, .offset = -1};
                pointer[layer] += 1;
                length -= 1;
            }
            layer += 1;
            length >>= 3;
        }
        layer -= 1;

        while (pointer[layer] == 8) {
            array[index + 0] = stack[layer][0];
            array[index + 1] = stack[layer][1];
            array[index + 2] = stack[layer][2];
            array[index + 3] = stack[layer][3];
            array[index + 4] = stack[layer][4];
            array[index + 5] = stack[layer][5];
            array[index + 6] = stack[layer][6];
            array[index + 7] = stack[layer][7];
            stack[layer + 1][pointer[layer + 1]] = (struct node){.data = 0, .offset = index};
            pointer[layer + 1] += 1;
            pointer[layer] -= 8;
            index += 8;
            layer += 1;
        }
    }
    array[0] = stack[layer][0];
    index += 1;
    printf("%f%% of 2gb octree used\n", 100 * ((float)index * 8) / ((float)1024 * 1024 * 1024 * 2));
    save("test.oct", array, index * sizeof(struct node));
    return 0;
}
