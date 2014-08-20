#version 150

uniform vec3 materialColour;

out vec4 outColor;

void main()
{
    outColor = vec4(materialColour, 1.0);
}
