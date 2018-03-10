#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>

#include "file.h"
#include "vector.h"

static void error_callback(int error, const char* description) {
    printf("glfw error: %s\n", description);
}

int main(int argc, char* argv[]) {
    //setup glfw, window
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        printf("glfw error: failed to initialise\n");
		return 1;
    }
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    uint32_t width = mode->width;
    uint32_t height = mode->height;
    GLFWwindow* window = glfwCreateWindow(width, height, "project", NULL, NULL);
    if (!window) {
        glfwTerminate();
        printf("glfw error: failed to create window\n");
		return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    //create programs
    char* error_log = calloc(4096, sizeof(char));
    const char* shader_vertex[] = {
        "#version 450\n",
        "#line 1 0\n", load("src/header.glsl"),
        "#line 1 1\n", load("src/vertex.glsl"),
    };
    GLuint program_vertex = glCreateShaderProgramv(GL_VERTEX_SHADER, sizeof(shader_vertex) / sizeof(void*), shader_vertex);
    glGetProgramInfoLog(program_vertex, 4096, NULL, error_log);
    if (error_log[0] != 0) {
        printf("vertex:\n%s\n", error_log);
        return 1;
    }
    const char* shader_fragment[] = {
        "#version 450\n",
        "#line 1 0\n", load("src/header.glsl"),
        "#line 1 1\n", load("src/fragment.glsl"),
    };
    GLuint program_fragment = glCreateShaderProgramv(GL_FRAGMENT_SHADER, sizeof(shader_fragment) / sizeof(void*), shader_fragment);
    glGetProgramInfoLog(program_fragment, 4096, NULL, error_log);
    if (error_log[0] != 0) {
        printf("fragment:\n%s\n", error_log);
        return 1;
    }
    const char* shader_raycast[] = {
        "#version 450\n",
        "#line 1 0\n", load("src/header.glsl"),
        "#line 1 1\n", load("src/raycast.glsl"),
    };
    GLuint program_raycast = glCreateShaderProgramv(GL_COMPUTE_SHADER, sizeof(shader_raycast) / sizeof(void*), shader_raycast);
    glGetProgramInfoLog(program_raycast, 4096, NULL, error_log);
    if (error_log[0] != 0) {
        printf("raycast:\n%s\n", error_log);
        return 1;
    }
    GLuint pipeline_render;
    glGenProgramPipelines(1, &pipeline_render);
    glUseProgramStages(pipeline_render, GL_VERTEX_SHADER_BIT, program_vertex);
    glUseProgramStages(pipeline_render, GL_FRAGMENT_SHADER_BIT, program_fragment);
    GLuint pipeline_compute;
    glGenProgramPipelines(1, &pipeline_compute);
    glUseProgramStages(pipeline_compute, GL_COMPUTE_SHADER_BIT, program_raycast);

    //create uniform buffer
    struct inputs {
        struct vector screen;
        struct vector camera_pos;
        struct matrix camera_dir;
        struct vector scene_pos;
        struct matrix scene_dir;
    } inputs;
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
    GLuint octree;
    glGenBuffers(1, &octree);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, glGetProgramResourceIndex(program_raycast, GL_SHADER_STORAGE_BLOCK, "tree_block"), octree);
    struct node {
        uint32_t data, offset;
    } *nodes = (struct node*)load(argv[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size(argv[1]), nodes, GL_DYNAMIC_DRAW);

    //create image buffer
    GLuint image_buffer;
    glGenBuffers(1, &image_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, glGetProgramResourceIndex(program_raycast, GL_SHADER_STORAGE_BLOCK, "image_block"), image_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * 4 * width * height, NULL, GL_DYNAMIC_DRAW);
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

    uint32_t frames = 0;
    float dt = 0;
    float time = 0;
    glfwSetTime(time);

    while (1) {
        dt = glfwGetTime() - time;
        time = glfwGetTime();
        glfwPollEvents();
        if (glfwWindowShouldClose(window)) {
            printf("%f\n", frames / time);
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
        entities[0].lm = addvv(entities[0].lm, mulvs(fm, 100 * dt));
        entities[0].am = addvv(entities[0].am, mulvs(cross(fm, fp), 100 * dt));

        fm = mulmv(matv(mulvs(entities[0].ap, -1)), (struct vector){
            glfwGetKey(window, GLFW_KEY_RIGHT) - glfwGetKey(window, GLFW_KEY_LEFT),
            glfwGetKey(window, GLFW_KEY_UP)    - glfwGetKey(window, GLFW_KEY_DOWN),
            glfwGetKey(window, GLFW_KEY_E)     - glfwGetKey(window, GLFW_KEY_Q),
        });
        fp = mulmv(matv(mulvs(entities[0].ap, -1)), (struct vector){0, 0, 1});
        entities[0].lm = addvv(entities[0].lm, mulvs(fm, 0.1 * dt));
        entities[0].am = addvv(entities[0].am, mulvs(cross(fm, fp), 0.1 * dt));

        entities[0].lp = addvv(entities[0].lp, mulvs(entities[0].lm, dt));
        entities[0].ap = addvv(entities[0].ap, mulvs(entities[0].am, dt));

        inputs.screen = (struct vector){width, height};
        inputs.camera_pos = entities[0].lp;
        inputs.camera_dir = matv(entities[0].ap);
        inputs.scene_pos = entities[1].lp;
        inputs.scene_dir = matv(entities[1].ap);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(inputs), &inputs);

        glBindProgramPipeline(pipeline_compute);
        glDispatchCompute(width / 8, height / 8, 1);
        glBindProgramPipeline(pipeline_render);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glFinish();

        glfwSwapBuffers(window);
        frames++;
    }
}
