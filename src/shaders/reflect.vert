#version 330 core

in vec3 position;

out vec3 eyeSpacePosition;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main() {
    gl_Position = proj * view * model * vec4(position, 1.0);

    vec4 tmp = view * model * vec4(position, 1.0);
    eyeSpacePosition = tmp.xyz;
}
