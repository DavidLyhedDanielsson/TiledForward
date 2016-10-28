#version 330

uniform float colors[3];
uniform mat4x2 weirdMatrix;

out vec4 outColor;

void main()
{
    outColor = vec4(colors[0] * weirdMatrix[0][0], colors[1], colors[2], 1.0f);
}