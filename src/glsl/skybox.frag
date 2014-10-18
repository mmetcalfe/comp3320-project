#version 330 core

//in vec3 Mapcoord;
in vec4 eyeSpacePosition;

uniform mat4 viewInverse;

uniform samplerCube texEnvironmentMap;

out vec4 outColor;

void main() {
    vec3 incident = normalize(eyeSpacePosition.xyz);
    vec4 mapcoord = viewInverse * vec4(incident, 0.0);
    mapcoord = vec4(mapcoord.x, mapcoord.z, -mapcoord.y, 1);

    outColor = texture(texEnvironmentMap, mapcoord.xyz);
}
