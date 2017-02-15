#version 330 core

uniform mat4 viewProjectionMatrix;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

layout(location = 3) in vec4 color;
layout(location = 4) in mat4 worldMatrix;

out vec4 Color;

void main()
{
    gl_Position = viewProjectionMatrix * worldMatrix * vec4(position, 1.0f);

    Color = color;
}