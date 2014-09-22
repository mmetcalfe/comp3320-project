#version 330 core

in vec3 position;
in vec3 normal;
in vec2 texcoord;

out vec2 Texcoord;
out vec4 eyeSpacePosition;
out vec3 eyeSpaceNormal;
//out vec4 lightClipPos;

uniform mat4 model;
uniform mat4 view;
//uniform mat4 proj;
uniform mat4 mvp;
//uniform mat4 viewInverse;
//
//// Light uniforms:
//struct Light {
//    vec3 pos;
//    vec3 dir;
//    float attenuationConstant;
//    float attenuationLinear;
//    float attenuationQuadratic;
//    vec3 colDiffuse;
//    vec3 colSpecular;
//    vec3 colAmbient;
//    float angleConeInner;
//    float angleConeOuter;
//
//    bool hasShadowMap;
//    sampler2D texShadowMap;
//    mat4 view;
//    mat4 proj;
//};
//
//uniform Light light;


void main() {
    Texcoord = texcoord;
//    gl_Position = proj * view * model * vec4(position, 1.0);
    gl_Position = mvp * vec4(position, 1.0);

    mat4 modelView = view * model;

    eyeSpacePosition = modelView * vec4(position, 1.0);

    vec4 eyeSpaceNormalTmp = modelView * vec4(normal, 0);
    eyeSpaceNormal = eyeSpaceNormalTmp.xyz;

//    lightClipPos = light.proj * light.view * viewInverse * eyeSpacePosition;
}
