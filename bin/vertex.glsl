#version 330

uniform mat4 viewProjectionMatrix;

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;

layout(location = 2) in mat4 worldMatrix;

out vec2 TexCoord;

void main()
{
    gl_Position = viewProjectionMatrix * worldMatrix * vec4(position, 1.0f);

    TexCoord = texCoord;
}