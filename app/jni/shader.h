

#ifndef SHADER_H
#define SHADER_H
#include "matrix.h"
#include "common.h"
#include <unordered_map>

class Shader
{
public:
GLuint m_id;
    Shader();

    bool load_from_src(const string *sources, GLenum *types, int count);
    bool load_from_src(string vs_src, string fs_src);
    bool load_from_file(const string *paths, GLenum *types, int count);
    bool load_from_file(string vs_path, string fs_path);
    bool link();
    void dispose();

    void use();
    void unuse();

    GLint get_uniform_location(string name);
    GLint get_attribute_location(string name);

    void set_attribfv(string name, GLsizei num_components, GLsizei stride, GLsizei offset);
    void unset_attrib(string name);

    void set_uniform(string name, const mat4 &v);
    void set_uniform(string name, const vec4 &v);
    void set_uniform(string name, const vec3 &v);
    void set_uniform(string name, const vec2 &v);
    void set_uniform(string name, double v);
    void set_uniform(string name, float v);
    void set_uniform(string name, int v);
    void set_uniform(string name, unsigned int v);
private:
    std::unordered_map<string, GLint> m_attributes;
    std::unordered_map<string, GLint> m_uniforms;
    
    std::vector<GLuint> m_shaders;
};

#endif