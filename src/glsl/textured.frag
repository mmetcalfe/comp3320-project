#version 150

in vec2 Texcoord;

out vec4 outColor;

uniform sampler2D texDiffuse;

void main() {
    outColor = vec4(texture(texDiffuse, Texcoord).rgb, 1.0);
}
