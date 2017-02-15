#version 330 core

uniform mat4 viewProjectionMatrix;
uniform mat4 worldMatrix;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

out vec3 WorldPosition;
out vec3 Normal;
out vec2 TexCoord;

void main()
{
    vec4 tempWorldPosition = worldMatrix * vec4(position, 1.0f);

    gl_Position = viewProjectionMatrix * tempWorldPosition;

    WorldPosition = tempWorldPosition.xyz;
    Normal = normal;
    TexCoord = texCoord;
}