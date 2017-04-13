#version 330 core

uniform mat4 projectionMatrix;
uniform mat4 worldViewMatrix;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

out vec3 ViewPosition;
out vec3 Normal;
out vec2 TexCoord;

void main()
{
    vec4 tempViewPosition = worldViewMatrix * vec4(position, 1.0f);
    vec4 projectedPosition = projectionMatrix * tempViewPosition;

    gl_Position = projectedPosition;

    ViewPosition = tempViewPosition.xyz;
    Normal = (worldViewMatrix * vec4(normal, 0.0f)).xyz;
    TexCoord = texCoord;
}