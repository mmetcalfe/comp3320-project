#version 330 core

in vec3 position;

//uniform mat4 model;
//uniform mat4 view;
//uniform mat4 proj;
uniform mat4 mvp;

void main() {
//    gl_Position = proj * view * model * vec4(position, 1.0);
    gl_Position = mvp * vec4(position, 1.0);
}
