#version 460 core
layout (location = 0) out vec4 FragColor;

in vec2 vUV;
in vec2 vTexCoords;
uniform float uTime;
uniform vec2 uResolution;
uniform sampler2D texture;
uniform vec2 uTexelSize;
uniform vec4 uSphere; // x (range 0-1), y (range 0-1), r (range 0-1), strength (range -Inf +Inf)

void main(void)
{
    vec3 color = texture2D(texture, vUV).xyz;

    float d = pow(clamp((1 - length(vUV - uSphere.xy) - (1 - uSphere.z)) / (uSphere.z), 0, 1), 4) * uSphere.w;
    FragColor = vec4(color + vec3(0, d, 0), 1);
}
