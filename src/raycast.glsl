layout(local_size_x = 8, local_size_y = 8) in;

vec2 aligned_ray_cube_intersect(vec3 rp, vec3 rd, ivec3 pos, float size) {
    vec3 t0 = (pos - rp) / rd;
    vec3 t1 = (pos - rp + vec3(size)) / rd;
    vec3 t2 = min(t0, t1);
    vec3 t3 = max(t0, t1);
    return vec2(
        max(max(t2.x, t2.y), t2.z),
        min(min(t3.x, t3.y), t3.z)
    );
}

vec4 raycast(vec3 rp, vec3 rd, vec3 cp, mat3 cd) {
    const uint tree_num = 1;
    const uint cellsize = uint(pow(2, tree_num));
    const uint treesize = uint(pow(cellsize, 3));
    const uint maxlevel = 12 / tree_num;
    const uint maxiter = 128;
    rp += -cp;
    rp *= inverse(cd);
    rd *= inverse(cd);
    vec2 ti = aligned_ray_cube_intersect(rp, rd, ivec3(0), pow(cellsize, maxlevel));
    ivec3 pos;
    vec3 fpos = vec3(0);
    if (ti.x < ti.y - 0.001) {
        if (ti.y <= 0) {
            return vec4(1);
        } else if (ti.x <= 0) {
            pos = ivec3(rp);
        } else {
            pos = ivec3(rp + rd * (ti.x + 0.25));
        }
    } else {
        return vec4(0);
    }
    uint level = maxlevel;
    node stack[maxlevel + 1];
    stack[maxlevel] = nodes[0];
    for (uint i = 0; i < maxiter; i++) {
        while (stack[level].offset != -1) {
            stack[--level] = nodes[stack[level + 1].offset
                + uint(dot((pos >> (tree_num * level + 0)) & ivec3(1), ivec3(1, 2, 4)))
                //+ uint(dot((pos >> (tree_num * level + 1)) & ivec3(1), ivec3(8, 16, 32)))
                //+ uint(dot((pos >> (tree_num * level + 2)) & ivec3(1), ivec3(64, 128, 256)))
            ];
        }
        pos = (pos >> (tree_num * level)) << (tree_num * level);
        if (stack[level].data != 0) {
            return vec4(i) / maxiter;
        }
        uvec3 dpos = uvec3(pos);
        fpos = rp + rd * (aligned_ray_cube_intersect(rp, rd, pos, pow(cellsize, level)).y + 0.01) + 0.001 * sign(rd);
        pos = ivec3(floor(rp + rd * (aligned_ray_cube_intersect(rp, rd, pos, pow(cellsize, level)).y + 0.01) + 0.001 * sign(rd)));
        dpos ^= uvec3(pos);
        level = findMSB(dpos.x | dpos.y | dpos.z) / tree_num + 1;
        if (level > maxlevel) {
            return vec4(i) / maxiter;
        }
    }
    return vec4(0);
};

void main() {
    uvec2 screen_pos = gl_GlobalInvocationID.xy;
    image[screen_pos.x + screen_pos.y * int(screen.x)] = raycast(
        camera_pos.xyz,
        mat3(camera_dir) * vec3((screen_pos.xy + vec2(0.5) - screen.xy / 2) / screen.xx, 1),
        scene_pos.xyz,
        mat3(scene_dir)
    );
}
