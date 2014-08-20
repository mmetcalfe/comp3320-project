#version 330 core

in vec3 Mapcoord;

uniform samplerCube texEnvironmentMap;

out vec4 outColor;

void main() {
    outColor = texture(texEnvironmentMap, Mapcoord);
}
