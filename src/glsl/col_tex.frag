#version 150

in vec3 Color;
in vec2 Texcoord;

out vec4 outColor;

uniform sampler2D texDiffuse;

void main()
{
    outColor = texture(texDiffuse, Texcoord) * vec4(Color, 1.0);
}
