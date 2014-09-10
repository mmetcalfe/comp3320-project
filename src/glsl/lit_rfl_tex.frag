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
    vec3 lightVecRaw = eyeSpacePosition - (view * vec4(light.pos, 1)).xyz;
    vec3 lightVec = normalize(lightVecRaw);
    vec3 lightReflect = reflect(lightVec, normal);
    vec3 viewReflect = reflect(incident, normal);
//    vec3 halfAngleReflection = mix(incident, reflection, 0.5);

    float intensity = calculateIntensity(length(lightVecRaw));

//    // Environment map reflection:
//    float phongSpecular = phong(incident, viewReflect, shininess);
//    vec4 tmp_sampleCoord = modelViewInverse * vec4(reflection, 0);
//    vec3 sampleCoord = tmp_sampleCoord.xyz;
//    vec4 reflectCol = texture(texEnvironmentMap, sampleCoord);
//    vec4 outSpecular = vec4(colSpecular, 1.0) * reflectCol * phongSpecular; // * shininessStrength;

    // Specular reflection:
    float phongSpecular = phong(incident, lightReflect, shininess);
    vec3 outSpecular = colSpecular * light.colSpecular * phongSpecular; // * shininessStrength;

    vec4 texCol = texture(texDiffuse, Texcoord);
    vec3 outDiffuse = texCol.rgb * colDiffuse;

    vec3 outAmbient = colAmbient * light.colAmbient;

    // Don't show lighting on surfaces that are facing the wrong way:
    float lightDot = dot(normal, lightVec);
    // Use a ramp near zero to remove noise when the light is very near the plane of the surface.
    #define DOT_CUTOFF 0.001
    float dotLessZero = lightDot < -DOT_CUTOFF ? 1 : -(1.0/DOT_CUTOFF) * lightDot;
    float normalDirTest = lightDot < 0 ? dotLessZero : 0;
//    float normalDirTest = lightDot < 0 ? 1 : 0; // What it looks like without the ramp (causes speckles when the light it coplanar with the surface).


    outColor = vec4(outAmbient + (outDiffuse + outSpecular) * intensity, 1.0) * normalDirTest;
}
