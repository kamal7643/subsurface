

#include "meshloader.h"
#include "glutil.h"
#include "common.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
using std::stringstream;

/*
The .bin format is simply a memory dump of the model data
as it resides in the GPU. For instance, the teapot mesh has
3xPosition, 2xTexel, 3xNormal and has N vertices.
This gives N * 8 attributes, which is the first part of
the .bin file. The remaining data is the element index data.
*/
bool load_mesh_binary(Mesh &mesh, string path)
{
    std::ifstream file(path);
    if (!file.is_open())
        return false;
    std::stringstream bindata;
    bindata << file.rdbuf();
    file.close();
    int attrib_count; bindata >> attrib_count;
    float *vertex_data = new float[attrib_count];
    for (int i = 0; i < attrib_count; i++)
        bindata >> vertex_data[i];
    int index_count; bindata >> index_count;
    uint32 *index_data = new uint32[index_count];
    for (int i = 0; i < index_count; i++)
        bindata >> index_data[i];

    mesh.vertex_buffer = gen_buffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW, attrib_count * sizeof(float), vertex_data);
    mesh.index_buffer = gen_buffer(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, index_count * sizeof(uint32), index_data);
    delete[] vertex_data;
    delete[] index_data;
    mesh.num_indices = index_count;
    return true;
}