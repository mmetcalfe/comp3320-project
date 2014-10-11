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

uniform mat4 viewInverse;

// Material uniforms
uniform vec3 colDiffuse;
uniform vec3 colSpecular;
uniform float shininess;
//uniform float shininessStrength;
uniform samplerCube texEnvironmentMap;
uniform sampler2D texDiffuse;

void main() {
    // Face normal in eye space:
    vec3 normal = normalize(eyeSpaceNormal.xyz);
    vec3 incident = normalize(eyeSpacePosition.xyz);

    // Normal:
    outNormal.xy = normal.xy;

    // Environment map reflection:
    outEnvMapColSpecIntensity.rgb = vec3(0, 0, 0);
    if (shininess > 0) {
        vec3 viewReflect = reflect(incident, normal);
        vec4 sampleCoord = viewInverse * vec4(viewReflect, 0);
        vec4 reflectCol = texture(texEnvironmentMap, sampleCoord.xyz);
        outEnvMapColSpecIntensity.rgb = reflectCol.rgb * colSpecular;
    }

    // Diffuse albedo:
    vec4 texCol = texture(texDiffuse, Texcoord);
    outAlbedoRoughness.rgb = texCol.rgb * colDiffuse;

    // Roughness:
    outAlbedoRoughness.a = shininess;
}
