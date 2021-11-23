#version 460 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) uniform float uTime;
layout (location = 2) uniform vec2 uPosition;

out vec2 vUV;
out vec2 vTexCoords;

void main(void)
{
    gl_Position = vec4(aPosition, 1.0);
    vUV = aPosition.xy * 0.5 + 0.5;
}
