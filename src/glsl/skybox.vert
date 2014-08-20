#version 330 core

in vec3 position;
in vec3 normal;

out vec3 Mapcoord;
out vec4 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main() {
    Mapcoord = normalize(position);
    Normal = vec4(normal, 0);

    gl_Position = proj * view * model * vec4(position, 1.0);

    // Fix to the far plane
    gl_Position.z = gl_Position.w - 0.00001;
}
