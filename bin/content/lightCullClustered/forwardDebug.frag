#version 450 core

uniform int tileCountX;

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

#include "tileLights.glsl"

vec2 ProjectedToTexel(vec2 projectedPosition)
{
    return vec2((projectedPosition.x + 1.0f) * 0.5f * screenWidth, (projectedPosition.y + 1.0f) * 0.5f * screenHeight);
}

void main()
{
    vec4 projectedPosition = viewProjectionMatrix * vec4(WorldPosition, 1.0f);
    vec2 texel = ProjectedToTexel(projectedPosition.xy / projectedPosition.w);

    int tileX = int(texel.x / float(screenWidth));
    int tileY = int(texel.y / float(screenHeight));

    Lightdata light = finalLights[tileY * tileCountX + tileX];

    vec3 finalColor = vec3(light.color);
    vec3 textureColor = texture(tex, TexCoord).xyz;

    finalColor += ambientStrength;
    finalColor *= materials[materialIndex].diffuseColor;
    finalColor *= textureColor;

    outColor = vec4(finalColor, materials[materialIndex].opacity);
}