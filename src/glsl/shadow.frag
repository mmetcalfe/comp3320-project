#version 330 core

in vec2 Texcoord;
in vec4 eyeSpacePosition;
in vec3 eyeSpaceNormal;
//in vec4 lightClipPos;

out vec4 outColor;

// Transform uniforms:
uniform mat4 modelViewInverse;
uniform mat4 view;
uniform mat4 viewInverse;

// Material uniforms
uniform float opacity;
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

float doShadowMapping() {
    float isLit = 1.0;
    if (light.hasShadowMap) {
        vec4 lightClipPos = light.proj * light.view * viewInverse * eyeSpacePosition;
//      vec4 lightClipPos = light.proj * light.view * inverse(view) * eyeSpacePosition;

        vec3 lightClipPosDivided = lightClipPos.xyz / lightClipPos.w;
        vec3 shadowLookup = (lightClipPosDivided * 0.5) + 0.5;
//        vec4 shadowLookup = lightClipPos;

//        vec4 transformedClipPos = (lightClipPos * 0.5) + 0.5;
//        transformedClipPos.w = lightClipPos.w;
//        vec4 shadowLookup = transformedClipPos;
//        vec4 shadowLookup = transformedClipPos / transformedClipPos.w;

        float shadowDepth = texture(light.texShadowMap, shadowLookup.xy).x;
//        float shadowDepth = texture(light.texShadowMap, shadowLookup.xy / shadowLookup.w).z;
        float bias = 0.0;
        isLit = shadowDepth < ((lightClipPos.z - bias) / lightClipPos.w) * 0.5 + 0.5 ? 0 : 1;
//        float isLit = textureProj(light.texShadowMap, shadowLookup.xyw).z < (shadowLookup.z - bias) / shadowLookup.w;// ? 0 : 1;

        // Apply the view frustum:
        float upper = 1;
        float lower = 0;
        if (shadowLookup.x < lower || shadowLookup.x > upper ||
                shadowLookup.y < lower || shadowLookup.y > upper
                || shadowLookup.z > upper // || shadowLookup.z < lower
                ) {
            isLit = 0;
//            outColor = vec4(0.5, 0, 0, 1);
        } else if (shadowLookup.z < lower) {
            isLit = 1;
//            outColor = vec4(1, 0, 0, 1);
        }

//        outColor = vec4(shadowDepth, shadowDepth, shadowDepth, 1);
//        return;
    }

    return isLit;
}

void main() {
    // face normal in eye space:
    vec3 normal = normalize(eyeSpaceNormal.xyz);
    vec3 lightVecRaw = eyeSpacePosition.xyz - (view * vec4(light.pos, 1)).xyz;
    vec3 lightVec = normalize(lightVecRaw);

    // Don't show lighting on surfaces that are facing the wrong way:
    float lightDot = -dot(lightVec, normal);
    float normalDirTest = 1;
    #define DOT_CUTOFF 0.11
    if (lightDot < DOT_CUTOFF) {
        // Use a ramp near zero to remove noise when the light is very near the plane of the surface.
        normalDirTest = lightDot > 0 ? (1.0 / DOT_CUTOFF) * lightDot : 0;

//      // What it looks like without the ramp (causes speckles when the light it coplanar with the surface).
//        float normalDirTest = lightDot < 0 ? 1 : 0;
//        outColor = vec4(1, 0, 1, 1) * normalDirTest;
//        return;
    }

    vec3 incident = normalize(eyeSpacePosition.xyz);

    // Environment map reflection:
    vec3 outReflect = vec3(0, 0, 0);
    if (shininess > 0) {
        vec3 viewReflect = reflect(incident, normal);
        float phongViewSpecular = phong(incident, viewReflect, shininess);
        vec4 sampleCoord = viewInverse * vec4(viewReflect, 0);
        sampleCoord = vec4(sampleCoord.x, sampleCoord.z, -sampleCoord.y, 1);
        vec4 reflectCol = texture(texEnvironmentMap, sampleCoord.xyz);
        outReflect = reflectCol.rgb * colSpecular * phongViewSpecular;
    }

    // Specular reflection:
    vec3 outSpecular = vec3(0, 0, 0);
    if (shininess > 0) {
        vec3 lightReflect = reflect(lightVec, normal);
        float phongSpecular = phong(incident, lightReflect, shininess);
        vec3 phongHighlightCol = light.colSpecular * phongSpecular;
        outSpecular = phongHighlightCol * colSpecular;
    }

    // Shadow mapping:
    float isLit = doShadowMapping();

    // Diffuse component:
    vec4 texCol = texture(texDiffuse, Texcoord);
    vec3 outDiffuse = texCol.rgb * colDiffuse * light.colDiffuse * clamp(lightDot, 0, 1);

    // Ambient component:
    vec3 outAmbient = colAmbient * light.colAmbient;

    // Final colour:
    float intensity = calculateIntensity(length(lightVecRaw));
    vec3 finalColor = outAmbient + outReflect + (outDiffuse + outSpecular) * intensity * isLit * normalDirTest;

    outColor = vec4(finalColor, opacity);
//    outColor = vec4(eyeSpaceNormal, 1.0);
}
