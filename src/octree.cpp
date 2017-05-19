#include <vector>
#include <stdint.h>

#include "morton.cpp"

namespace octree {
    template <typename P> class octree {
        public:
            struct octree_node {
                uint32_t children_index, data;
            };
            std::vector<struct octree_node> nodes;
            float res;
            const uint32_t max_level = 32;

            octree(float resolution) {
                res = resolution;
                struct octree_node n;
                n.children_index = 0;
                nodes.push_back(n);
            };
            morton::pos world_to_octree(P point) {
                return (morton::pos) {
                    .x = (uint32_t)((int32_t)(point.x / res)) + (1 << 31),
                    .y = (uint32_t)((int32_t)(point.y / res)) + (1 << 31),
                    .z = (uint32_t)((int32_t)(point.z / res)) + (1 << 31),
                };
            };
            P octree_to_world(morton::pos pos) {
                P point;
                point.x = ((float)((int32_t)(pos.x - (1 << 31)))) * res;
                point.y = ((float)((int32_t)(pos.y - (1 << 31)))) * res;
                point.z = ((float)((int32_t)(pos.z - (1 << 31)))) * res;
                return point;
            };

            void insert() {
                P point;
                morton::pos pos = world_to_octree(point);
                __uint128_t index = morton::encode(pos);
                uint32_t level = max_level;
                uint32_t stack[max_level + 1];
                stack[max_level] = 0;
                struct octree_node* node = &(nodes[stack[level]]);
                while (level > 0) {
                    if (node->children_index == 0) {
                        node->children_index = nodes.size();
                        for (uint32_t i = 0; i < 8; i++) {
                            struct octree::octree_node n;
                            n.children_index = 0;
                            nodes.push_back(n);
                        }
                        node = &(nodes[stack[level]]);
                    }
                    level -= 1;
                    uint32_t child_index = (index >> 3 * level) & 7;
                    stack[level] = node->children_index + child_index;
                    node = &(nodes[stack[level]]);
                }
            };

            //modify the input values while traversing
            uint32_t traverse2(__uint128_t& index, uint32_t& level, uint32_t stack[33]) {
                struct octree_node* node = &(nodes[stack[level]]);
                while (level > 0 && node->children_index != 0) {
                    level -= 1;
                    uint32_t child_index = (index >> 3 * level) & 7;
                    stack[level] = node->children_index + child_index;
                    node = &(nodes[stack[level]]);
                }
                if (level == 0) {
                    return stack[level];
                } else {
                    return 0;
                }
            };
    };
}
