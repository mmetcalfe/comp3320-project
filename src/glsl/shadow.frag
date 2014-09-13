#version 330 core

in vec2 Texcoord;
in vec4 eyeSpacePosition;
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

    bool hasShadowMap;
    sampler2D texShadowMap;
    mat4 view;
    mat4 proj;
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
    vec3 normal = normalize(eyeSpaceNormal.xyz);
    vec3 incident = normalize(eyeSpacePosition.xyz);
    vec3 lightVecRaw = eyeSpacePosition.xyz - (view * vec4(light.pos, 1)).xyz;
    vec3 lightVec = normalize(lightVecRaw);
    vec3 lightReflect = reflect(lightVec, normal);
    vec3 viewReflect = reflect(incident, normal);
//    vec3 halfAngleReflection = mix(incident, reflection, 0.5);

    float intensity = calculateIntensity(length(lightVecRaw));

    // Environment map reflection:
    float phongViewSpecular = phong(incident, viewReflect, shininess);
    vec4 tmp_sampleCoord = modelViewInverse * vec4(viewReflect, 0);
    vec3 sampleCoord = tmp_sampleCoord.xyz;
    vec4 reflectCol = texture(texEnvironmentMap, sampleCoord);
    vec3 envReflectCol = reflectCol.rgb * phongViewSpecular;

    // Specular reflection:
    float phongSpecular = phong(incident, lightReflect, shininess);
    vec3 phongHighlightCol = light.colSpecular * phongSpecular;

    vec3 outSpecular = (phongHighlightCol + envReflectCol) * colSpecular;
//    vec3 outSpecular = phongHighlightCol;

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

    // Do shadow mapping:
    float isLit = 1.0;
    if (light.hasShadowMap) {
        vec4 lightClipPos = inverse(view) * eyeSpacePosition;
        lightClipPos.w = 1;
        lightClipPos = light.proj * light.view * lightClipPos;

        vec3 lightClipPosDivided = lightClipPos.xyz / lightClipPos.w;
        vec3 shadowLookup = (lightClipPosDivided * 0.5) + 0.5;

//    vec4 transformedClipPos = (lightClipPos * 0.5) + 0.5;
//    vec4 shadowLookup = transformedClipPos;//transformedClipPos / transformedClipPos.w;
//    vec4 shadowLookup = transformedClipPos / transformedClipPos.w;

        float shadowDepth = texture(light.texShadowMap, shadowLookup.xy).z;
//    float shadowDepth = textureProj(light.texShadowMap, shadowLookup.xyw).z;
        float bias = 0.004;
        isLit = shadowDepth < shadowLookup.z - bias ? 0 : 1;
//    float isLit = textureProj(light.texShadowMap, shadowLookup.xyw).z  <  (shadowLookup.z-bias)/shadowLookup.w ? 0 : 1;

        // Apply the view frustum:
        if (lightClipPosDivided.x < -1 || lightClipPosDivided.x > 1 ||
                lightClipPosDivided.y < -1 || lightClipPosDivided.y > 1
//            || lightClipPosDivided.z < -1 || lightClipPosDivided.z > 1
                ) {
//            outColor = vec4(0, 0, 0, 1);
//        outColor += vec4(0.1, 0, 0.2, 1);
            isLit = 0;
        } else if (lightClipPosDivided.z < -1 || lightClipPosDivided.z > 1) {
            isLit = 1;
        }
    }

    outColor = vec4(outAmbient + (outDiffuse + outSpecular) * intensity, 1.0) * isLit * normalDirTest;
//    outColor = vec4(shadowDepth, shadowDepth, shadowDepth, 1.0);
}
