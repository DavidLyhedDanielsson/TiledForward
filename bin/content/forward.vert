#version 330 core

uniform mat4 viewProjectionMatrix;
uniform mat4 worldMatrix;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

out vec3 WorldPosition;
out vec3 Normal;
out vec2 TexCoord;
out mat4 outMatrix;

void main()
{
    vec4 tempWorldPosition = worldMatrix * vec4(position, 1.0f);
    vec4 projectedPosition = viewProjectionMatrix * tempWorldPosition;

    gl_Position = projectedPosition;

    outMatrix = viewProjectionMatrix;
    WorldPosition = tempWorldPosition.xyz;
    Normal = normal;
    TexCoord = texCoord;
}