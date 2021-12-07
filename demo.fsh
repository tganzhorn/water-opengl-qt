#version 460 core
layout (location = 0) out vec4 FragColor;

in vec2 vUV;
uniform sampler2D texture;
uniform bool uNormalize;
uniform float uMultiplier;

void main(void)
{
    if (uNormalize)
    {
        FragColor = vec4(normalize(texture2D(texture, vUV).rgb) * 0.5 + 0.5, 1);
    } else {
        FragColor = texture2D(texture, vUV) * max(1, uMultiplier);
    }
}
