#version 460 core

layout (location = 0) in vec3 aPosition;

out vec2 vUV;

void main(void)
{
    gl_Position = vec4(aPosition, 1.0);
    vUV = aPosition.xy * 0.5 + 0.5;
}
