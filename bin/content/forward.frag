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

const int MAX_LIGHTS = 64;

layout (std140) uniform Lights
{
    LightData lights[MAX_LIGHTS];
    float ambientStrength;
    int lightCount;
    vec2 padding;
};

void main()
{
    vec3 textureColor = texture(tex, TexCoord).xyz;

    vec3 finalColor = vec3(0.0f);

    for(int i = 0; i < lightCount; ++i)
    {
        vec3 lightDirection = WorldPosition - lights[i].position;
        float lightDistance = length(lightDirection);

        if(lightDistance > lights[i].strength)
            continue;

        lightDirection = normalize(lightDirection);

        float linearFactor = 2.0f / lights[i].strength;
        float quadraticFactor = 1.0f / (lights[i].strength * lights[i].strength);

        float attenuation = 1.0f / (1.0f + linearFactor * lightDistance + quadraticFactor * lightDistance * lightDistance);
        attenuation *= max((lights[i].strength - lightDistance) / lights[i].strength, 0.0f);

        float diffuse = max(dot(-lightDirection, normalize(Normal)), 0.0f);
        finalColor += lights[i].color * diffuse * attenuation;
    }

    finalColor += ambientStrength;
    finalColor *= materials[materialIndex].diffuseColor;

    outColor = vec4(textureColor * finalColor, materials[materialIndex].opacity);
}