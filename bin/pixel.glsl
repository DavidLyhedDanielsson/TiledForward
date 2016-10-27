#version 330

uniform vec3 colorAttrib;

in vec3 vertexColor;

out vec4 outColor;

void main()
{
    outColor = vec4(colorAttrib, 1.0f);
}