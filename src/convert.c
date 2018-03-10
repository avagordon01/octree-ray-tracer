#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

struct __attribute__((packed)) node_old {
    uint64_t data;
    uint64_t base;
    uint8_t offsets[8];
};
struct __attribute__((packed)) data_old {
    uint64_t morton;
    float c[3];
    float n[3];
};

struct __attribute__((packed)) node_new {
    uint32_t data;
    uint32_t offset;
};

struct node_old* input_nodes;
struct data_old* payload_nodes;
struct node_new* output_nodes;
uint64_t output_index;

//tree size   8   64  512 4096
//max level  12    6    4    3

const uint32_t treesize = 64;
const uint32_t maxlevel = 6;
struct node_new stack[maxlevel + 1][treesize] = {{{0}}};
uint32_t pointer[maxlevel + 1] = {0};

void insert(uint32_t data, uint32_t level) {
    uint32_t value = 0;
    if (data != 0) {
        struct data_old d = payload_nodes[data];
        value =
            ((uint8_t)((d.n[0] + 1) / 2 * 256) << 0) |
            ((uint8_t)((d.n[1] + 1) / 2 * 256) << 8) |
            ((uint8_t)((d.n[2] + 1) / 2 * 256) << 16);
    }
    stack[level][pointer[level]] = (struct node_new){.data = value, .offset = -1};
    pointer[level] += 1;
    while (pointer[level] == treesize) {
        uint32_t combine = 1;
        for (int i = 0;i < treesize;i++) {
            combine = combine &&
                stack[level][0].data == stack[level][i].data &&
                stack[level][i].offset == -1;
        }
        if (combine) {
            stack[level + 1][pointer[level + 1]] = (struct node_new){.data = stack[level][0].data, .offset = -1};
        } else {
            stack[level + 1][pointer[level + 1]] = (struct node_new){.data = 0, .offset = output_index};
            for (int i = 0;i < treesize;i++) {
                output_nodes[output_index] = stack[level][i];
                output_index += 1;
            }
        }
        pointer[level + 1] += 1;
        pointer[level] = 0;
        level += 1;
    }
}

void traverse(uint32_t input_index, uint32_t level) {
    struct node_old n = input_nodes[input_index];
    if (*((uint64_t*)&n.offsets) == (uint64_t)-1) {
        uint32_t num_voxels = 1 << (3 * level);
        uint32_t l = level * 3 / (uint32_t)log2(treesize);
        for (int i = 0;i < num_voxels / (uint32_t)pow(treesize, l);i++) {
            insert(n.data, l);
        }
        /*for (int i = 0;i < num_voxels;i++) {
            insert(n.data, 0);
        }*/
    } else {
        for (int i = 0;i < 8;i++) {
            if (n.offsets[i] == (uint8_t)-1) {
                uint32_t num_voxels = 1 << (3 * (level - 1));
                uint32_t l = (level - 1) * 3 / (uint32_t)log2(treesize);
                for (int j = 0;j < num_voxels / (uint32_t)pow(treesize, l);j++) {
                    insert(n.data, l);
                }
                /*for (int j = 0;j < num_voxels;j++) {
                    insert(n.data, 0);
                }*/
            } else {
                traverse(n.base + n.offsets[i], level - 1);
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("expecting filename\n");
        return 1;
    }
    char filename_nodes[80];
    strcpy(filename_nodes, argv[1]);
    strcat(filename_nodes, "nodes");
    FILE* input_file = fopen(filename_nodes, "rb");
    if (!input_file) {
        printf("couldnt open input file\n");
        return 1;
    }
    fseek(input_file, 0, SEEK_END);
    uint32_t input_length = (uint32_t)ftell(input_file);
    fseek(input_file, 0, SEEK_SET);
    input_nodes = (struct node_old*)malloc(input_length);
    if (!input_nodes) {
        printf("couldnt allocate input_nodes\n");
        return 1;
    }
    fread(input_nodes, 1, input_length, input_file);
    fclose(input_file);

    char filename_payload[80];
    strcpy(filename_payload, argv[1]);
    strcat(filename_payload, "data");
    FILE* payload_file = fopen(filename_payload, "rb");
    if (!payload_file) {
        printf("couldnt open payload file\n");
        return 1;
    }
    fseek(payload_file, 0, SEEK_END);
    uint32_t payload_length = (uint32_t)ftell(input_file);
    fseek(payload_file, 0, SEEK_SET);
    payload_nodes = (struct data_old*)malloc(payload_length);
    if (!payload_nodes) {
        printf("couldnt allocate payload nodes\n");
        return 1;
    }
    fread(payload_nodes, 1, payload_length, payload_file);
    fclose(payload_file);

    output_nodes = (struct node_new*)malloc(sizeof(struct node_new) * 100000000);
    if (!output_nodes) {
        printf("couldnt allocate output_nodes\n");
        return 1;
    }

    printf("starting...\n");
    output_index = 1;
    traverse(input_length / sizeof(struct node_old) - 1, 12);
    output_nodes[0] = stack[maxlevel][0];
    printf("finishing...\n");

    FILE* output_file = fopen(argv[2], "wb");
    fwrite(output_nodes, sizeof(struct node_new), output_index, output_file);
    fclose(output_file);
    return 0;
}
