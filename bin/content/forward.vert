#version 330

uniform mat4 viewProjectionMatrix;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

out vec3 WorldPosition;
out vec3 Normal;
out vec2 TexCoord;

void main()
{
    gl_Position = viewProjectionMatrix * vec4(position * 0.01f, 1.0f);

    WorldPosition = position * 0.01f;
    Normal = normal;
    TexCoord = texCoord;
}