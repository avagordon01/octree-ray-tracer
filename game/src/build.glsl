layout(local_size_x = 1, local_size_y = 1) in;

//return the morton de-interlaced of a vector to find the xyz coordinates from the morton coordinate
uvec3 shrink(uvec3 x) {
    x &= 0x09249249;
    x = (x ^ (x >>  2)) & 0x030c30c3;
    x = (x ^ (x >>  4)) & 0x0300f00f;
    x = (x ^ (x >>  8)) & 0xff0000ff;
    x = (x ^ (x >> 16)) & 0x000003ff;
    return x;
};

//generate a voxel data value from the xyz coordinates
uint density(uint x) {
    return uint(noise(vec4(shrink(uvec3(x) / uvec3(1, 2, 4)), 0) / 64) > 0.5);
    //return uint(length(vec3(shrink(uvec3(x) / uvec3(1, 2, 4))) / 64 - 0.5) <= 0.5);
};

void main() {
    node stack[32][8];
    uint pointer[32] = uint[](
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    );
    uint morton = 0;
    uint index = 1;
    uint layer = 0;
    uint start;
    uint d_start;
    uint limit;
    uint length;

    uint maxlevel = 6;
    while (morton < exp2(3 * maxlevel)) {
        /*start = morton;
        layer = uint(findLSB(start) / 3) * 3;
        limit = (start & ~(uint(7) << layer)) + (uint(8) << layer);
        if (start == 0)
            limit = -1;
        d_start = density(start);
        morton += 1;
        while (density(morton) == d_start && morton < limit && morton < 64 * 64 * 64) {
            morton += 1;
        }
        length = morton - start;
        layer = 0;
        while (length > 0) {
            while ((length & 7) > 0) {
                stack[layer][pointer[layer]] = node(d_start, -1);
                pointer[layer] += 1;
                length -= 1;
            }
            layer += 1;
            length >>= 3;
        }*/
        //insert 8 nodes into lowest level
        layer = 0;
        stack[layer][0] = node(density(morton + 0), -1);
        stack[layer][1] = node(density(morton + 1), -1);
        stack[layer][2] = node(density(morton + 2), -1);
        stack[layer][3] = node(density(morton + 3), -1);
        stack[layer][4] = node(density(morton + 4), -1);
        stack[layer][5] = node(density(morton + 5), -1);
        stack[layer][6] = node(density(morton + 6), -1);
        stack[layer][7] = node(density(morton + 7), -1);
        pointer[layer] += 8;
        morton += 8;
        //combination stage
        while (pointer[layer] == 8) {
            //if the nodes are the same and can be combined
            if (
                stack[layer][0].data == stack[layer][1].data &&
                stack[layer][0].data == stack[layer][2].data &&
                stack[layer][0].data == stack[layer][3].data &&
                stack[layer][0].data == stack[layer][4].data &&
                stack[layer][0].data == stack[layer][5].data &&
                stack[layer][0].data == stack[layer][6].data &&
                stack[layer][0].data == stack[layer][7].data &&
                stack[layer][0].offset == -1 &&
                stack[layer][1].offset == -1 &&
                stack[layer][2].offset == -1 &&
                stack[layer][3].offset == -1 &&
                stack[layer][4].offset == -1 &&
                stack[layer][5].offset == -1 &&
                stack[layer][6].offset == -1 &&
                stack[layer][7].offset == -1
            ) {
                //combine 8 nodes into 1 node at the next level
                stack[layer + 1][pointer[layer + 1]] = node(stack[layer][0].data, -1);
                pointer[layer + 1] += 1;
                pointer[layer] -= 8;
                layer += 1;
            } else {
                //output 8 nodes into array and create pointer node at the next level
                nodes[index + 0] = stack[layer][0];
                nodes[index + 1] = stack[layer][1];
                nodes[index + 2] = stack[layer][2];
                nodes[index + 3] = stack[layer][3];
                nodes[index + 4] = stack[layer][4];
                nodes[index + 5] = stack[layer][5];
                nodes[index + 6] = stack[layer][6];
                nodes[index + 7] = stack[layer][7];
                stack[layer + 1][pointer[layer + 1]] = node(0, index);
                pointer[layer + 1] += 1;
                pointer[layer] -= 8;
                layer += 1;
                index += 8;
            }
        }
    }
    //output the root node into array
    nodes[0] = stack[layer][0];
}
