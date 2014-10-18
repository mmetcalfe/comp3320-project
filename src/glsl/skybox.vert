#version 330 core

in vec3 position;

//out vec3 Mapcoord;
out vec4 eyeSpacePosition;

uniform mat4 mvp;
uniform mat4 view;
uniform mat4 model;

void main() {
//    Mapcoord = normalize(position);

    eyeSpacePosition = view * model * vec4(position, 1.0);

    gl_Position = mvp * vec4(position, 1.0);

    // Fix to the far plane:
    gl_Position.z = gl_Position.w - 0.00001;
}
