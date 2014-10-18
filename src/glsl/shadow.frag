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
    float fov;

    bool hasShadowMap;
    sampler2D texShadowMap;
    mat4 view;
    mat4 proj;
};

uniform Light light;


float phong(in vec3 incident, in vec3 reflection, in float shininess) {
    return pow(clamp(dot(-incident, reflection), 0, 1), shininess);
}

float calculateIntensity(in float lightDist) {
    float denom = light.attenuationConstant;
    denom += light.attenuationLinear * lightDist;
    denom += light.attenuationQuadratic * lightDist * lightDist;
    return 1.0 / denom;
}

float rand(in vec4 seed) {
    float dot_product = dot(seed, vec4(12.9898, 78.233, 45.164, 94.673));
    return fract(sin(dot_product) * 43758.5453);
}

float doShadowMapping(in vec4 eyeSpacePosition) {
    float lightVisibility = 1.0;
    if (light.hasShadowMap) {
        vec4 lightClipPos = light.proj * light.view * viewInverse * eyeSpacePosition;

        vec3 lightClipPosDivided = lightClipPos.xyz / lightClipPos.w;
        vec3 shadowLookup = (lightClipPosDivided * 0.5) + 0.5;

        float bias = 0.0;
        float samples = 16;
        float radius = 1.0 / 300.0;
        for (int i = 0; i < samples; i++) {
//            int index = int(4.0 * rand(vec4(eyeSpacePosition.xyz, i))) % 4;
//            vec2 stratifiedCoord = vec2(shadowLookup.xy + poissonDisk[index] / 700.0); //,  (shadowLookup.z - bias ) / shadowLookup.w);
            vec2 offset = vec2(rand(vec4(eyeSpacePosition.xyz, i)), rand(vec4(eyeSpacePosition.xyz, i + samples)));

            vec2 stratifiedCoord = vec2(shadowLookup.xy + offset * radius);

            float occluderDepth = texture(light.texShadowMap, stratifiedCoord.xy).x;

            if (occluderDepth < ((lightClipPos.z - bias) / lightClipPos.w) * 0.5 + 0.5)
                lightVisibility -= 1.0 / samples;
        }

        lightVisibility = clamp(lightVisibility, 0.0, 1.0);

        // Apply the view frustum:
        float upper = 1;
        float lower = 0;
        if (shadowLookup.x < lower || shadowLookup.x > upper ||
                shadowLookup.y < lower || shadowLookup.y > upper
                || shadowLookup.z > upper // || shadowLookup.z < lower
                ) {
            lightVisibility = 0;
        } else if (shadowLookup.z < lower) {
            lightVisibility = 1;
        }
    }

    return lightVisibility;
}

float calcSpotlightFactor(in vec3 lightVec) {
    float spotFactor = 1.0;

    if (light.hasShadowMap) {
        float lightDirDot = dot((viewInverse * vec4(lightVec, 0)).xyz, light.dir);
        spotFactor = lightDirDot > cos(light.fov / 2.0) ? 1.0 : 0.0;
    }

    return spotFactor;
}

void main() {
    // face normal in eye space:
    vec3 normal = normalize(eyeSpaceNormal.xyz);
    vec3 lightVecRaw = eyeSpacePosition.xyz - (view * vec4(light.pos, 1)).xyz;
    vec3 lightVec = normalize(lightVecRaw);

    // Don't show lighting on surfaces that are facing the wrong way:
    float lightDot = -dot(lightVec, normal);

    vec3 incident = normalize(eyeSpacePosition.xyz);

    // Environment map reflection:
    vec3 outReflect;
    if (shininess > 0) {
        vec3 viewReflect = reflect(incident, normal);
//        float phongViewSpecular = phong(incident, viewReflect, shininess);
        vec4 sampleCoord = viewInverse * vec4(viewReflect, 0);
        sampleCoord = vec4(sampleCoord.x, sampleCoord.z, -sampleCoord.y, 1);
        vec4 reflectCol = texture(texEnvironmentMap, sampleCoord.xyz);
//        outReflect = reflectCol.rgb * colSpecular; // * phongViewSpecular;

        // Based on: http://en.wikibooks.org/wiki/GLSL_Programming/Unity/Specular_Highlights_at_Silhouettes
        float fresnelFactor = pow(1.0 - max(0.0, dot(normal, -incident)), 2.0);
        vec3 fresnelCol = mix(vec3(0.1), vec3(1), fresnelFactor);
        outReflect = fresnelCol * reflectCol.rgb * colSpecular;
    } else {
        outReflect = vec3(0, 0, 0);
    }

    // Specular reflection:
    vec3 colSpecular = vec3(1.0, 1.0, 1.0); // Ignore specular color to match 'deferredShading.frag'.
    vec3 outSpecular;
    if (shininess > 0 && lightDot > 0) {
        vec3 lightReflect = reflect(lightVec, normal);
        float phongSpecular = phong(incident, lightReflect, shininess);
//        outSpecular = colSpecular * light.colSpecular * phongSpecular;

        // Based on: http://en.wikibooks.org/wiki/GLSL_Programming/Unity/Specular_Highlights_at_Silhouettes
        vec3 halfVec = normalize(-lightVec - incident);
        float fresnelFactor = pow(1.0 - max(0.0, dot(halfVec, -incident)), 5.0);
        vec3 fresnelCol = mix(colSpecular, vec3(1.0), fresnelFactor);
        outSpecular = fresnelCol * light.colSpecular * phongSpecular;
    } else {
        outSpecular = vec3(0, 0, 0);
    }

    // Shadow mapping:
    float lightVisibility = doShadowMapping(eyeSpacePosition);

    // Spotlight cone:
    float spotFactor = calcSpotlightFactor(lightVec);

    // Diffuse component:
    vec4 texCol = texture(texDiffuse, Texcoord);
    vec3 outDiffuse = texCol.rgb * colDiffuse * light.colDiffuse * max(lightDot, 0);

    // Ambient component:
    vec3 outAmbient = colAmbient * light.colAmbient;

    // Final colour:
    float intensity = calculateIntensity(length(lightVecRaw));
    vec3 finalColor = outAmbient + outReflect + (outDiffuse + outSpecular) * intensity * lightVisibility * spotFactor;

    outColor = vec4(finalColor, opacity);
}
