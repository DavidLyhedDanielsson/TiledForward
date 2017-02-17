#version 330

uniform mat4 viewProjectionMatrix;

in vec3 position;
in vec3 color;

out vec3 outColor;

void main()
{
    gl_Position = viewProjectionMatrix * vec4(position, 1.0f);

    outColor = color;
}