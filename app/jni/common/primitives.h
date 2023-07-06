

#ifndef PRIMITIVES_H
#define PRIMITIVES_H
#include "glutil.h"
#include "shader.h"

struct Mesh
{
    GLuint vertex_buffer;
    GLuint index_buffer;
    int num_indices;
    int num_vertices;

    void dispose();
    void bind();
};

Mesh gen_normal_cube();
Mesh gen_normal_plane();
Mesh gen_quad();
Mesh gen_normal_sphere(int t_sample, int s_samples);

#endif