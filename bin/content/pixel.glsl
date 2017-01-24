#version 330

uniform sampler2D tex;

in vec3 Normal;
in vec2 TexCoord;

out vec4 outColor;

const vec3 LIGHT_DIR = normalize(vec3(0.1f, 1.0f, 0.3f));

void main()
{
    outColor = vec4(texture(tex, TexCoord).xyz * clamp(dot(Normal, LIGHT_DIR) + 0.3f, 0.0f, 1.0f), 1.0f);//vec4(color, color, color, 1.0f);
    //outColor = vec4(TexCoord, 1.0f, 1.0f);
}