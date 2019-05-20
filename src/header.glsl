layout(std140) uniform inputs {
    vec4 screen;
    vec4 camera_pos;
    mat4 camera_dir;
    vec4 scene_pos;
    mat4 scene_dir;
    vec4 prev_camera_pos;
    mat4 prev_camera_dir;
};

layout(binding = 0, std140) buffer image_block {
    vec4 image[];
};

struct node {
    uint data, offset;
};
layout(binding = 1, std430) buffer tree_block {
    node nodes[];
};
