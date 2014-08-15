#version 330 core

in vec3 eyeSpacePosition;

uniform samplerCube environmentMap;

uniform mat4 modelViewInverse;

out vec4 outColor;

void main() {
    // face normal in eye space:
    vec3 face_normal_eye = normalize(cross(dFdx(eyeSpacePosition), dFdy(eyeSpacePosition)));

    vec3 incident = normalize(eyeSpacePosition);

    vec3 reflection = reflect(incident, face_normal_eye);

    vec4 tmp_sampleCoord = modelViewInverse * vec4(reflection, 0);
    vec3 sampleCoord = tmp_sampleCoord.xyz;

    outColor = texture(environmentMap, sampleCoord);
}
