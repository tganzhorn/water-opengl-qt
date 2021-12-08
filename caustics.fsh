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

float signedLength(in vec2 vec)
{
    float l = length(vec);
    if (sign(vec.x) < 0 || sign(vec.y) < 0)
        return l;
    return -l;
}

void main(void)
{
    // This shader is not even remotely accurate but it fakes caustics quite okay
    vec3 normal = calculateSmoothNormal();
    vec3 ld = normalize(uLightPosition - vec3(vUV, normal.b));
    vec3 r0 = vec3(vUV, normal.b);

    vec3 rd = refract(normal, ld, 0.69);
    float tr = -dot(r0 - uPlanePosition, uPlaneNormal) / dot(rd, uPlaneNormal);
    vec2 rrUV = (r0 + rd * tr).xy;
    vec2 rp = rrUV - vUV;
    float dr = signedLength(dFdx(rp) + dFdy(rp));

    rd = refract(normal, ld, 0.74);
    float tg = -dot(r0 - uPlanePosition, uPlaneNormal) / dot(rd, uPlaneNormal);
    vec2 rgUV = (r0 + rd * tg).xy;
    vec2 gp = rgUV - vUV;
    float dg = signedLength(dFdx(gp) + dFdy(gp));

    rd = refract(normal, ld, 0.79);
    float tb = -dot(r0 - uPlanePosition, uPlaneNormal) / dot(rd, uPlaneNormal);
    vec2 rbUV = (r0 + rd * tb).xy;
    vec2 bp = rbUV - vUV;
    float db = signedLength(dFdx(bp) + dFdy(bp));

    FragColor = vec3(dr, dg, db);
}
