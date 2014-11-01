#version 330 core

in vec2 Texcoord;
//in vec3 Position;

out vec4 outColor;

// Transform uniforms:
//uniform mat4 modelViewInverse;
uniform mat4 view;
uniform mat4 viewInverse;
uniform mat4 projInverse;

// G-Buffer uniforms:
uniform sampler2D texDepthStencil;
uniform sampler2D texNormal;
uniform sampler2D texAlbedoRoughness;
uniform sampler2D texEnvMapColSpecIntensity;

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

    bool isSpotlight;
    bool isDirectional;
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

vec2 randVec2(in vec3 seed1, in float seed2, in float seed3) {
    return vec2(rand(vec4(seed1, seed2)), rand(vec4(seed1, seed3)));
}

float doShadowMapping(in vec4 eyeSpacePosition) {
    float lightVisibility = 1.0;
    if (light.hasShadowMap) {
        vec4 lightClipPos = light.proj * light.view * viewInverse * eyeSpacePosition;

        vec3 lightClipPosDivided = lightClipPos.xyz / lightClipPos.w;
        vec3 shadowLookup = (lightClipPosDivided * 0.5) + 0.5;

        float bias = 0.0;
        float samples = 16;
        float radius = (1.0 / 300.0);
        if (light.isDirectional) {
            radius = (1.0 / 600.0);
        }

        for (int i = 0; i < samples; i++) {
//            int index = int(4.0 * rand(vec4(eyeSpacePosition.xyz, i))) % 4;
//            vec2 stratifiedCoord = vec2(shadowLookup.xy + poissonDisk[index] / 700.0); //,  (shadowLookup.z - bias ) / shadowLookup.w);
            vec2 offset = randVec2(eyeSpacePosition.xyz, i, i + samples);

            vec2 stratifiedCoord = vec2(shadowLookup.xy + offset * radius);

            float occluderDepth = texture(light.texShadowMap, stratifiedCoord.xy).x;

            if (occluderDepth < ((lightClipPos.z - bias) / lightClipPos.w) * 0.5 + 0.5)
                lightVisibility -= 1.0 / samples;
        }

        lightVisibility = clamp(lightVisibility, 0.0, 1.0);

        // Apply the view frustum:
        if (!light.isDirectional) {
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
    }

    return lightVisibility;
}

// Based on code from: http://mynameismjp.wordpress.com/2009/03/10/reconstructing-position-from-depth/
vec3 eyeSpacePosFromDepth(in float depth, in vec2 texcoord) {

    // Get x/w and y/w from the viewport position:
    float x = texcoord.x * 2 - 1;
    float y = texcoord.y * 2 - 1;
    float z = depth * 2 - 1;

    // Construct clip-space position:
    vec4 clipPos = vec4(x, y, z, 1.0f);

    // Transform by the inverse projection matrix:
    vec4 viewPos =  projInverse * clipPos;

    // Divide by w to get the view-space position:
    return viewPos.xyz / viewPos.w;
}

float calcSpotlightFactor(in vec3 lightVec) {
    if (!light.isSpotlight)
        return 1.0;

    float minDot = cos(light.angleConeOuter / 2.0); // TODO: precalc.
    float lightDirDot = dot((viewInverse * vec4(lightVec, 0)).xyz, light.dir);

    if (lightDirDot < minDot)
        return 0.0;

    float innerDot = cos(light.angleConeInner / 2.0); // TODO: precalc.

    if (lightDirDot > innerDot)
        return 1.0;

    return (lightDirDot - minDot) / (innerDot - minDot);
}

void main() {
    vec3 colSpecular = vec3(1.0, 1.0, 1.0);

    // Extract g-buffer components:
    vec4 depthStencil = texture(texDepthStencil, Texcoord);
    vec4 normalXY = texture(texNormal, Texcoord);
    vec4 albedoRoughness = texture(texAlbedoRoughness, Texcoord);
    vec4 envMapColSpecIntensity = texture(texEnvMapColSpecIntensity, Texcoord);

    float depth = depthStencil.x;

    vec2 normalXY_2 = normalXY.xy * normalXY.xy;
    float normalZ = sqrt(1 - normalXY_2.x - normalXY_2.y);
    vec3 eyeSpaceNormal = vec3(normalXY.xy, normalZ);

    vec3 albedo = albedoRoughness.rgb;
    float roughness = albedoRoughness.a * 8; // Map from [0, 1].
    vec3 envMapCol = envMapColSpecIntensity.rgb;
    float emissiveFactor = envMapColSpecIntensity.a;
    bool emissive = emissiveFactor > 0.5;

    vec3 eyeSpacePosition = eyeSpacePosFromDepth(depth, Texcoord);

    // Normal and light vectors:
    vec3 normal = normalize(eyeSpaceNormal.xyz);
    vec3 lightVecRaw;
    vec3 lightVec;
    if (!light.isDirectional) {
        lightVecRaw = eyeSpacePosition.xyz - (view * vec4(light.pos, 1)).xyz;
        lightVec = normalize(lightVecRaw);
    } else {
        lightVecRaw = (view * vec4(light.dir, 0)).xyz;
        lightVec = normalize(lightVecRaw);
    }

    // Don't show lighting on surfaces that are facing the wrong way:
    float lightDot = -dot(lightVec, normal);

    vec3 incident = normalize(eyeSpacePosition.xyz);

    // Specular reflection:
    vec3 outSpecular;
    if (roughness > 0 && lightDot > 0) {
        vec3 lightReflect = reflect(lightVec, normal);
        float phongSpecular = phong(incident, lightReflect, roughness);
//        outSpecular = colSpecular * light.colSpecular * phongSpecular;

        // Based on: http://en.wikibooks.org/wiki/GLSL_Programming/Unity/Specular_Highlights_at_Silhouettes
        vec3 halfVec = normalize(-lightVec - incident);
        float fresnelExp = 5.0;
        float fresnelFactor = pow(1.0 - max(0.0, dot(halfVec, -incident)), fresnelExp);
        vec3 fresnelCol = mix(colSpecular, vec3(1.0), fresnelFactor);
        outSpecular = fresnelCol * light.colSpecular * phongSpecular;
    } else {
        outSpecular = vec3(0, 0, 0);
    }

    // Shadow mapping:
    float lightVisibility = doShadowMapping(vec4(eyeSpacePosition, 1));

    // Spotlight cone:
    float spotFactor = calcSpotlightFactor(lightVec);

    // Diffuse component:
    vec3 outDiffuse = albedo * light.colDiffuse * max(lightDot, 0);

    // Ambient component:
    vec3 outAmbient = albedo * light.colAmbient;

    // Final colour:
    float intensity = calculateIntensity(length(lightVecRaw));
    vec3 finalColor = outAmbient + (outDiffuse + outSpecular) * intensity * lightVisibility * spotFactor;

    if (emissive) {
        finalColor = albedo;
    }

    outColor = vec4(finalColor, 1.0);
}
