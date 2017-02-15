#version 330

uniform mat4 viewProjectionMatrix;
uniform mat4 worldMatrix;

in vec3 position;

void main()
{
    gl_Position = viewProjectionMatrix * worldMatrix * vec4(position, 1.0f);
}