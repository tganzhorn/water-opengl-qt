#version 460 core
layout (location = 0) out vec4 FragColor;

in vec2 vUV;
in vec2 vTexCoords;
uniform float uTime;
uniform vec2 uResolution;
uniform sampler2D texture;
uniform vec3 uTexelSize;

void main(void)
{
    vec4 central = texture2D(texture, vUV);

    float k = 0.9;
    float average = (central.r + k * (
        texture2D(texture, vUV - vec2(0, uTexelSize.y)).r +
        texture2D(texture, vUV + vec2(0, uTexelSize.y)).r +
        texture2D(texture, vUV - vec2(uTexelSize.x, 0)).r +
        texture2D(texture, vUV + vec2(uTexelSize.x, 0)).r
    ) * 0.25) / (1.0 + k);

    central.g += (average - central.r) * 2.0;
    central.g *= 0.99;
    central.r += central.g;

    FragColor = central;
    //FragColor = texture2D(texture, vUV);
}
