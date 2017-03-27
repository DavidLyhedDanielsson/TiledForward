#version 330 core

in vec2 position;
in vec2 texCoord;
in vec4 color;

out vec2 outTexCoord;
out vec4 outColor;

uniform mat4x4 viewProjMatrix;

void main()
{
    outTexCoord = texCoord;
    outColor = color;

    gl_Position = viewProjMatrix * vec4(position, 0.0f, 1.0f);
}