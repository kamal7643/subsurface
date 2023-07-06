#version 300 es
precision mediump float;


uniform float zNear;
uniform float zFar;
uniform sampler2D colorTexture;
// in vec2 aTex;
out vec4 outColor;

void main()
{
    // Write accumulated lighting back to framebuffer
    // with gamma correction (gamma of 2.0)
    // outColor.rgb = sqrt(storage.lighting.rgb);
    // outColor= vec4(1.0, 1.0, 1.0, 1.0);

    // Alpha lighting is unused
    // outColor.a = 1.0;
    // outColor = texture(colorTexture, aTex);
    vec2 uv = gl_FragCoord.xy/vec2(1280.0, 720.0);
    outColor =  texture(colorTexture, uv);
    // if(outColor.x == 1.0){
    //     outColor= vec4(1.0, 0.0, 0.0, 1.0);
    // }
    // else {
    //     outColor= vec4(1.0, 0.5, 0.5, 1.0);
    // }

    //outColor = vec4(uv, 1.0, 1.0);
}
