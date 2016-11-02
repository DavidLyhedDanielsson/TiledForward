#version 330

uniform float colors[3];
uniform float singleColor;
uniform int intColor;

out vec4 outColor;

void main()
{
    outColor = vec4(colors[0], singleColor, intColor / 255.0f, 1.0f);
}