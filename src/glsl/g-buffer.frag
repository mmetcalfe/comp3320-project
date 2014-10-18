#version 330 core

/*
 * G-Buffer format:
 * |--------+--------+--------+--------|--------|
 * |   R8   |   G8   |   B8   |   A8   | attach |
 * |--------+--------+--------+--------|--------|
 * |          depth           | stencil|   DS   |
 * |--------+--------+--------+--------|--------|
 * | normal x (FP16) | normal y (FP16) |   RT0  | outNormal
 * |--------+--------+--------+--------|--------|
 * |      diffuse albedo      | rough  |   RT1  | outAlbedoRoughness
 * |--------+--------+--------+--------|--------|
 * |      env map col         | spec in|   RT2  | outEnvMapColSpecIntensity
 * |--------+--------+--------+--------|--------|
 * |                          |        |   RT3  |
 * |--------+--------+--------+--------|--------|
 */

in vec2 Texcoord;
in vec4 eyeSpacePosition;
in vec3 eyeSpaceNormal;

// G-Buffer outputs:
out vec4 outNormal;
out vec4 outAlbedoRoughness;
out vec4 outEnvMapColSpecIntensity;

uniform mat4 modelViewInverse;
uniform mat4 viewInverse;

// Material uniforms
uniform vec3 colDiffuse;
uniform vec3 colSpecular;
uniform float shininess;
//uniform float shininessStrength;
uniform samplerCube texEnvironmentMap;
uniform sampler2D texDiffuse;

float phong(in vec3 incident, in vec3 reflection, in float shininess) {
    return pow(clamp(dot(-incident, reflection), 0, 1), shininess);
}

void main() {
    // Face normal in eye space:
    vec3 normal = normalize(eyeSpaceNormal.xyz);
    vec3 incident = normalize(eyeSpacePosition.xyz);

    // Normal:
    outNormal.xy = normal.xy;

    // Environment map reflection:
    outEnvMapColSpecIntensity.rgba = vec4(0, 0, 0, 0);

    if (shininess > 0) {
        vec3 viewReflect = reflect(incident, normal);
//        float phongViewSpecular = phong(incident, viewReflect, shininess);
        vec4 sampleCoord = viewInverse * vec4(viewReflect, 0);
        sampleCoord = vec4(sampleCoord.x, sampleCoord.z, -sampleCoord.y, 1);
        vec4 reflectCol = texture(texEnvironmentMap, sampleCoord.xyz);
//        outEnvMapColSpecIntensity.rgb = reflectCol.rgb * colSpecular * phongViewSpecular;

        // Based on: http://en.wikibooks.org/wiki/GLSL_Programming/Unity/Specular_Highlights_at_Silhouettes
        float fresnelFactor = pow(1.0 - max(0.0, dot(normal, -incident)), 2.0);
        vec3 fresnelCol = mix(vec3(0), vec3(1), fresnelFactor);
        outEnvMapColSpecIntensity.rgb = fresnelCol * reflectCol.rgb * colSpecular;
    } else {
        outEnvMapColSpecIntensity.rgba = vec4(0, 0, 0, 0);
    }

    // Diffuse albedo:
    vec4 texCol = texture(texDiffuse, Texcoord);
    outAlbedoRoughness.rgb = texCol.rgb * colDiffuse;

    // Roughness:
    outAlbedoRoughness.a = shininess;
}
