#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "file.h"
#include "vector.h"
struct __attribute__((packed)) inputs {
    struct vector screen;
    struct vector camera_pos;
    struct matrix camera_dir;
    struct vector scene_pos;
    struct matrix scene_dir;
    struct vector prev_camera_pos;
    struct matrix prev_camera_dir;
} inputs;

float (*image)[4];
struct node {
    uint32_t data, offset;
} __attribute__((packed));
struct node* nodes;
int width, height;
#include "traverse.c"

static void error_callback(int error, const char* description) {
    printf("glfw error: %s\n", description);
}

int main(int argc, char* argv[]) {
    //setup glfw
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        printf("glfw error: failed to initialise\n");
		return 1;
    }

    //create window
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    width = mode->width;
    height = mode->height;
    GLFWwindow* window = glfwCreateWindow(width, height, "project", NULL, NULL);
    if (!window) {
        glfwTerminate();
        printf("glfw error: failed to create window\n");
		return 1;
    }

    //setup window
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    //glfwGetFramebufferSize(window, &width, &height);

    //create shaders
    char* error_log = calloc(4096, sizeof(char));
    const char* shader_fragment[] = {
        "#version 450\n",
        "#line 1 0\n", load("header.glsl"),
        "#line 1 1\n", load("fragment.glsl"),
    };
    GLuint program_fragment = glCreateShaderProgramv(GL_FRAGMENT_SHADER, sizeof(shader_fragment) / sizeof(void*), shader_fragment);
    glGetProgramInfoLog(program_fragment, 4096, NULL, error_log);
    if (error_log[0] != 0) {
        printf("fragment:\n%s\n", error_log);
        return 1;
    }
    const char* shader_raycast[] = {
        "#version 450\n",
        "#line 1 0\n", load("header.glsl"),
        "#line 1 1\n", load("raycast.glsl"),
    };
    GLuint program_raycast = glCreateShaderProgramv(GL_COMPUTE_SHADER, sizeof(shader_raycast) / sizeof(void*), shader_raycast);
    glGetProgramInfoLog(program_raycast, 4096, NULL, error_log);
    if (error_log[0] != 0) {
        printf("raycast:\n%s\n", error_log);
        return 1;
    }

    //create vertex buffer
    float vertices[] = {-1, -1, -1, 3, 3, -1};
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    //create uniform buffer
    GLuint ubo;
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(inputs), &inputs, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, glGetUniformBlockIndex(program_raycast, "inputs"), ubo);

    //create octree buffer
    if (argc != 2) {
        printf("expects one argument: svt filename\n");
        return 1;
    }
    char* filename = argv[1];
    GLuint octree;
    glGenBuffers(1, &octree);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, glGetProgramResourceIndex(program_raycast, GL_SHADER_STORAGE_BLOCK, "tree_block"), octree);
    nodes = (struct node*)load(filename);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size(filename), nodes, GL_DYNAMIC_DRAW);

    //create image buffer
    GLuint image_buffer;
    glGenBuffers(1, &image_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, glGetProgramResourceIndex(program_raycast, GL_SHADER_STORAGE_BLOCK, "image_block"), image_buffer);
    image = malloc(sizeof(float) * 4 * width * height);
    glBufferData(GL_SHADER_STORAGE_BUFFER, width * height * 48 * 2 + 8, NULL, GL_DYNAMIC_DRAW);
    glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_R32UI, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    //setup entities
    struct entity {
        struct vector lp, lm, ap, am;
    } entities[] = {
        {
            .lp = (struct vector){2048, 2048, -4096 * 2},
            .ap = (struct vector){0, 0, 0},
        },
        {0},
    };

    uint64_t total_frames = 0;
    float total_time = 0;
    float dt = 0;
    glfwSetTime(0);

    while (1) {
        dt = glfwGetTime() - total_time;
        total_time = glfwGetTime();
        glfwPollEvents();
        if (glfwWindowShouldClose(window)) {
            glfwDestroyWindow(window);
            glfwTerminate();
            return 0;
        }

        struct vector fm, fp, jm, jp;
        fm = mulmv(matv(mulvs(entities[0].ap, -1)), (struct vector){
            glfwGetKey(window, GLFW_KEY_D)     - glfwGetKey(window, GLFW_KEY_A),
            glfwGetKey(window, GLFW_KEY_SPACE) - glfwGetKey(window, GLFW_KEY_LEFT_CONTROL),
            glfwGetKey(window, GLFW_KEY_W)     - glfwGetKey(window, GLFW_KEY_S),
        });
        fp = mulmv(matv(mulvs(entities[0].ap, -1)), (struct vector){0, 0, 0});
        entities[0].lm = addvv(entities[0].lm, mulvs(fm, 40 * dt));
        entities[0].am = addvv(entities[0].am, mulvs(cross(fm, fp), 40 * dt));

        fm = mulmv(matv(mulvs(entities[0].ap, -1)), (struct vector){
            glfwGetKey(window, GLFW_KEY_RIGHT) - glfwGetKey(window, GLFW_KEY_LEFT),
            glfwGetKey(window, GLFW_KEY_UP)    - glfwGetKey(window, GLFW_KEY_DOWN),
            glfwGetKey(window, GLFW_KEY_E)     - glfwGetKey(window, GLFW_KEY_Q),
        });
        fp = mulmv(matv(mulvs(entities[0].ap, -1)), (struct vector){0, 0, 1});
        entities[0].lm = addvv(entities[0].lm, mulvs(fm, 0.1 * dt));
        entities[0].am = addvv(entities[0].am, mulvs(cross(fm, fp), 0.1 * dt));

        if (glfwGetKey(window, GLFW_KEY_P)) {
            entities[0].lm = (struct vector){{0, 0, 0}};
            entities[0].am = (struct vector){{0, 0, 0}};
        }

        entities[0].lp = addvv(entities[0].lp, mulvs(entities[0].lm, dt));
        entities[0].ap = addvv(entities[0].ap, mulvs(entities[0].am, dt));

        inputs.screen = (struct vector){width, height};
        inputs.prev_camera_pos = inputs.camera_pos;
        inputs.prev_camera_dir = inputs.camera_dir;
        inputs.camera_pos = entities[0].lp;
        inputs.camera_dir = matv(entities[0].ap);
        inputs.scene_pos = entities[1].lp;
        inputs.scene_dir = matv(entities[1].ap);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(inputs), &inputs);

        for (uint32_t y = 0; y < 1024; y++) {
            for (uint32_t x = 0; x < 1024; x++) {
                image[x + y * width][0] = 0;
                image[x + y * width][1] = 0;
                image[x + y * width][2] = 0;
            }
        }

        /*for (uint32_t x = 0; x < 1024; x++) {
            for (uint32_t y = 0; y < 1024; y++) {
                traverse(x, y, 1, 0, 0, 0, 4096, 0, 0);
            }
        }*/
        traverse(0, 0, 1024, 0, 0, 0, 4096, 0, 0);

        printf("%lu\n", total_frames);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, image_buffer);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, width * height * sizeof(float) * 4, &image[0]);

        /*glUseProgram(program_raycast);
        glDispatchCompute(width / 16, height / 16, 1);
        glFinish();*/

        glUseProgram(program_fragment);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glFinish();

        glfwSwapBuffers(window);
        total_frames += 1;
    }
}
