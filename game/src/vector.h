#include <math.h>

struct vector {
    float x[4];
};
struct matrix {
    float x[16];
};
float dot(struct vector a, struct vector b) {
    return a.x[0] * b.x[0] + a.x[1] * b.x[1] + a.x[2] * b.x[2];
}
float length(struct vector a) {
    return sqrt(a.x[0] * a.x[0] + a.x[1] * a.x[1] + a.x[2] * a.x[2]);
}
struct vector addvv(struct vector a, struct vector b) {
    return (struct vector){{
        a.x[0] + b.x[0],
        a.x[1] + b.x[1],
        a.x[2] + b.x[2],
    }};
}
struct vector subvv(struct vector a, struct vector b) {
    return (struct vector){{
        a.x[0] - b.x[0],
        a.x[1] - b.x[1],
        a.x[2] - b.x[2],
    }};
}
struct vector divvv(struct vector a, struct vector b) {
    return (struct vector){{
        a.x[0] / b.x[0],
        a.x[1] / b.x[1],
        a.x[2] / b.x[2],
    }};
}
struct vector cross(struct vector a, struct vector b) {
    return (struct vector){{
        a.x[1] * b.x[2] - a.x[2] * b.x[1],
        a.x[2] * b.x[0] - a.x[0] * b.x[2],
        a.x[0] * b.x[1] - a.x[1] * b.x[0],
    }};
}
struct vector mulvs(struct vector a, float b) {
    return (struct vector){{
        a.x[0] * b,
        a.x[1] * b,
        a.x[2] * b,
    }};
}
struct vector mulmv(struct matrix a, struct vector b) {
    return (struct vector){{
        a.x[0] * b.x[0] + a.x[1] * b.x[1] + a.x[2] * b.x[2],
        a.x[4] * b.x[0] + a.x[5] * b.x[1] + a.x[6] * b.x[2],
        a.x[8] * b.x[0] + a.x[9] * b.x[1] + a.x[10] * b.x[2],
    }};
}
struct vector normv(struct vector a) {
    return mulvs(a, 1 / length(a));
}
struct matrix matv(struct vector a) {
    float l = length(a);
    struct vector b = (struct vector){0};
    if (l != 0)
        b = mulvs(a, 1 / l);
    float c = cos(l);
    float s = sin(l);
    float t = 1 - c;
    return (struct matrix){{
        b.x[0] * b.x[0] * t + c         , b.x[0] * b.x[1] * t - b.x[2] * s, b.x[0] * b.x[2] * t + b.x[1] * s, 0,
        b.x[1] * b.x[0] * t + b.x[2] * s, b.x[1] * b.x[1] * t + c         , b.x[1] * b.x[2] * t - b.x[0] * s, 0,
        b.x[2] * b.x[0] * t - b.x[1] * s, b.x[2] * b.x[1] * t + b.x[0] * s, b.x[2] * b.x[2] * t + c, 0,
    }};
}
struct matrix invm(struct matrix a) {
    float id = 1 / (
        a.x[0] * (a.x[5] * a.x[10] - a.x[9] * a.x[6]) -
        a.x[1] * (a.x[4] * a.x[10] - a.x[6] * a.x[8]) +
        a.x[2] * (a.x[4] * a.x[ 9] - a.x[5] * a.x[8])
    );
    return (struct matrix){{
        (a.x[5] * a.x[10] - a.x[9] * a.x[ 6]) * id, (a.x[2] * a.x[ 9] - a.x[1] * a.x[10]) * id, (a.x[1] * a.x[6] - a.x[2] * a.x[5]) * id, 0,
        (a.x[6] * a.x[ 8] - a.x[4] * a.x[10]) * id, (a.x[0] * a.x[10] - a.x[2] * a.x[ 8]) * id, (a.x[4] * a.x[2] - a.x[0] * a.x[6]) * id, 0,
        (a.x[4] * a.x[ 9] - a.x[8] * a.x[ 5]) * id, (a.x[8] * a.x[ 1] - a.x[0] * a.x[ 9]) * id, (a.x[0] * a.x[5] - a.x[4] * a.x[1]) * id, 0,
    }};
}
struct matrix transm(struct matrix a) {
    return (struct matrix){{
        a.x[0], a.x[4], a.x[8], 0,
        a.x[1], a.x[5], a.x[9], 0,
        a.x[2], a.x[6], a.x[10], 0,
    }};
}
struct vector colm(struct matrix a, uint32_t i) {
    return (struct vector){{a.x[0 + i], a.x[4 + i], a.x[8 + i], a.x[12 + i]}};
}
