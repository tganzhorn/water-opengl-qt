#version 460 core
layout (location = 0) out vec4 FragColor;

in vec2 vUV;
//uniform float uTime;
uniform vec2 uResolution;
uniform sampler2D texture;
uniform vec2 uTexelSize;
uniform float uK;
uniform float uDamping;

void main(void)
{
    vec4 central = texture2D(texture, vUV);

    float average = (central.r + uK * (
        texture2D(texture, vUV - vec2(0, uTexelSize.y)).r +
        texture2D(texture, vUV + vec2(0, uTexelSize.y)).r +
        texture2D(texture, vUV - vec2(uTexelSize.x, 0)).r +
        texture2D(texture, vUV + vec2(uTexelSize.x, 0)).r
    ) * 0.25) / (1.0 + uK);

    central.g += (average - central.r) * 2.0;
    central.g *= uDamping;
    central.r += central.g;

    FragColor = central;
    //FragColor = texture2D(texture, vUV);
}
