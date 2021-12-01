#version 460 core

const float PI = 3.14159265359; // We have to use consts since QT Creator isnt smart enough for defines

out vec4 FragColor;

in vec2 vUV;
layout (location = 0) uniform sampler2D simulationTexture;
layout (location = 1) uniform sampler2D albedoTexture;
layout (location = 2) uniform sampler2D environmentTexture;
layout (location = 3) uniform sampler2D normalsTexture;
layout (location = 4) uniform sampler2D causticsTexture;
uniform float uLightStrength;
uniform vec3 uLightPosition;
uniform vec3 uPlanePosition;
uniform vec3 uPlaneNormal;
uniform vec3 uCameraPosition;
uniform vec2 uRenderTexelSize;

vec3 calculateSmoothNormal()
{
    vec3 smoothNormal = texture2D(normalsTexture, vUV).xyz;
    smoothNormal += texture2D(normalsTexture, vUV + vec2(uRenderTexelSize.x, uRenderTexelSize.y)).xyz;
    smoothNormal += texture2D(normalsTexture, vUV + vec2(uRenderTexelSize.x, -uRenderTexelSize.y)).xyz;
    smoothNormal += texture2D(normalsTexture, vUV + vec2(-uRenderTexelSize.x, uRenderTexelSize.y)).xyz;
    smoothNormal += texture2D(normalsTexture, vUV + vec2(-uRenderTexelSize.x, -uRenderTexelSize.y)).xyz;
    return smoothNormal / 5.0;
}

float calculateAttenuation(in vec3 fragPos, in vec3 lightPos)
{
    float distance = length(lightPos - fragPos);
    return 1.0 / (distance * distance);
}

float DistributionGGX(vec3 N, vec3 H, float a)
{
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom    = a2;
    float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
    denom        = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float k)
{
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float k)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, k);
    float ggx2 = GeometrySchlickGGX(NdotL, k);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// Refraction calculation using plane ray intersection
vec3 refraction(in vec3 V, in vec3 normal, in vec3 p0, in vec3 pn, in float h0)
{
    // RED light
    vec3 r0 = vec3(vUV, h0); // Ray origin
    vec3 rd = refract(normal, V, 0.5);
    float t = -dot(r0 - p0, pn) / dot(rd, pn);
    vec2 rrUV = (r0 + rd * t).xy;

    //GREEN light
    rd = refract(normal, V, 0.55);
    t = -dot(r0 - p0, pn) / dot(rd, pn);
    vec2 rgUV = (r0 + rd * t).xy;

    //BLUE light
    rd = refract(normal, V, 0.6);
    t = -dot(r0 - p0, pn) / dot(rd, pn);
    vec2 rbUV = (r0 + rd * t).xy;

    return vec3(
                texture2D(albedoTexture, rrUV).r,
                texture2D(albedoTexture, rgUV).g,
                texture2D(albedoTexture, rbUV).b
    );
}

void main(void)
{       
    vec3 cameraPos = uCameraPosition;
    vec3 imagePlane = vec3(vUV, 0);
    vec3 lightColor = vec3(1, 1, 1);
    vec3 lightPos = uLightPosition;
    float lightStrength = uLightStrength;
    vec3 wi = normalize(lightPos - imagePlane);

    vec3 N = calculateSmoothNormal(); // Normal
    vec3 V = normalize(imagePlane - cameraPos); // View vector

    float cosTheta = max(dot(N, wi), 0.0);
    float attenuation = calculateAttenuation(imagePlane, lightPos) * lightStrength;
    vec3 radiance = lightColor * attenuation + texture2D(causticsTexture, vUV).rgb * 1;

    vec3 F0 = vec3(0.02); // F0 value for water

    vec3 R = fresnelSchlick(cosTheta, F0); // Reflection term
    vec3 T = 1 - R; // Transmission term

    vec3 L = normalize(lightPos - imagePlane); // Light direction
    vec3 H = normalize(V + L); // Halfway vector

    float NDF = DistributionGGX(N, H, 0.02);
    float G = GeometrySmith(N, V, L, 0.02);

    vec3 numerator = NDF * G * vec3(0.02);
    float denom = 4.0 * max(dot(N,V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denom;

    vec3 kS = R;
    vec3 kD = vec3(1.0) - kS;

    // Refraction
    float h0 = N.z * 0.25;
    vec3 p0 = uPlanePosition;
    vec3 pn = uPlaneNormal;

    vec3 refractionColor = refraction(V, N, p0, pn, h0);

    // Reflection
    vec3 reflectN = normalize(reflect(N, vec3(1, 0, 0) - vec3(0, vUV)));
    vec3 reflectionColor = texture2D(environmentTexture, reflectN.xy * 0.5 + 0.5).rgb;
    vec3 albedo = mix(refractionColor, reflectionColor, R); // Correct?!

    // Sum colors
    float NdotL = max(dot(N, L), 0.0);
    vec3 Lo = (kD * albedo / PI + specular) * radiance * NdotL;
    vec3 ambient = vec3(0.1) * albedo;
    vec3 color = Lo + ambient;

    //color = color / (color + vec3(1.0));
    //color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1);
    //FragColor = vec4(texture2D(causticsTexture, vUV).rgb, 1);
}
