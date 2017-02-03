#version 330

uniform sampler2D tex;
uniform int materialIndex;

in vec3 WorldPosition;
in vec3 Normal;
in vec2 TexCoord;

out vec4 outColor;

const int MAX_MATERIALS = 64;
const float AMBIENT_STRENGTH = 0.6f;

struct Material
{
    vec3 ambientColor;
    float specularExponent;
    vec3 diffuseColor;
    float opacity;
};

layout (std140) uniform Materials
{
    Material materials[MAX_MATERIALS];
};

layout (std140) uniform LightData
{
    vec3 lightPosition;
    float padding0;
    vec3 lightColor;
    float padding1;
};

void main()
{
    vec3 lightDirection = WorldPosition - lightPosition;
    float lightDistance = length(lightDirection);
    lightDirection = normalize(lightDirection);

    lightDistance = max(lightDistance, 0.001f); // Prevent division by zero
    float attenuation = min(1.0f, 1.0f / (0.5f * lightDistance + 0.0f * lightDistance * lightDistance));

    float diffuse = max(dot(-lightDirection, Normal), 0.0f);

    float lightFactor = attenuation + diffuse;

    vec3 diffuseColor = materials[materialIndex].diffuseColor * diffuse * attenuation;
    vec3 ambientColor = materials[materialIndex].ambientColor * AMBIENT_STRENGTH;

    vec3 textureColor = texture(tex, TexCoord).xyz;

    outColor = vec4(textureColor * ambientColor + textureColor * diffuseColor, materials[materialIndex].opacity);
}