#include <stdio.h>
#include <stdint.h>
#include <cstdlib>
#include <math.h>
#include <iostream>
#include <queue>
#include <vector>
#include <algorithm>

#include "octree.cpp"

#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>
typedef OpenMesh::PolyMesh_ArrayKernelT<> MyMesh;
MyMesh mesh;

void render_faces() {
    const uint32_t width = 1920;
    const uint32_t height = 1200;
    struct colour {
        uint8_t r, g, b;
    };
    struct colour* atlas = (struct colour*)calloc(height * width, sizeof(struct colour));
    for (MyMesh::FaceIter face = mesh.faces_begin(); face != mesh.faces_end(); face++) {
        struct colour surface_colour = (struct colour){
            (uint8_t)rand(),
                (uint8_t)rand(),
                (uint8_t)rand(),
        };
        OpenMesh::Vec2f face_min(+INFINITY, +INFINITY);
        OpenMesh::Vec2f face_max(-INFINITY, -INFINITY);
        for (MyMesh::FaceHalfedgeIter halfedge = mesh.fh_iter(*face); halfedge.is_valid(); halfedge++) {
            OpenMesh::Vec3f pos = mesh.point(mesh.to_vertex_handle(*halfedge)) * OpenMesh::Vec3f(width, height, 0);
            if (pos[0] < face_min[0]) face_min[0] = pos[0];
            if (pos[1] < face_min[1]) face_min[1] = pos[1];
            if (pos[0] > face_max[0]) face_max[0] = pos[0];
            if (pos[1] > face_max[1]) face_max[1] = pos[1];
        }
        for (uint32_t y = floor(face_min[1]); y < ceil(face_max[1]); y++) {
            for (uint32_t x = floor(face_min[0]); x < ceil(face_max[0]); x++) {
                OpenMesh::Vec2f pos((float)x / width, (float)y / height);
                bool inside = true;
                MyMesh::FaceHalfedgeIter halfedge;
                for (halfedge = mesh.fh_iter(*face); halfedge.is_valid(); halfedge++) {
                    OpenMesh::Vec3f p0 = mesh.point(mesh.to_vertex_handle(*halfedge));
                    OpenMesh::Vec3f p1 = mesh.point(mesh.to_vertex_handle(mesh.next_halfedge_handle(*halfedge)));
                    if ((pos[0] - p0[0]) * (p1[1] - p0[1]) - (pos[1] - p0[1]) * (p1[0] - p0[0]) < 0) {
                        inside = false;
                    }
                }
                if (inside) {
                    atlas[y * width + x] = surface_colour;
                }
            }
        }
    }
    FILE* f = fopen("out.ppm", "w");
    fprintf(f, "P6\n%i %i\n255\n", width, height);
    fwrite((uint8_t*)atlas, width * height, sizeof(struct colour), f);
    fclose(f);
}

void traverse() {
    MyMesh frustum_mesh;
    MyMesh::Vec3f frustum_origin;
    MyMesh::FaceHandle frustum_face;
    struct node octree_nodes[];
    uint32_t octree_pos[3];
    uint32_t octree_level;
    uint32_t octree_maxlevel;
    uint32_t octree_node;
    //TODO optimisation, flag if frustum direction is only in a single octant, gets passed directly to children if true
    //TODO optimisation, combine input faces on a node, and "flip" them into the output faces
    //TODO optimisation, could have a uint32 mesh instead of floats...
    //TODO optimisation, traverse breadth first, so we can join adjacent frustum faces
    //TODO the perfect loop would be over the nodes with input faces, in a breadth first manner
    //probably the closest fast loop would be over the faces, checking the iteration number of the face (if it is an input or output of this iteration)
    //TODO with this loop it wont be easy to pass the stack around between loop iterations, because its breadth first
    //find all the adjacent faces that are input faces on the same node, combine them
    //"backproject" the remaining corners of the node onto the combined face
    //maybe just delete the old ones and insert new ones?
    //split the combined face into the output face, move the vertices to their new positions
    //1-3 input faces, 0-6 output faces
    it could be difficult to keep all of the faces in sync so they can be merged
    maybe this is fixed with clever keeping track of the iteration count for each face
    if iteration count is just the number of steps from their first face, then it is ok
    also need to traverse from the corner outwards somehow? z order would work, so long as the corner is at the origin
    or just loop over the neighbour faces outwards from the corner
    could still pass the stack around by going neighbour to neighbour?
    going in z order would be great for passing the stack around, perfect even, but we dont have a fast method for getting the mesh faces from a xyz position
    with occlusion, its not perfect, you could have overlapping triangles but if they are rendered at the correct depth it doesnt matter
    also it doesnt matter because rendering half a face or the whole face makes no difference to the processing
    if (nodes[node].data != 0) {
        //TODO once a face has been coloured, remove it from the mesh, add it to the "final" mesh
        mesh.set_color(face, MyMesh::Color(0, 1, 1));
        return;
    }
    if (nodes[node].children_index != 0) {
        //TODO split the frustum face into four if the octree node has children
    }
    for (uint32_t out_face = 0; out_face < 6; out_face++) {
        //if out_face is facing away from the frustum
        if () {
            continue;
        }
        //project the frustum_face onto the out_face
        for (MyMesh::FaceVertexIter vertex = mesh.fv_iter(frustum_face); vertex.is_valid(); vertex++) {
            mesh.point(vertex);
            //projection
        }
        //clip the frustum_face with the out_face, discard the outside parts
        mesh.split_edge();
        mesh.insert_edge();
        //if the resulting face is small
        if () {
            continue;
        }
        //traverse the neighbour node, joined by the out_face
        morton::increment();
        morton::common_ancestor();
        octree::traverse();
        traverse();
    }
}

int main() {
    return 0;
}
