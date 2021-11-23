#version 460 core

out vec4 FragColor;

in vec2 vUV;
in vec2 vTexCoords;
layout (location = 0) uniform sampler2D simulationTexture;
layout (location = 1) uniform sampler2D albedoTexture;
layout (location = 2) uniform sampler2D environmentTexture;
uniform vec3 uTexelSize;

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main(void)
{       
    float height = texture2D(simulationTexture, vUV).r;

    // Normal calculation with sobel filter
    float h00 = texture2D(simulationTexture, vUV - vec2(uTexelSize.x, uTexelSize.y)).r;
    float h01 = texture2D(simulationTexture, vUV - vec2(uTexelSize.x, 0.0f)).r;
    float h02 = texture2D(simulationTexture, vUV - vec2(uTexelSize.x, -uTexelSize.y)).r;

    float h10 = texture2D(simulationTexture, vUV - vec2(0.0f, uTexelSize.y)).r;
    float h12 = texture2D(simulationTexture, vUV + vec2(0.0f, uTexelSize.y)).r;

    float h20 = texture2D(simulationTexture, vUV + vec2(uTexelSize.x, -uTexelSize.y)).r;
    float h21 = texture2D(simulationTexture, vUV + vec2(uTexelSize.x, 0.0f)).r;
    float h22 = texture2D(simulationTexture, vUV + vec2(uTexelSize.x, uTexelSize.y)).r;

    float dx = h00 + h01 * 2.0f + h02 - h20 - h21 * 2.0f - h22;
    float dy = h00 + h10 * 2.0f + h20 - h02 - h12 * 2.0f - h22;

    vec3 normal = normalize(vec3(dx, dy, 0.5f));

    // Refraction calculation using plane ray intersection
    // RED light
    vec3 r0 = vec3(vUV, 0.0f); // Ray origin
    vec3 rd = refract(normal, vec3(0, 0, -1), 0.5);
    vec3 p0 = vec3(0, 0, -0.1f);
    vec3 pn = vec3(0, 0, 1);

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

    vec3 lightPos = normalize(vec3(0.5, 0.5, 1) - vec3(vUV, 0));

    // Super cheap shadow calculation
    float shadow = clamp(dot(lightPos, normal), 0, 1);
    shadow = clamp(shadow * 0.3 + 0.7, 0, 1);

    float specular = clamp(dot(lightPos, reflect(normal, vec3(0, 0, -1))), 0, 1);

    float lightTerm = clamp(shadow + specular, 0, 1);

    vec3 albedoColor = vec3(
                texture2D(albedoTexture, rrUV).r,
                texture2D(albedoTexture, rgUV).g,
                texture2D(albedoTexture, rbUV).b
    );

    vec3 heightColor = vec3(height);

    vec3 waterColor = clamp(vec3(0.24, 0.66, 0.95) * 2., vec3(0), vec3(1));

    vec3 N = normalize(reflect(normal, vec3(1, 0, 0) - vec3(0, vUV)));
    vec3 V = vec3(0, 0, 1) - vec3(vUV, 0);
    vec3 reflectionColor = texture2D(environmentTexture, N.xy * 0.5 + 0.5).rgb;

    vec3 color = albedoColor * lightTerm + reflectionColor * 0.2;

    FragColor = vec4(color, 1);
    //FragColor = vec4(reflectionColor, 1);
}
