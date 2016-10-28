#version 330

uniform float r;
uniform float g;
uniform float b;
uniform float a;

out vec4 outColor;

void main()
{
    outColor = vec4(r, g, b, a);
}