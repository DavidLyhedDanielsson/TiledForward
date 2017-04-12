#version 450 core

#include "../lightCalculation.glsl"

layout(location = 0) uniform int tileCountX;

layout(std430) buffer ScreenSize
{
    int screenWidth;
    int screenHeight;
};

uniform mat4 viewProjectionMatrix;
uniform mat4 worldMatrix;

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

layout(std140) buffer FinalLights
{
    LightData[] finalLights;
};

struct LightData
{
    vec3 position;
    float strength;
    vec3 color;
    float padding;
};

layout(std430) buffer Lights
{
    vec3 padding;
    float ambientStrength;
    LightData lights[];
};


vec2 ProjectedToTexel(vec2 projectedPosition)
{
    return vec2((projectedPosition.x + 1.0f) * 0.5f * screenWidth, (projectedPosition.y + 1.0f) * 0.5f * screenHeight);
}

void main()
{
    vec4 projectedPosition = viewProjectionMatrix * vec4(WorldPosition, 1.0f);
    vec2 texel = ProjectedToTexel(projectedPosition.xy / projectedPosition.w);

    // tileCountX == tildCountY because it's a tree
    int tileX = int(texel.x / (screenWidth / float(tileCountX)));
    int tileY = int(texel.y / (screenHeight / float(tileCountX)));

    LightData light = finalLights[tileY * tileCountX + tileX];

    vec3 lightDirection = WorldPosition - light.position;
    float lightDistance = length(lightDirection);
    lightDirection = normalize(lightDirection);

    vec3 textureColor = texture(tex, TexCoord).xyz;
    vec3 finalColor = vec3(0.0f);

    float attenuation;
    float diffuse;

    CalculateLighting(WorldPosition, Normal, light.position, light.strength, attenuation, diffuse);

    finalColor += vec3(light.color) * attenuation * diffuse;
    //vec3 finalColor = vec3(0.0f);

    finalColor += ambientStrength;
    finalColor *= materials[materialIndex].diffuseColor;
    finalColor *= textureColor;

    outColor = vec4(finalColor, materials[materialIndex].opacity);
}