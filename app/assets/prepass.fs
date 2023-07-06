#version 300 es
precision highp float;

layout (location =0) out vec4 data;
layout (location =1) out vec4 data2;


uniform vec3 albedo;
in vec4 vPosition;
in vec3 vNormal;

void main()
{
    vec3 n = normalize(vNormal);
    // storage.lighting = vec4(0.0);
    // storage.minMaxDepth = vec2(-vPosition.z, -vPosition.z);
    // storage.albedo.rgb = albedo;
    // storage.albedo.a = sign(n.z);
    // storage.normalXY = n.xy;
    
    data = vec4(albedo, n.y);
    data = vec4(-vPosition.z, sign(n.z), n.x, n.y);
    data2.xyz = albedo;
    //data2 = vec4(uv, 1.0, 1.0); // testing
    data2.w = sign(n.z);
}
