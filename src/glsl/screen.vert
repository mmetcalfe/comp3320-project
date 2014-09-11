#version 150

in vec2 position;
in vec2 texcoord;

out vec2 Texcoord;

uniform mat4 model;

void main() {
    Texcoord = texcoord;

    vec4 transformed = model * vec4(position, 0, 1);
    gl_Position = vec4(transformed.xy, 0.0, 1.0);
}
