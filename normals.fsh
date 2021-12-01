#version 460 core

out vec3 FragColor;

in vec2 vUV;
layout (location = 0) uniform sampler2D simulationTexture;
uniform vec2 uSimTexelSize;
uniform float uNormalStrength;

void main(void)
{
    // Normal calculation with sobel filter
    float h00 = texture2D(simulationTexture, vUV - vec2(uSimTexelSize.x, uSimTexelSize.y)).r;
    float h01 = texture2D(simulationTexture, vUV - vec2(uSimTexelSize.x, 0.0f)).r;
    float h02 = texture2D(simulationTexture, vUV - vec2(uSimTexelSize.x, -uSimTexelSize.y)).r;

    float h10 = texture2D(simulationTexture, vUV - vec2(0.0f, uSimTexelSize.y)).r;
    float h11 = texture2D(simulationTexture, vUV).r;
    float h12 = texture2D(simulationTexture, vUV + vec2(0.0f, uSimTexelSize.y)).r;

    float h20 = texture2D(simulationTexture, vUV + vec2(uSimTexelSize.x, -uSimTexelSize.y)).r;
    float h21 = texture2D(simulationTexture, vUV + vec2(uSimTexelSize.x, 0.0f)).r;
    float h22 = texture2D(simulationTexture, vUV + vec2(uSimTexelSize.x, uSimTexelSize.y)).r;

    float dx = h00 + h01 * 2.0f + h02 - h20 - h21 * 2.0f - h22;
    float dy = h00 + h10 * 2.0f + h20 - h02 - h12 * 2.0f - h22;

    FragColor = normalize(vec3(dx, dy, uNormalStrength));
}
