//super fast method to generate a pseudo random hash from a coordinate value
uint hash(uvec4 d) {
    d = d * 0xcc9e2d51;
    d = ((d << 15) | (d >> 17)) * 0x1b873593;

    uint h = 0x71eb3944 ^ d.x;
    h = (((h << 13) | (h >> 19)) * 5 + 0xe6546b64) ^ d.y;
    h = (((h << 13) | (h >> 19)) * 5 + 0xe6546b64) ^ d.z;
    h = (((h << 13) | (h >> 19)) * 5 + 0xe6546b64) ^ d.w;
    h = (((h << 13) | (h >> 19)) * 5 + 0xe6546b64);

    h = (h ^ 4);
    h = (h ^ (h >> 16)) * 0x85ebca6b;
    h = (h ^ (h >> 13)) * 0xc1b2ae35;
    h = (h ^ (h >> 16));
    return h;
}

//smooth noise function generates a 0-1 float from a xyzw coordinate
float noise(vec4 pos) {
    const float F_CONST = 0.30901699437;
    const float G_CONST = 0.13819660112;
    vec4 cell = vec4(floor(pos + dot(pos, vec4(F_CONST))));
    vec4 offset0 = pos - cell + dot(cell, vec4(G_CONST));

    vec4 rank = vec4(
        float(offset0.x >  offset0.y) + float(offset0.x >  offset0.z) + float(offset0.x >  offset0.w),
        float(offset0.y >= offset0.x) + float(offset0.y >  offset0.z) + float(offset0.y >  offset0.w),
        float(offset0.z >= offset0.x) + float(offset0.z >= offset0.y) + float(offset0.z >  offset0.w),
        float(offset0.w >= offset0.x) + float(offset0.w >= offset0.y) + float(offset0.w >= offset0.z)
    );

    vec4 cell1 = vec4(greaterThan(rank, vec4(2)));
    vec4 cell2 = vec4(greaterThan(rank, vec4(1)));
    vec4 cell3 = vec4(greaterThan(rank, vec4(0)));

    vec4 offset1 = offset0 - cell1 + 1 * G_CONST;
    vec4 offset2 = offset0 - cell2 + 2 * G_CONST;
    vec4 offset3 = offset0 - cell3 + 3 * G_CONST;
    vec4 offset4 = offset0 - 1     + 4 * G_CONST;

    vec4 grad0 = ((hash(uvec4(cell + 0    )) >> uvec4(0, 1, 2, 3)) & 1) * vec4(2) - vec4(1);
    vec4 grad1 = ((hash(uvec4(cell + cell1)) >> uvec4(0, 1, 2, 3)) & 1) * vec4(2) - vec4(1);
    vec4 grad2 = ((hash(uvec4(cell + cell2)) >> uvec4(0, 1, 2, 3)) & 1) * vec4(2) - vec4(1);
    vec4 grad3 = ((hash(uvec4(cell + cell3)) >> uvec4(0, 1, 2, 3)) & 1) * vec4(2) - vec4(1);
    vec4 grad4 = ((hash(uvec4(cell + 1    )) >> uvec4(0, 1, 2, 3)) & 1) * vec4(2) - vec4(1);

    float n0 = 0.6 - dot(offset0, offset0);
    float n1 = 0.6 - dot(offset1, offset1);
    float n2 = 0.6 - dot(offset2, offset2);
    float n3 = 0.6 - dot(offset3, offset3);
    float n4 = 0.6 - dot(offset4, offset4);

    return 0.5 + 10.125 * (
        float(n0 > 0) * n0 * n0 * n0 * n0 * dot(grad0, offset0) +
        float(n1 > 0) * n1 * n1 * n1 * n1 * dot(grad1, offset1) +
        float(n2 > 0) * n2 * n2 * n2 * n2 * dot(grad2, offset2) +
        float(n3 > 0) * n3 * n3 * n3 * n3 * dot(grad3, offset3) +
        float(n4 > 0) * n4 * n4 * n4 * n4 * dot(grad4, offset4)
    );
}

//octave noise function generates realistic ish noise from the smooth noise function
float octave_noise(float octaves, float weight, vec4 pos) {
    float total = 0, amplitude = 1, maximum = 0;
    for (int i = 0;i < octaves;i++) {
        total += amplitude * noise(pos);
        pos *= 2;
        maximum += amplitude;
        amplitude *= weight;
    }
    return total / maximum;
}
