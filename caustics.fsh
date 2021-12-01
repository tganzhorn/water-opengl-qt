#version 460 core
layout (location = 0) out vec3 FragColor;

in vec2 vUV;
layout (location = 0) uniform sampler2D normalsTexture;
layout (location = 1) uniform sampler2D simulationTexture;
uniform vec3 uLightPosition;
uniform vec3 uPlanePosition;
uniform vec3 uPlaneNormal;
uniform vec2 uRenderTexelSize;
uniform vec2 uSimTexelSize;

vec3 calculateSmoothNormal()
{
    vec3 smoothNormal = texture2D(normalsTexture, vUV).xyz;
    smoothNormal += texture2D(normalsTexture, vUV + vec2(uSimTexelSize.x, uSimTexelSize.y)).xyz;
    smoothNormal += texture2D(normalsTexture, vUV + vec2(uSimTexelSize.x, -uSimTexelSize.y)).xyz;
    smoothNormal += texture2D(normalsTexture, vUV + vec2(-uSimTexelSize.x, uSimTexelSize.y)).xyz;
    smoothNormal += texture2D(normalsTexture, vUV + vec2(-uSimTexelSize.x, -uSimTexelSize.y)).xyz;
    return smoothNormal / 5.0;
}

void main(void)
{
    // This shader is not even remotely accurate but it fakes caustics quite okay
    vec3 normal = calculateSmoothNormal();
    vec3 ld = normalize(uLightPosition - vec3(vUV, normal.b));
    vec3 r0 = vec3(vUV, normal.b);

    vec3 rd = refract(normal, ld, 0.5);
    float t = -dot(r0 - uPlanePosition, uPlaneNormal) / dot(rd, uPlaneNormal);
    vec2 rrUV = (r0 + rd * t).xy;
    float dr = length(fwidth(rrUV - vUV) - uRenderTexelSize);

    rd = refract(normal, ld, 0.55);
    t = -dot(r0 - uPlanePosition, uPlaneNormal) / dot(rd, uPlaneNormal);
    vec2 rgUV = (r0 + rd * t).xy;
    float dg = length(fwidth(rgUV - vUV));

    rd = refract(normal, ld, 0.6);
    t = -dot(r0 - uPlanePosition, uPlaneNormal) / dot(rd, uPlaneNormal);
    vec2 rbUV = (r0 + rd * t).xy;
    float db = length(fwidth(rbUV - vUV));

    FragColor = pow(
        max(vec3(0), vec3(dr, dg, db) - uRenderTexelSize.x),
        vec3(0.3)
    );
}
