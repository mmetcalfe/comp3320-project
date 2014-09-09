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

// Light uniforms:
struct Light {
    vec3 pos;
    vec3 dir;
    float attenuationConstant;
    float attenuationLinear;
    float attenuationQuadratic;
    vec3 colDiffuse;
    vec3 colSpecular;
    vec3 colAmbient;
    float angleConeInner;
    float angleConeOuter;
};

uniform Light light;


float phong(in vec3 incident, in vec3 reflection, in float shininess) {
    return pow(dot(-incident, reflection), shininess);
}

float calculateIntensity(in float lightDist) {
    float denom = light.attenuationConstant;
    denom += light.attenuationLinear * lightDist;
    denom += light.attenuationQuadratic * lightDist * lightDist;
    return 1.0 / denom;
}

void main() {
    // face normal in eye space:
    vec3 normal = normalize(eyeSpaceNormal);
    vec3 incident = normalize(eyeSpacePosition);
    vec3 reflection = reflect(incident, normal);
    vec3 lightVec = eyeSpacePosition - (view * vec4(light.pos, 1)).xyz;
//    vec3 halfAngleReflection = mix(incident, reflection, 0.5);

    float intensity = calculateIntensity(length(lightVec.xyz));

//    // Environment map reflection:
//    float phongSpecular = phong(incident, reflection, shininess);
//    vec4 tmp_sampleCoord = modelViewInverse * vec4(reflection, 0);
//    vec3 sampleCoord = tmp_sampleCoord.xyz;
//    vec4 reflectCol = texture(texEnvironmentMap, sampleCoord);
//    vec4 outSpecular = vec4(colSpecular, 1.0) * reflectCol * phongSpecular; // * shininessStrength;

    // Environment map reflection:
    float phongSpecular = phong(incident, reflection, shininess);
    vec4 tmp_sampleCoord = modelViewInverse * vec4(reflection, 0);
    vec3 sampleCoord = tmp_sampleCoord.xyz;
    vec4 reflectCol = texture(texEnvironmentMap, sampleCoord);
    vec4 outSpecular = vec4(colSpecular, 1.0) * reflectCol * phongSpecular; // * shininessStrength;

    vec4 texCol = texture(texDiffuse, Texcoord);
    vec4 outDiffuse = texCol * vec4(colDiffuse, 1.0);

    vec4 outAmbient = vec4(colAmbient * light.colAmbient, 1.0);

    outColor = (outAmbient + outDiffuse + outSpecular) * intensity;
}
