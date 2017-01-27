#version 330

uniform sampler2D tex;

in vec3 WorldPosition;
in vec3 Normal;
in vec2 TexCoord;

out vec4 outColor;

layout (std140) uniform LightData
{
    vec3 lightPosition;
    vec3 lightColor;
};

void main()
{
    vec3 lightDir = normalize(WorldPosition - lightPosition);

    outColor = vec4(texture(tex, TexCoord).xyz * clamp(dot(Normal, -lightDir) + 0.3f, 0.0f, 1.0f), 1.0f);
}