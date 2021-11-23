#version 460 core

layout (location = 0) out vec4 FragColor;

// Source for algorithm: https://www.researchgate.net/publication/299225104_Simulation_of_2D_Saint-Venant_equations_in_open_channel_by_using_MATLAB

in vec2 vUV;
in vec2 vTexCoords;
uniform float uTime;
uniform vec2 uResolution;
uniform sampler2D texture;
uniform vec3 uTexelSize;

void main(void)
{
    vec4 central = texture2D(texture, vUV);

    float k = 1.5;
    /*float average = (central.r + k * (
        texture2D(texture, vUV - vec2(uTexelSize.x, uTexelSize.y)).r +
        texture2D(texture, vUV - vec2(0.0f, uTexelSize.y)).r +
        texture2D(texture, vUV - vec2(-uTexelSize.x, uTexelSize.y)).r +

        texture2D(texture, vUV - vec2(uTexelSize.x, 0.0f)).r +
        texture2D(texture, vUV - vec2(-uTexelSize.x, 0.0f)).r +

        texture2D(texture, vUV + vec2(-uTexelSize.x, uTexelSize.y)).r +
        texture2D(texture, vUV + vec2(0.0f, uTexelSize.y)).r +
        texture2D(texture, vUV + vec2(uTexelSize.x, uTexelSize.y)).r
    ) * 0.125) / (1.0 + k);*/

    float average = 0.0f;

    for (float x = -1.0; x <= 1.0; x++) {
        for (float y = -1.0; y <= 1.0; y++) {
            if (x == 0 && y == 0) continue;
            average += texture2D(texture, vUV + vec2(uTexelSize.x * x, uTexelSize.y * y)).r;
        }
    }

    average = (central.r + k * average / 8) / (1.0f + k);

    central.g += (average - central.r) * 2.0;
    central.g *= 0.99;
    central.r += central.g;

    FragColor = central;
}
