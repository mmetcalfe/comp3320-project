#version 330 core

in vec2 Texcoord;
in vec3 eyeSpacePosition;
in vec3 eyeSpaceNormal;

out vec4 outColor;

// Transform uniforms:
uniform mat4 modelViewInverse;
uniform mat4 view;

// Material uniforms
uniform vec3 colAmbient;
uniform vec3 colDiffuse;
uniform vec3 colSpecular;
uniform float shininess;
//uniform float shininessStrength;
uniform samplerCube texEnvironmentMap;
uniform sampler2D texDiffuse;

void main() {
    // face normal in eye space:
    vec3 normal = normalize(eyeSpaceNormal);
    vec3 incident = normalize(eyeSpacePosition);
    vec3 reflection = reflect(incident, normal);
//    vec3 halfAngleReflection = mix(incident, reflection, 0.5);

    float phongSpecularReflectFactor = pow(dot(-incident, reflection), shininess);

    vec4 tmp_sampleCoord = modelViewInverse * vec4(reflection, 0);
    vec3 sampleCoord = tmp_sampleCoord.xyz;

    vec4 reflectCol = texture(texEnvironmentMap, sampleCoord);
    vec4 texCol = texture(texDiffuse, Texcoord);

    vec4 outAmbient = vec4(colAmbient, 1.0);
    vec4 outDiffuse = texCol * vec4(colDiffuse, 1.0);

    vec4 outSpecular = vec4(colSpecular, 1.0) * reflectCol * phongSpecularReflectFactor; // * shininessStrength;
    outColor = outAmbient + outDiffuse + outSpecular;
}
