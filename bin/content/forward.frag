#version 450 core

#define cppint(a) const int a = 1;

cppint(WORK_GROUP_WIDTH)
cppint(WORK_GROUP_HEIGHT)
cppint(MAX_LIGHTS_PER_TILE)

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

layout (std140) buffer Lights
{
    vec3 padding;
    float ambientStrength;
    LightData lights[];
};

// Memory is "allocated" by increasing (atomically) occupiedIndices
// Accessed once to increase occupiedIndices and then again to put lights into lightIndices
layout(std140) buffer LightIndices
{
    int occupiedIndices; // Needs to be initialized to 0!
    int lightIndices[40 * 23 * MAX_LIGHTS_PER_TILE];
};

struct TileLightData
{
    int start;
    int numberOfLights;
    ivec2 padding;
};

// Accessed once per tile
layout(std140) buffer TileLights
{
    TileLightData tileLightData[23][40];
};

layout(std140) buffer ScreenSize
{
    int screenWidth;
    int screenHeight;
};

vec2 ProjectedToTexel(vec2 projectedPosition)
{
    return vec2((projectedPosition.x + 1.0f) * 0.5f * 1280, (projectedPosition.y + 1.0f) * 0.5f * 720);
}

ivec2 TexelToGrid(vec2 texelPosition)
{
    return ivec2(texelPosition.x / 32, texelPosition.y / 32);
}

void main()
{
    vec3 textureColor = texture(tex, TexCoord).xyz;

    vec3 finalColor = vec3(0.0f);

    vec4 projectedPosition = viewProjectionMatrix * vec4(WorldPosition, 1.0f);

    vec2 texel = ProjectedToTexel(projectedPosition.xy / projectedPosition.w);
    vec2 gridIndex = TexelToGrid(texel);

    int lightStart = tileLightData[int(gridIndex.y)][int(gridIndex.x)].start;
    int lightCount = tileLightData[int(gridIndex.y)][int(gridIndex.x)].numberOfLights;
    for(int i = lightStart; i < lightStart + lightCount; ++i)
    {
        LightData light = lights[lightIndices[i]];

        vec3 lightDirection = WorldPosition - light.position;
        float lightDistance = length(lightDirection);

        if(lightDistance > light.strength)
            continue;

        lightDirection = normalize(lightDirection);

        float linearFactor = 2.0f / light.strength;
        float quadraticFactor = 1.0f / (light.strength * light.strength);

        float attenuation = 1.0f / (1.0f + linearFactor * lightDistance + quadraticFactor * lightDistance * lightDistance);
        attenuation *= max((light.strength - lightDistance) / light.strength, 0.0f);

        //finalColor += light.color * diffuse * attenuation;

        float diffuse = max(dot(-lightDirection, normalize(Normal)), 0.0f);
        finalColor += light.color * diffuse * attenuation;
    }

    finalColor += ambientStrength;
    finalColor *= materials[materialIndex].diffuseColor;
    finalColor *= textureColor;

    outColor = vec4(finalColor, 1.0f);
}