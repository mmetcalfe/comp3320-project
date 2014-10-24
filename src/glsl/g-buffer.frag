#version 330 core

/*
 * G-Buffer format:
 * |--------+--------+--------+--------|--------|
 * |   R8   |   G8   |   B8   |   A8   | attach |
 * |--------+--------+--------+--------|--------|
 * |          depth           | stencil|   DS   |
 * |--------+--------+--------+--------|--------|
 * | normal x (FP16) | normal y (FP16) |   RT0  | outNormal
 * |--------+--------+--------+--------|--------|
 * |      diffuse albedo      | rough  |   RT1  | outAlbedoRoughness
 * |--------+--------+--------+--------|--------|
 * |      env map col         | spec in|   RT2  | outEnvMapColSpecIntensity
 * |--------+--------+--------+--------|--------|
 * |                          |        |   RT3  |
 * |--------+--------+--------+--------|--------|
 */

in vec2 Texcoord;
in vec4 eyeSpacePosition;
in vec3 eyeSpaceNormal;

// G-Buffer outputs:
out vec4 outNormal;
out vec4 outAlbedoRoughness;
out vec4 outEnvMapColSpecIntensity;

uniform mat4 modelViewInverse;
uniform mat4 viewInverse;

// Material uniforms
uniform vec3 colDiffuse;
uniform vec3 colSpecular;
uniform float shininess;
//uniform float shininessStrength;
uniform samplerCube texEnvironmentMap;
uniform sampler2D texDiffuse;
uniform sampler2D texHeight;
uniform bool hasTexHeight;

float phong(in vec3 incident, in vec3 reflection, in float shininess) {
    return pow(clamp(dot(-incident, reflection), 0, 1), shininess);
}

// http://www.thetenthplanet.de/archives/1180
// and http://www.geeks3d.com/20130122/normal-mapping-without-precomputed-tangent-space-vectors/
mat3 cotangent_frame(vec3 N, vec3 p, vec2 uv) {
    // get edge vectors of the pixel triangle
    vec3 dp1 = dFdx( p );
    vec3 dp2 = dFdy( p );
    vec2 duv1 = dFdx( uv );
    vec2 duv2 = dFdy( uv );

    // solve the linear system
    vec3 dp2perp = cross( dp2, N );
    vec3 dp1perp = cross( N, dp1 );
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

    // construct a scale-invariant frame
    float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
    return mat3( T * invmax, B * invmax, N );
}

vec3 perturb_normal( vec3 N, vec3 V, vec2 texcoord ) {
    // assume N, the interpolated vertex normal and
    // V, the view vector (vertex to eye)
//    vec3 map = texture(tex1, texcoord ).xyz;

    float s_d = 1.0 / 640;
    float t_d = 1.0 / 640;
    float left = texture(texHeight, texcoord + vec2(-s_d, 0.0)).r;
    float right = texture(texHeight, texcoord + vec2(s_d, 0.0)).r;
    float top = texture(texHeight, texcoord + vec2(0.0, t_d)).r;
    float bottom = texture(texHeight, texcoord + vec2(0.0, -t_d)).r;

    vec3 map = normalize(vec3(left - right, bottom - top, 2.0));

//    map.y = -map.y;
    mat3 TBN = cotangent_frame(N, -V, texcoord);
    return normalize(TBN * map);
}

void main() {
    // Face normal in eye space:
    vec3 normal = normalize(eyeSpaceNormal.xyz);
    vec3 incident = normalize(eyeSpacePosition.xyz);

    // Apply heightmap:
    if (hasTexHeight) {
//        float s_d = 1.0 / 640;
//        float t_d = 1.0 / 640;
//        float top = texture(texHeight, Texcoord + vec2(0.0, t_d)).r;
//        float bottom = texture(texHeight, Texcoord + vec2(0.0, -t_d)).r;
//        float left = texture(texHeight, Texcoord + vec2(-s_d, 0.0)).r;
//        float right = texture(texHeight, Texcoord + vec2(s_d, 0.0)).r;
//
////        vec2 delH = vec2(left - right, bottom - top) / 2.0;
//        vec3 delH = normalize(vec3(left - right, bottom - top, 2.0));
//
////        float sample = texture(texHeight, Texcoord).x;
////        float dHdX = dFdx(sample);
////        float dHdY = dFdy(sample);
//        vec2 dSTdX = dFdx(Texcoord.st);
//        vec2 dSTdY = dFdy(Texcoord.st);
//        vec2 delS = vec2(dSTdX.s, dSTdY.s) / s_d;
//        vec2 delT = vec2(dSTdX.t, dSTdY.t) / t_d;
//
//        vec3 disp = delS * delH.s + delT * delH.t;
//
////        normal = vec3(disp, 0);
//        normal = normalize(vec3(disp, 0) + normal);
        normal = perturb_normal(normal, incident, Texcoord);
    }

    // Normal:
    outNormal.xy = normal.xy;

    // Environment map reflection:
    outEnvMapColSpecIntensity.rgba = vec4(0, 0, 0, 0);

    if (shininess > 0) {
        vec3 viewReflect = reflect(incident, normal);
//        float phongViewSpecular = phong(incident, viewReflect, shininess);
        vec4 sampleCoord = viewInverse * vec4(viewReflect, 0);
        sampleCoord = vec4(sampleCoord.x, sampleCoord.z, -sampleCoord.y, 1);
        vec4 reflectCol = texture(texEnvironmentMap, sampleCoord.xyz);
//        outEnvMapColSpecIntensity.rgb = reflectCol.rgb * colSpecular; // * phongViewSpecular;

        // Based on: http://en.wikibooks.org/wiki/GLSL_Programming/Unity/Specular_Highlights_at_Silhouettes
        float fresnelFactor = pow(1.0 - max(0.0, dot(normal, -incident)), 2.0);
        vec3 fresnelCol = mix(vec3(0.1), vec3(1), fresnelFactor);
        outEnvMapColSpecIntensity.rgb = fresnelCol * reflectCol.rgb * colSpecular;
    } else {
        outEnvMapColSpecIntensity.rgba = vec4(0, 0, 0, 0);
    }

    // Diffuse albedo:
    vec4 texCol = texture(texDiffuse, Texcoord);
    outAlbedoRoughness.rgb = texCol.rgb * colDiffuse;

    // Roughness:
    outAlbedoRoughness.a = shininess / 8; // Map to [0, 1].
}
