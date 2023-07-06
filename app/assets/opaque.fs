#version 300 es

precision highp float;

layout (location =0) out vec4 lighting;


in vec4 vPosition;
in vec3 vNormal;

uniform vec3 lightPos0;
uniform vec3 lightPos1;
uniform vec3 lightCol0;
uniform vec3 lightCol1;
uniform float lightInt0;
uniform float lightInt1;

vec3 lambert(vec3 P, vec3 Lp, vec3 N, vec3 Ldiff, float Li)
{
    vec3 L = Lp - P;
    float r = length(L);
    L /= r;
    float NdotL = max(dot(N, L), 0.0);
    float att = Li / (1.0 + 0.4 * r * r);
    return att * NdotL * Ldiff;
}

void main()
{
    vec3 P = vPosition.xyz;
    vec3 N = normalize(vNormal);

    /* Procedural checkerpatterns! :D

    1: Take your floor coordinates modulo some size.
    This will make the coordinates loop in the range [0, size).

    2: Divide the looped coordinates by the size, to normalize
    to the range [0, 1).

    3: Create smooth transitions between edges along both axes.
    The parameter d determines how smooth the transition is.
     ______ ______
    |      |      |   00 ~ fx = 0 and fz = 0
    |  00  |  10  |   01 ~ fx = 0 and fz = 1
    |______|______|   10 ~ fx = 1 and fz = 0
    |      |      |   11 ~ fx = 1 and fz = 1
    |  01  |  11  |
    |______|______|

    Here: fx = 0 when x is on the left, and fx = 1 when x is on the right.
    It is a blend when x is in the middle.

    4: Select out the pattern. We want white where fx and fy are both 0
    or both 1. That is, when fx * fz + (1.0 - fx) * (1.0 - fz) is 1.
    */
    float d = 0.03;
    float size = 0.35;
    float fx = mod(vPosition.x / size, 2.0);
    fx = smoothstep(1.0 - d, 1.0, fx) - smoothstep(2.0 - d, 2.0, fx);
    float fz = mod(vPosition.z / size, 2.0);
    fz = smoothstep(1.0 - d, 1.0, fz) - smoothstep(2.0 - d, 2.0, fz);
    float pattern = fx * fz + (1.0 - fx) * (1.0 - fz);

    // Make floor dark when stuff is above it
    float ao = 1.0 - exp2(-1.0 * dot(vPosition.xz, vPosition.xz));

    vec3 color = vec3(0.0);
    color += lambert(P, lightPos0, N, lightCol0, lightInt0);
    color += lambert(P, lightPos1, N, lightCol1, lightInt1);
    color *= (pattern + 0.2 * (1.0 - pattern)) * ao;

    lighting = vec4(color, 1.0);
    // storage.lighting.rgb = color;
    // storage.lighting.a = 1.0;
}
