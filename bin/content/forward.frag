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

struct LightData
{
    vec3 position;
    float strength;
    vec3 color;
    float padding;
};

const int MAX_LIGHTS = 4;

layout (std140) uniform Lights
{
    LightData lights[MAX_LIGHTS];
    float ambientStrength;
    vec3 padding;
};

void main()
{
    vec3 textureColor = texture(tex, TexCoord).xyz;
    vec3 finalColor = textureColor * ambientStrength;

    for(int i = 0; i < MAX_LIGHTS; ++i)
    {
        vec3 lightDirection = WorldPosition - lights[i].position;
        float lightDistance = length(lightDirection);
        lightDirection = normalize(lightDirection);

        lightDistance = max(lightDistance, 0.001f); // Prevent division by zero

        float linearFactor = 2.0f / lights[i].strength;
        float quadraticFactor = 1.0f / (lights[i].strength * lights[i].strength);

        float attenuation = min(1.0f, 1.0f / (1.0f + linearFactor * lightDistance + quadraticFactor * lightDistance * lightDistance));

        float diffuse = max(dot(-lightDirection, normalize(Normal)), 0.0f);

        vec3 diffuseColor = materials[materialIndex].diffuseColor * diffuse * attenuation;

        finalColor += textureColor * diffuseColor;
    }

    outColor = vec4(finalColor, materials[materialIndex].opacity);
}