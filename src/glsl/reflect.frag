#version 330 core

in vec3 eyeSpacePosition;
in vec3 eyeSpaceNormal;

uniform samplerCube environmentMap;

uniform mat4 modelViewInverse;
uniform mat4 view;

out vec4 outColor;

void main() {
    // face normal in eye space:
    vec3 normal = normalize(eyeSpaceNormal);
    vec3 incident = normalize(eyeSpacePosition);

    vec3 reflection = reflect(incident, normal);

    vec4 tmp_sampleCoord = modelViewInverse * vec4(reflection, 0);
    vec3 sampleCoord = tmp_sampleCoord.xyz;

    outColor = texture(environmentMap, sampleCoord);
    // outColor = vec4(normalize(eyeSpaceNormal), 1);

    // vec4 tmp = inverse(view) * vec4(eyeSpaceNormal, 0);
    // outColor = vec4(normalize(tmp.xyz), 1);
}
