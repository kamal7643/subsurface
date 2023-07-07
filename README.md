# SUBSURFACE SCATTERING IN MOBILES

Solution : `https://github.com/ARM-software/opengl-es-sdk-for-android/tree/master/samples/advanced_samples/Translucency`
This solution had a ARM specific dependency. we will try to remove that and make it cross plateform.

Implementation : `link`

## Implementation

1. Remove extension checking code block
    `
    // Check if we have support for pixel local storage
    std::string ext = std::string((const char*)glGetString(GL_EXTENSIONS));
    ASSERT(ext.find("GL_EXT_shader_pixel_local_storage") != std::string::npos, "This device does not support shader pixel local storage");
    `

    Inside shaders 
    `#extension GL_EXT_shader_pixel_local_storage : require`

2. Usages of frame buffers and texture
    Create frame buffer object
    Bind frame buffer object
    Create texture/s 
    Bind texture/s to frame buffer
    perform rendering

    in next pass we can use those textures as input for next frame buffers by connecting them.

3. Code for prepass pass
    old code : ```
    cull(false);
    use_shader(shader_prepass);
    render_pass_thickness(false);
    ```

    new code : ```
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


    // Set the list of draw buffers
    GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, drawBuffers);

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
    ```

![Screenshot](/results/app.png)