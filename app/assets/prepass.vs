#version 300 es

in vec3 position;
in vec3 normal;

out vec4 vPosition;
out vec3 vNormal;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    vNormal = (view * model * vec4(normal, 0.0)).xyz;
    vPosition = view * model * vec4(position, 1.0);
    gl_Position = projection * vPosition;
}
