#version 150

uniform vec3 colDiffuse;

out vec4 outColor;

void main() {
    outColor = vec4(colDiffuse, 1.0);
}
