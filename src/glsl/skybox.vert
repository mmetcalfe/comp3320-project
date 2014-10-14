#version 330 core

in vec3 position;

out vec3 Mapcoord;

uniform mat4 mvp;

void main() {
    Mapcoord = normalize(position);

    gl_Position = mvp * vec4(position, 1.0);

    // Fix to the far plane
    gl_Position.z = gl_Position.w - 0.00001;
}
