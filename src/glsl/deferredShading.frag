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
//      vec4 lightClipPos = light.proj * light.view * inverse(view) * eyeSpacePosition;

        vec3 lightClipPosDivided = lightClipPos.xyz / lightClipPos.w;
        vec3 shadowLookup = (lightClipPosDivided * 0.5) + 0.5;
//        vec4 shadowLookup = lightClipPos;

//        vec4 transformedClipPos = (lightClipPos * 0.5) + 0.5;
//        transformedClipPos.w = lightClipPos.w;
//        vec4 shadowLookup = transformedClipPos;
//        vec4 shadowLookup = transformedClipPos / transformedClipPos.w;

//        float shadowDepth = texture(light.texShadowMap, shadowLookup.xy).x;
////        float shadowDepth = texture(light.texShadowMap, shadowLookup.xy / shadowLookup.w).z;
        float bias = 0.0;
//        lightVisibility = shadowDepth < ((lightClipPos.z - bias) / lightClipPos.w) * 0.5 + 0.5 ? 0 : 1;
////        float lightVisibility = textureProj(light.texShadowMap, shadowLookup.xyw).z < (shadowLookup.z - bias) / shadowLookup.w;// ? 0 : 1;

        vec2 poissonDisk[4] = vec2[](
                vec2( -0.94201624, -0.39906216 ),
                vec2( 0.94558609, -0.76890725 ),
                vec2( -0.094184101, -0.92938870 ),
                vec2( 0.34495938, 0.29387760 )
        );

        float samples = 16;
        for (int i = 0; i < samples; i++) {
//            int index = int(4.0 * rand(vec4(eyeSpacePosition.xyz, i))) % 4;
//            vec2 stratifiedCoord = vec2(shadowLookup.xy + poissonDisk[index] / 700.0); //,  (shadowLookup.z - bias ) / shadowLookup.w);
//            vec2 stratifiedCoord = vec2(shadowLookup.xy + poissonDisk[index] / 700.0); //,  (shadowLookup.z - bias ) / shadowLookup.w);
            vec2 offset = vec2(rand(vec4(eyeSpacePosition.xyz, i)), rand(vec4(eyeSpacePosition.xyz, i + samples)));

//            float l = clamp(length(offset), 0, 1);
//            offset = normalize(offset) * l;
//            vec2 stratifiedCoord = vec2(shadowLookup.xy + normalize(offset) / 300.0);
            vec2 stratifiedCoord = vec2(shadowLookup.xy + offset / 300.0);

            float occluderDepth = texture(light.texShadowMap, stratifiedCoord.xy).x;

            if (occluderDepth < ((lightClipPos.z - bias) / lightClipPos.w) * 0.5 + 0.5)
                lightVisibility -= 1.0/samples;// * (1.0 - texture(light.texShadowMap, stratifiedCoord));
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
//            outColor = vec4(0.5, 0, 0, 1);
        } else if (shadowLookup.z < lower) {
            lightVisibility = 1;
//            outColor = vec4(1, 0, 0, 1);
        }

//        outColor = vec4(shadowDepth, shadowDepth, shadowDepth, 1);
//        return;
    }

    return lightVisibility;
}

// Based on code from: http://mynameismjp.wordpress.com/2009/03/10/reconstructing-position-from-depth/
vec3 eyeSpacePosFromDepth(in float depth, in vec2 texcoord) {

    // Get x/w and y/w from the viewport position
    float x = texcoord.x * 2 - 1;
    float y = texcoord.y * 2 - 1;
    float z = depth * 2 - 1;

    // Construct clip-space position:
    vec4 clipPos = vec4(x, y, z, 1.0f);

    // Transform by the inverse projection matrix
    vec4 viewPos =  projInverse * clipPos;

    // Divide by w to get the view-space position
    return viewPos.xyz / viewPos.w;
}


void main() {
    vec3 colSpecular = vec3(1.0, 1.0, 1.0);

    vec4 depthStencil = texture(texDepthStencil, Texcoord);
    vec4 normalXY = texture(texNormal, Texcoord);
    vec4 albedoRoughness = texture(texAlbedoRoughness, Texcoord);
    vec4 envMapColSpecIntensity = texture(texEnvMapColSpecIntensity, Texcoord);

    float depth = depthStencil.x;

    vec2 normalXY_2 = normalXY.xy * normalXY.xy;
    float normalZ = sqrt(1 - normalXY_2.x - normalXY_2.y);
    vec3 eyeSpaceNormal = vec3(normalXY.xy, normalZ);

    vec3 albedo = albedoRoughness.rgb;
    float roughness = albedoRoughness.a;
    vec3 envMapCol = envMapColSpecIntensity.rgb;
    float specIntensity = envMapColSpecIntensity.a;

    vec3 eyeSpacePosition = eyeSpacePosFromDepth(depth, Texcoord);
//
//    outColor = vec4(albedo, 1.0);
//
//    outColor = vec4(envMapCol, 1.0);
//    outColor = vec4(eyeSpacePos, 1.0);
//    outColor = vec4(eyeSpacePos, 1.0);
//    outColor = vec4(normal, 1.0);

    // face normal in eye space:
    vec3 normal = normalize(eyeSpaceNormal.xyz);
    vec3 lightVecRaw = eyeSpacePosition.xyz - (view * vec4(light.pos, 1)).xyz;
    vec3 lightVec = normalize(lightVecRaw);

    // Don't show lighting on surfaces that are facing the wrong way:
    float lightDot = -dot(lightVec, normal);
    float normalDirTest = 1;
    #define DOT_CUTOFF 0.01
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
    if (roughness > 0) {
        vec3 viewReflect = reflect(incident, normal);
//        float phongViewSpecular = phong(incident, viewReflect, roughness);
//        vec3 envReflectCol = envMapCol.rgb * phongViewSpecular;
        vec3 envReflectCol = envMapCol.rgb;
        outReflect = envReflectCol * colSpecular;
    }

    // Specular reflection:
    vec3 outSpecular = vec3(0, 0, 0);
    if (roughness > 0) {
        vec3 lightReflect = reflect(lightVec, normal);
        float phongSpecular = phong(incident, lightReflect, roughness);
        vec3 phongHighlightCol = light.colSpecular * phongSpecular;
        outSpecular = phongHighlightCol * colSpecular;
    }

    // Shadow mapping:
    float lightVisibility = doShadowMapping(vec4(eyeSpacePosition, 1));

    // Diffuse component:
//    vec4 texCol = texture(texDiffuse, Texcoord);
//    vec3 outDiffuse = texCol.rgb * colDiffuse * light.colDiffuse * clamp(lightDot, 0, 1);
    vec3 outDiffuse = albedo * light.colDiffuse * clamp(lightDot, 0, 1);

    // Ambient component:
//    vec3 outAmbient = colAmbient * light.colAmbient;
    vec3 outAmbient = albedo * light.colAmbient;

    // Final colour:
    float intensity = calculateIntensity(length(lightVecRaw));
    vec3 finalColor = outAmbient + outReflect + (outDiffuse + outSpecular) * intensity * lightVisibility * normalDirTest;

    outColor = vec4(finalColor, 1.0);
}
