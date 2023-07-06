#version 300 es


precision highp float;
out vec4 data;

uniform sampler2D u_Texture;
uniform vec3 albedo;
in vec4 vPosition;
in vec3 vNormal;

void main()
{
    vec2 uv = gl_FragCoord.xy/vec2(1280.0, 720.0);
    //float depth = -vPosition.z;
    float depth = max(-vPosition.z, texture(u_Texture, uv).x);
    vec3 v = albedo;
    data = vec4(depth, v);

    //data = texture(u_Texture, uv);// for testing
    //data = vec4(uv, 1.0, 1.0);
}
