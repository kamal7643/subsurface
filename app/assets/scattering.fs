#version 300 es
precision mediump float;

out vec4 data;

// Light properties
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float lightIntensity;
uniform float lightRadius;

// Scattering parameters
uniform float ambient;
uniform float distortion;
uniform float sharpness;
uniform float scale;

// Position reconstruction parameters
uniform float zNear;
uniform float zFar;
uniform float top;
uniform float right;
uniform vec2 invResolution;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D texture3;
uniform sampler2D texture4;//albedo

// Subsurface scattering shading
// Based on "Approximating Translucency for a Fast, Cheap and Convincing Subsurface Scattering Look" and
// http://colinbarrebrisebois.com/2012/04/09/approximating-translucency-revisited-with-simplified-spherical-gaussian/
// by Colin Barré-Brisebois.
float sss(vec3 P, vec3 L, vec3 V, vec3 N, float r, float thickness)
{
    vec3 Lt = -(L + N * distortion);

    // This approximates the pow call with a cheaper call to exp2
    // Corresponds to doing pow(clamp(dot(V, -Lt), 0.0, 1.0), sharpness) * scale
    const float invln2 = 1.4427;
    float VdotL = exp2(-sharpness * invln2 * (1.0 - max(dot(V, Lt), 0.0))) * scale;

    // Sum up contributions and attenuate by thickness
    return (VdotL + ambient) * clamp(1.0 - thickness * 0.5, 0.0, 1.0);
}

vec3 lighting(vec3 P, vec3 Lp, vec3 N, vec3 V, float thickness, float Li, vec3 Ldiff, vec3 Cdiff)
{
    // Compute distance to light from point being shaded
    vec3 L = Lp - P;
    float r = length(L);

    // Normalize to get direction
    L /= r;

    // Apply Blinn-Phong shading model
    vec3 H = normalize(V + L);
    float specular = clamp(6.0 * exp2(-128.0 * (1.0 - max(dot(N, H), 0.0))), 0.0, 1.0);
    float NdotL = max(dot(N, L), 0.0);

    // When light is close enough we use the distance to the light directly,
    // instead of the thickness - which would be an overestimate of the distance
    // to the light.
    thickness = min(thickness, r);
    float Is = sss(P, L, V, N, r, thickness);
    
    // Light effect diminishes as it gets further away, so we
    // also attenuate by the distance to the light, using somewhat
    // arbitrarily chosen scaling factors.
    float attenuation = 10.0 * lightRadius * Li / (1.0 + 0.1 * r * r);
    return attenuation * (Is + specular + NdotL) * Ldiff * Cdiff;
}

void main()
{
    // storage vecs
    vec2 xy = gl_FragCoord.xy/vec2(1280.0, 720.0);
    //vec4 storagelighting = texture(texture1, xy);
    //vec2 minMaxDepth = vec2(texture(texture2, xy).x, texture(texture3, xy).x);
    //vec4 storagealbedo = vec4(0.0, 0.0, 0.0, 0.0);
    //vec2 normalXY = texture(texture2, xy).zw;

    vec4 color1 = texture(texture1, xy);
    vec4 color2 = texture(texture2, xy);
    vec4 color3= texture(texture3, xy);
    vec4 color4 = texture(texture4, xy);

    vec4 storagelighting = color1;
    vec2 minMaxDepth = vec2(color2.x, color3.x);
    vec4 storagealbedo =color4;//vec4(0.7, 0.4, 0.2, color2.y);//color4;// something is wrong
     vec2 normalXY = vec2(color2.zw );

    // privious code 
    /*
    
    float depth = storage.minMaxDepth.x;
    float depthNormalized = clamp((depth - zNear) / (zFar - zNear), 0.0, 1.0);

    // Reconstruct view-space position from depth
    vec2 uv = vec2(-1.0) + 2.0 * gl_FragCoord.xy * invResolution;
    vec2 frustum = vec2(right, top);
    vec3 P = vec3(uv * frustum * (1.0 + (zFar / zNear - 1.0) * depthNormalized), -depth);
    vec3 V = -normalize(P);

    // Reconstruct view-space normal
    vec3 N = vec3(storage.normalXY, 0.0);
    float nzsign = storage.albedo.a;
    N.z = nzsign * sqrt(1.0 - min(dot(N.xy, N.xy), 1.0));

    // Additive lighting
    vec3 albedo = storage.albedo.rgb;
    float thickness = storage.minMaxDepth.y - storage.minMaxDepth.x;
    storage.lighting.rgb += lighting(P, lightPos, N, V, thickness, lightIntensity, lightColor, albedo);
    */

    // new code
    float depth = minMaxDepth.x;
    float depthNormalized = clamp((depth -zNear)/(zFar-zNear), 0.0, 1.0);

    vec2 uv =vec2(-1.0) + 2.0 * gl_FragCoord.xy * invResolution;
    vec2 frustum = vec2(right, top);
    vec3 P = vec3(uv*frustum*(1.0 +(zFar/zNear-1.0)*depthNormalized), -depth);
    vec3 V = -normalize(P);

    vec3 N = vec3(normalXY, 0.0);
    float nzsign = storagealbedo.a;
    N.z = nzsign*sqrt(1.0-min(dot(N.xy, N.xy), 1.0));

    vec3 albedo = storagealbedo.rgb;
    float thickness = minMaxDepth.y - minMaxDepth.x;
    data.xyz = storagelighting.xyz + lighting(P, lightPos, N, V, thickness, lightIntensity, lightColor, albedo);
    //data.xyz = vec3(uv, 1.0);
    //data = texture(texture4, xy);
}
