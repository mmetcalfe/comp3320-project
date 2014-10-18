#version 150

in vec2 Texcoord;

out vec4 outColor;

uniform sampler2D texDiffuse;

void main() {
    outColor = vec4(vec3(texture(texDiffuse, Texcoord).a), 1.0);
}
