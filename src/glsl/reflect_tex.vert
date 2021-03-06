#version 330 core

in vec3 position;
in vec3 normal;
in vec2 texcoord;

out vec2 Texcoord;
out vec3 eyeSpacePosition;
out vec3 eyeSpaceNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main() {
    Texcoord = texcoord;
    gl_Position = proj * view * model * vec4(position, 1.0);

    mat4 modelView = view * model;

    vec4 eyeSpacePositionTmp = modelView * vec4(position, 1.0);
    eyeSpacePosition = eyeSpacePositionTmp.xyz;

    vec4 eyeSpaceNormalTmp = modelView * vec4(normal, 0);
    eyeSpaceNormal = eyeSpaceNormalTmp.xyz;
}
