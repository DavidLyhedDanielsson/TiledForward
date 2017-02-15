#version 330 core

in vec2 outTexCoord;
in vec4 outColor;

out vec4 finalColor;

uniform sampler2D tex;

void main()
{
    finalColor = texture(tex, outTexCoord) * outColor;
}