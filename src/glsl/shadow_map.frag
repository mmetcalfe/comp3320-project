#version 330 core

//out vec4 outColor;
out float outColor;


void main() {
//    float tmp = gl_FragCoord.z;
//    outColor = vec4(tmp, tmp, tmp, 1.0);

    outColor = gl_FragCoord.z;
}
