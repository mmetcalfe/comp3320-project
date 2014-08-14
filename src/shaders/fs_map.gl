#version 330 core

in vec3 Mapcoord;

uniform samplerCube environmentMap;

out vec4 outColor;

void main() {
    outColor = texture(environmentMap, Mapcoord);
}
