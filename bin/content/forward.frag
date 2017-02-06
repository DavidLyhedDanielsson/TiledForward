#version 330

uniform sampler2D tex;
uniform int materialIndex;

in vec3 WorldPosition;
in vec3 Normal;
in vec2 TexCoord;

out vec4 outColor;

const int MAX_MATERIALS = 64;

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
    float lightStrength;
    vec3 lightColor;
    float ambientStrength;
};

void main()
{
    vec3 lightDirection = WorldPosition - lightPosition;
    float lightDistance = length(lightDirection);
    lightDirection = normalize(lightDirection);

    lightDistance = max(lightDistance, 0.001f); // Prevent division by zero

    float linearFactor = 2.0f / lightStrength;
    float quadraticFactor = 1.0f / (lightStrength * lightStrength);

    float attenuation = min(1.0f, 1.0f / (1.0f + linearFactor * lightDistance + quadraticFactor * lightDistance * lightDistance));

    float diffuse = max(dot(-lightDirection, Normal), 0.0f);

    vec3 diffuseColor = materials[materialIndex].diffuseColor * diffuse * attenuation;
    vec3 ambientColor = materials[materialIndex].ambientColor * ambientStrength;

    vec3 textureColor = texture(tex, TexCoord).xyz;

    outColor = vec4(textureColor * ambientColor + textureColor * diffuseColor + lightColor * 0.001f, materials[materialIndex].opacity);
}