/* Copyright (c) 2014-2017, ARM Limited and Contributors
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge,
 * to any person obtaining a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <GLES3/gl3.h>
#include "app.h"
#include "glutil.h"
#include "timer.h"
#include "shader.h"
#include "common.h"
#include "primitives.h"
#include "meshloader.h"
#include <fstream>

//#ifndef GL_EXT_shader_pixel_local_storage
//#define GL_EXT_shader_pixel_local_storage 1
//#define GL_MAX_SHADER_PIXEL_LOCAL_STORAGE_FAST_SIZE_EXT 0x8F63
//#define GL_MAX_SHADER_PIXEL_LOCAL_STORAGE_SIZE_EXT 0x8F67
//#define GL_SHADER_PIXEL_LOCAL_STORAGE_EXT 0x8F64
//#endif

int 
    window_width,
    window_height;

Shader
    shader_prepass,
    shader_thickness,
    shader_scattering,
    shader_resolve,
    shader_opaque;

mat4 
    mat_projection,
    mat_view;

Mesh quad, cube, teapot, sphere;

const int num_lights = 2;
vec3 light_pos[num_lights] = {
    vec3(0.0), vec3(0.0)
};

vec3 light_color[num_lights] = {
    vec3(0.2, 0.8, 0.9),
    vec3(1.0, 0.4, 0.2)
};

float light_intensity[num_lights] = {
    0.6f, 0.9f
};

float light_radius[num_lights] = {
    0.1f, 0.14f
};

static float model_scale = 0.01f;

// Subsurface scattering parameters
static float s_ambient    = 0.20f; // The minimum amount of light that is transmitted
static float s_distortion = 0.07f; // Distorts the light direction vector
static float s_sharpness  = 10.0f; // Produces more focused or diffused transmittance
static float s_scale      = 3.50f; // Brightness scaling factor

// Camera parameters
static float zoom = 3.0f;
static float rot_x = -0.5f;
static float rot_y = 0.0f;
static float delta_x = 0.0f;
static float delta_y = 0.0f;

// Perspective projection parameters
static float z_near = 0.1f;
static float z_far = 15.0f;
static float fov_y = PI / 4.0f;
static float aspect_ratio = 1.0f;

bool init_app(int width, int height)
{
    window_width  = width;
    window_height = height;
    aspect_ratio = float(width) / height;

    // Check if we have support for pixel local storage
//    std::string ext = std::string((const char*)glGetString(GL_EXTENSIONS));
//    ASSERT(ext.find("GL_EXT_shader_pixel_local_storage") != std::string::npos, "This device does not support shader pixel local storage");

    string res = "/data/data/com.kamal.subsurface/files/";
    if (!shader_prepass.load_from_file(res + "prepass.vs", res + "prepass.fs") ||
        !shader_thickness.load_from_file(res + "thickness.vs", res + "thickness.fs") ||
        !shader_resolve.load_from_file(res + "resolve.vs", res + "resolve.fs") ||
        !shader_scattering.load_from_file(res + "scattering.vs", res + "scattering.fs") ||
        !shader_opaque.load_from_file(res + "opaque.vs", res + "opaque.fs"))
        return false;

    if (!shader_prepass.link() ||
        !shader_thickness.link() ||
        !shader_resolve.link() ||
        !shader_scattering.link() ||
        !shader_opaque.link())
        return false;

    sphere = gen_normal_sphere(24, 24);
    quad = gen_quad();
    cube = gen_normal_cube();

    if (!load_mesh_binary(teapot, res + "teapot.bin"))
        return false;

    mat_projection = perspective(fov_y, aspect_ratio, z_near, z_far);

    return true;
}

void free_app()
{
    shader_thickness.dispose();
    shader_resolve.dispose();
    shader_prepass.dispose();
    shader_scattering.dispose();
    shader_opaque.dispose();
}

void update_app(float dt)
{
    rot_y += 0.01f * delta_x * dt;
    rot_x += 0.01f * delta_y * dt;
    mat_view = 
        translate(0.0f, -0.2f, -zoom) *
        rotateX(rot_x) *
        rotateY(rot_y);

    float alpha = 0.5 + 0.5 * sin(get_elapsed_time() * 0.4);
    light_pos[0] = vec3(-1.0 + 2.0 * alpha, 0.0, 0.0);
    light_pos[1] = vec3(0.8 - 2.0 * alpha, 0.0, 0.0);

    float t = get_elapsed_time();
    light_intensity[0] = smoothstep(0.5f, 1.0f, t);
    light_intensity[1] = smoothstep(1.5f, 2.0f, t);
}
void render_teapot(mat4 model, bool normal = true)
{
    teapot.bind();
    uniform("model", model);
    attribfv("position", 3, 8, 0);
    if (normal) attribfv("normal", 3, 8, 5);
    glDrawElements(GL_TRIANGLES, teapot.num_indices, GL_UNSIGNED_INT, 0);
}

void render_sphere(mat4 model, bool normal = true)
{
    sphere.bind();
    uniform("model", model);
    attribfv("position", 3, 6, 0);
    if (normal) attribfv("normal", 3, 6, 3);
    glDrawElements(GL_TRIANGLES, sphere.num_indices, GL_UNSIGNED_INT, 0);
}

void render_cube(mat4 model, bool normal = true)
{
    cube.bind();
    uniform("model", model);
    attribfv("position", 3, 6, 0);
    if (normal) attribfv("normal", 3, 6, 3);
    glDrawElements(GL_TRIANGLES, cube.num_indices, GL_UNSIGNED_INT, 0);
}

void render_pass_thickness(bool second_pass)
{
    uniform("projection", mat_projection);
    uniform("view", mat_view);

    GLenum cmp = second_pass ? GL_EQUAL : GL_ALWAYS;
    bool use_albedo = !second_pass;
    bool use_normal = !second_pass;

    if (use_albedo) uniform("albedo", vec3(0.7f, 0.8f, 0.9f));
    glStencilFunc(cmp, 1, 0xFF);
    render_teapot(translate(0.5f, -0.07f, -0.9f) * scale(0.05f) * rotateY(-0.3f), use_normal);

    if (use_albedo) uniform("albedo", vec3(0.2f, 0.5f, 0.3f));
    glStencilFunc(cmp, 2, 0xFF);
    render_cube(translate(-0.3f, -0.05f, 0.1f) * scale(0.4f), use_normal);

    if (use_albedo) uniform("albedo", vec3(0.7f, 0.4f, 0.2f));
    glStencilFunc(cmp, 3, 0xFF);
    render_cube(translate(0.9f, -0.1f, -0.1f) * scale(0.35f) * rotateY(-0.3f), use_normal);

    for (int i = 0; i < num_lights; i++)
    {
        if (use_albedo) uniform("albedo", light_color[i]);
        glStencilFunc(cmp, i + 4, 0xFF);
        render_sphere(translate(light_pos[i]) * scale(light_radius[i]), use_normal);
    }
}

void render_pass_shading()
{
    // Shade translucent objects with sss algorithm
    use_shader(shader_scattering);
    uniform("zNear", z_near);
    uniform("zFar", z_far);
    uniform("top", z_near * tan(fov_y / 2.0f));
    uniform("right", aspect_ratio * z_near * tan(fov_y / 2.0f));
    uniform("invResolution", vec2(1.0f / window_width, 1.0f / window_height));
    uniform("ambient", s_ambient);
    uniform("distortion", s_distortion);
    uniform("sharpness", s_sharpness);
    uniform("scale", s_scale);

    // One fullscreen pass per light
    quad.bind();
    attribfv("position", 3, 3, 0);
    for (int i = 0; i < num_lights; i++)
    {
        uniform("lightPos",(mat_view * vec4(light_pos[i], 1.0f)).xyz());
        uniform("lightColor", light_color[i]);
        uniform("lightIntensity", light_intensity[i]);
        uniform("lightRadius", light_radius[i]);
        glDrawElements(GL_TRIANGLES, quad.num_indices, GL_UNSIGNED_INT, 0);
    }
}

void render_pass_opaque()
{
    use_shader(shader_opaque);
    uniform("projection", mat_projection);
    uniform("view", mat_view);
    uniform("lightPos0", light_pos[0]);
    uniform("lightPos1", light_pos[1]);
    uniform("lightCol0", light_color[0]);
    uniform("lightCol1", light_color[1]);
    uniform("lightInt0", light_intensity[0]);
    uniform("lightInt1", light_intensity[1]);
    render_cube(translate(0.0f, -0.5f, 0.0f) * scale(10.0f, 0.05f, 10.0f));
}

void render_pass_resolve()
{
    use_shader(shader_resolve);
    quad.bind();
    attribfv("position", 3, 3, 0);
    glDrawElements(GL_TRIANGLES, quad.num_indices, GL_UNSIGNED_INT, 0);
}

void checkFrameBufferStatus()
{
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        switch (status)
        {
            case GL_FRAMEBUFFER_UNDEFINED:
                LOGE("Framebuffer error: GL_FRAMEBUFFER_UNDEFINED");
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                LOGE("Framebuffer error: GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT");
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                LOGE("Framebuffer error: GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT");
                break;
            case GL_FRAMEBUFFER_UNSUPPORTED:
                LOGE("Framebuffer error: GL_FRAMEBUFFER_UNSUPPORTED");
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                LOGE("Framebuffer error: GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE");
                break;
            default:
                LOGE("Framebuffer error: Unknown error");
                break;
        }
    }
}


void render_app(float dt)
{
    GLuint fbo1;
    glGenFramebuffers(1, &fbo1);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo1);

    GLuint texture1;
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, window_width, window_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture1, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        __android_log_write(ANDROID_LOG_INFO, "frame buffer 1", "error");
        // Handle framebuffer creation failure
    }

    // Clearing all buffers at the beginning can lead to better performance
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
//    glEnable(GL_SHADER_PIXEL_LOCAL_STORAGE_EXT);
    glDepthMask(GL_TRUE);
    glStencilMask(0xFF);
    glClearDepthf(1.0f);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render opaque geometry and do forward lighting on it.
    // We write the ID of 0 to the stencil buffer, to seperate
    // regular geometry from translucent geometry later.
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    depth_write(true);
    depth_test(true, GL_LEQUAL);
    cull(true);
    render_pass_opaque();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    GLuint fbo2;
    glGenFramebuffers(1, &fbo2);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo2);

    GLuint texture2;
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, window_width, window_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture2, 0);

    GLuint texture21;
    glGenTextures(1, &texture21);
    glBindTexture(GL_TEXTURE_2D, texture21);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, window_width, window_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, texture21, 0);
    

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        __android_log_write(ANDROID_LOG_INFO, "frame buffer 2", "error");
        // Handle framebuffer creation failure
    }
    // In this pass we render the _closest_ object's material properties
    // to the local storage, and its ID to the stencil buffer. The ID will 
    // be used for the following pass, where we compute the thickness.
    cull(false);
    use_shader(shader_prepass);
    render_pass_thickness(false);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    GLuint fbo3;
    glGenFramebuffers(1, &fbo3);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo3);

    GLuint texture3;
    glGenTextures(1, &texture3);
    glBindTexture(GL_TEXTURE_2D, texture3);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, window_width, window_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture3, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        __android_log_write(ANDROID_LOG_INFO, "frame buffer 3", "error");
        // Handle framebuffer creation failure
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture2);
    GLuint textureLocation = glGetUniformLocation(shader_thickness.m_id, "u_Texture");
    



    // In this pass we render the same objects again, but we demand that
    // the object being rendered has the same ID as that in the stencil 
    // buffer - that is, it is in fact the closest object for that pixel.
    depth_write(false);
    depth_test(false);
    use_shader(shader_thickness);

    glUniform1i(textureLocation, 0);  // 0 represents the texture unit index
    render_pass_thickness(true);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    GLuint fbo4;
    glGenFramebuffers(1, &fbo4);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo4);

    GLuint texture4;
    glGenTextures(1, &texture4);
    glBindTexture(GL_TEXTURE_2D, texture4);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, window_width, window_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture4, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        __android_log_write(ANDROID_LOG_INFO, "frame buffer 4", "error");
        // Handle framebuffer creation failure
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture2);

     glActiveTexture(GL_TEXTURE2);
     glBindTexture(GL_TEXTURE_2D, texture3);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, texture21);

    GLuint loc1 = glGetUniformLocation(shader_scattering.m_id, "texture1");
    GLuint loc2 = glGetUniformLocation(shader_scattering.m_id, "texture2");
    GLuint loc3 = glGetUniformLocation(shader_scattering.m_id, "texture3");
    GLuint loc4 = glGetUniformLocation(shader_scattering.m_id, "texture21");

    // Apply subsurface scattering and front-lighting to geometry
    // Translucent objects have an ID that is >= 1. We only want to
    // shade these, so we stencil _out_ fragments with an ID = 0.
    glStencilFunc(GL_LEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    use_shader(shader_scattering);
    //glUniform1i(textureLocation, 0);
    // __android_log_write(ANDROID_LOG_INFO, "loc1", std::to_string(loc1).c_str());
    glUniform1i(loc1, 0);
    glUniform1i(loc2, 1);
    glUniform1i(loc3, 2);
    glUniform1i(loc4, 3);

    render_pass_shading();
    glBindFramebuffer(GL_FRAMEBUFFER, 0); 


    GLuint fbo5;
    glGenFramebuffers(1, &fbo5);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo5);

    GLuint texture5;
    glGenTextures(1, &texture5);
    glBindTexture(GL_TEXTURE_2D, texture5);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, window_width, window_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture5, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        __android_log_write(ANDROID_LOG_INFO, "frame buffer 4", "error");
        // Handle framebuffer creation failure
    }
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture4);
    textureLocation = glGetUniformLocation(shader_thickness.m_id, "u_Texture");
    // Write back lighting for _all_ the pixels!
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    use_shader(shader_resolve);
    glUniform1i(textureLocation, 0);
    render_pass_resolve();
    glDisable(GL_STENCIL_TEST);
//    glDisable(GL_SHADER_PIXEL_LOCAL_STORAGE_EXT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // These are no longer needed, so we don't bother writing back to framebuffer.
    //check each frame buffer
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo5);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, window_width, window_height, 0, 0, window_width, window_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    GLenum to_invalidate[] = { GL_DEPTH, GL_STENCIL };
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, to_invalidate);
}

static bool dragging = false;
static float last_x = 0.0f;
static float last_y = 0.0f;
void on_pointer_down(float x, float y)
{
    if (!dragging)
    {
        dragging = true;
        last_x = x;
        last_y = y;
    }
    else
    {
        delta_x = x - last_x;
        delta_y = y - last_y;
    }
}

void on_pointer_up(float x, float y)
{
    dragging = false;
    delta_x = 0.0f;
    delta_y = 0.0f;
}