#include <stdint.h>

namespace morton {
    __uint128_t spread(uint32_t a) {
        __uint128_t x = 0;
        x = a;
        x = (x ^ x << 64) & (((__uint128_t)0x000003ff00000000 << 64) | 0x00000000ffffffff);
        x = (x ^ x << 32) & (((__uint128_t)0x000003ff00000000 << 64) | 0xffff00000000ffff);
        x = (x ^ x << 16) & (((__uint128_t)0x030000ff0000ff00 << 64) | 0x00ff0000ff0000ff);
        x = (x ^ x <<  8) & (((__uint128_t)0x0300f00f00f00f00 << 64) | 0xf00f00f00f00f00f);
        x = (x ^ x <<  4) & (((__uint128_t)0x030c30c30c30c30c << 64) | 0x30c30c30c30c30c3);
        x = (x ^ x <<  2) & (((__uint128_t)0x0924924924924924 << 64) | 0x9249249249249249);
        return x;
    };

    uint32_t shrink(__uint128_t x) {
        x = x             & (((__uint128_t)0x0924924924924924 << 64) | 0x9249249249249249);
        x = (x ^ x >>  2) & (((__uint128_t)0x030c30c30c30c30c << 64) | 0x30c30c30c30c30c3);
        x = (x ^ x >>  4) & (((__uint128_t)0x0300f00f00f00f00 << 64) | 0xf00f00f00f00f00f);
        x = (x ^ x >>  8) & (((__uint128_t)0x030000ff0000ff00 << 64) | 0x00ff0000ff0000ff);
        x = (x ^ x >> 16) & (((__uint128_t)0x000003ff00000000 << 64) | 0xffff00000000ffff);
        x = (x ^ x >> 32) & (((__uint128_t)0x000003ff00000000 << 64) | 0x00000000ffffffff);
        x = (x ^ x >> 64) & (((__uint128_t)0x0000000000000000 << 64) | 0xffffffffffffffff);
        return (uint32_t)x;
    };

    struct pos {
        uint32_t x, y, z;
    };

    __uint128_t encode(struct pos pos) {
        return
            (spread(pos.x) << 0) |
            (spread(pos.y) << 1) |
            (spread(pos.z) << 2);
    };

    struct pos decode(__uint128_t morton) {
        return (struct pos){
            shrink(morton >> 0),
            shrink(morton >> 1),
            shrink(morton >> 2),
        };
    };

    uint32_t ancestor(__uint128_t m0, __uint128_t m1) {
        __uint128_t m = m0 ^ m1;
        uint32_t level = 0;
        while (m) {
            m >>= 3;
            level++;
        }
        return level;
    };

    const __uint128_t neg_dx = encode((struct pos){(uint32_t)-1, 0, 0});
    const __uint128_t neg_dy = encode((struct pos){0, (uint32_t)-1, 0});
    const __uint128_t neg_dz = encode((struct pos){0, 0, (uint32_t)-1});
    const __uint128_t pos_dx = encode((struct pos){+1, 0, 0});
    const __uint128_t pos_dy = encode((struct pos){0, +1, 0});
    const __uint128_t pos_dz = encode((struct pos){0, 0, +1});
    const __uint128_t mask_x = encode((struct pos){0xffffffff, 0, 0});
    const __uint128_t mask_y = encode((struct pos){0, 0xffffffff, 0});
    const __uint128_t mask_z = encode((struct pos){0, 0, 0xffffffff});

    __uint128_t increment(__uint128_t morton, uint32_t index) {
        __uint128_t inc_x = (index >> 0) & 1;
        __uint128_t inc_y = (index >> 1) & 1;
        __uint128_t inc_z = (index >> 2) & 1;
        return
            (((morton & mask_x) - neg_dx * inc_x) & mask_x) |
            (((morton & mask_y) - neg_dy * inc_y) & mask_y) |
            (((morton & mask_z) - neg_dz * inc_z) & mask_z);
    };

    __uint128_t decrement(__uint128_t morton, uint32_t index) {
        __uint128_t inc_x = (index >> 0) & 1;
        __uint128_t inc_y = (index >> 1) & 1;
        __uint128_t inc_z = (index >> 2) & 1;
        return
            (((morton & mask_x) - pos_dx * inc_x) & mask_x) |
            (((morton & mask_y) - pos_dy * inc_y) & mask_y) |
            (((morton & mask_z) - pos_dz * inc_z) & mask_z);
    };
};
