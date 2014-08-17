#version 330 core

in vec3 position;
in vec3 normal;

out vec3 eyeSpacePosition;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main() {
    gl_Position = proj * view * model * vec4(position, 1.0);
    Normal = normal;

    vec4 tmp = view * model * vec4(position, 1.0);
    eyeSpacePosition = tmp.xyz;
}
