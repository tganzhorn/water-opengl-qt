#version 460 core

out vec4 FragColor;

in vec2 vUV;
in vec2 vTexCoords;
layout (location = 0) uniform sampler2D simulationTexture;
layout (location = 1) uniform sampler2D albedoTexture;
layout (location = 2) uniform sampler2D environmentTexture;
layout (location = 3) uniform sampler2D normalsTexture;
uniform vec3 uTexelSize;

// Refraction calculation using plane ray intersection
vec3 refraction(in vec3 normal, in vec3 p0, in vec3 pn)
{
    // RED light
    vec3 r0 = vec3(vUV, 0.0f); // Ray origin
    vec3 rd = refract(normal, vec3(0, 0, -1), 0.5);

    float t = -dot(r0 - p0, pn) / dot(rd, pn);
    vec2 rrUV = (r0 + rd * t).xy;

    //GREEN light
    rd = refract(normal, vec3(0, 0, -1), 0.55);
    t = -dot(r0 - p0, pn) / dot(rd, pn);
    vec2 rgUV = (r0 + rd * t).xy;

    //BLUE light
    rd = refract(normal, vec3(0, 0, -1), 0.6);
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
    vec3 normal = texture2D(normalsTexture, vUV).xyz;
    vec3 lightPos = normalize(vec3(0.5, 0.5, 1) - vec3(vUV, 0));

    vec3 p0 = vec3(0, 0, -0.1f);
    vec3 pn = vec3(0, 0, 1);

    vec3 refractionColor = refraction(normal, p0, pn);


    // Super cheap shadow calculation
    float reflectionTerm = clamp(dot(lightPos, normal), 0, 1);
    float shadow = clamp(reflectionTerm * 0.3 + 0.7, 0, 1);

    float specular = clamp(dot(lightPos, reflect(normal, vec3(0, 0, -1))), 0, 1);

    float lightTerm = clamp(shadow + specular, 0, 1);

    vec3 waterColor = clamp(vec3(0.24, 0.66, 0.95) * 2., vec3(0), vec3(1));

    vec3 N = normalize(reflect(normal, vec3(1, 0, 0) - vec3(0, vUV)));
    vec3 V = vec3(0, 0, 1) - vec3(vUV, 0);
    vec3 reflectionColor = texture2D(environmentTexture, N.xy * 0.5 + 0.5).rgb;

    vec3 color = refractionColor * lightTerm * (reflectionTerm) + reflectionColor * (1 - reflectionTerm);

    FragColor = vec4(color, 1);
    //FragColor = vec4(vec3(causticTermR, causticTermG, causticTermB), 1);
    //FragColor = texture2D(simulationTexture, vUV);
}
