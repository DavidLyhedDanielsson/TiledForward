#version 330

in vec4 Color;

out vec4 outColor;

void main()
{
    outColor = vec4(Color.xyz, 1.0f);
}