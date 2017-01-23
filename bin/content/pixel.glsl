#version 330

uniform sampler2D tex;

in vec2 TexCoord;

out vec4 outColor;

void main()
{
    outColor = texture(tex, TexCoord);//vec4(color, color, color, 1.0f);
    //outColor = vec4(TexCoord, 1.0f, 1.0f);
}