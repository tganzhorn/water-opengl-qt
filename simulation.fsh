#version 460 core
layout (location = 0) out vec4 FragColor;

in vec2 vUV;
//uniform float uTime;
uniform vec2 uResolution;
uniform sampler2D texture;
uniform vec2 uSimTexelSize;
uniform float uK;
uniform float uDamping;
uniform int uSimulationFreeze;

void main(void)
{
    vec4 color = texture2D(texture, vUV);

    float average = (color.r + uK * (
        texture2D(texture, vUV - vec2(0, uSimTexelSize.y)).r +
        texture2D(texture, vUV + vec2(0, uSimTexelSize.y)).r +
        texture2D(texture, vUV - vec2(uSimTexelSize.x, 0)).r +
        texture2D(texture, vUV + vec2(uSimTexelSize.x, 0)).r
    ) * 0.25) / (1.0 + uK);

    vec4 central = color;

    central.g += (average - central.r) * 2.0;
    central.g *= uDamping;
    central.r += central.g;

    FragColor = (1 - uSimulationFreeze) * central + uSimulationFreeze * color;
    //FragColor = texture2D(texture, vUV);
}
