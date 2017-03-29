#version 450 core

#include "commonIncludes.glsl"

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

layout(std430) buffer ColorBuffer
{
    vec4 colors[];
};

vec2 ProjectedToTexel(vec2 projectedPosition)
{
    return vec2((projectedPosition.x + 1.0f) * 0.5f * screenWidth, (projectedPosition.y + 1.0f) * 0.5f * screenHeight);
}

ivec2 TexelToGrid(vec2 texelPosition)
{
    return ivec2(texelPosition.x / THREAD_GROUP_SIZE_X, texelPosition.y / THREAD_GROUP_SIZE_Y);
}

void main()
{
    vec3 textureColor = texture(tex, TexCoord).xyz;

    vec3 finalColor = vec3(0.0f);

    vec4 projectedPosition = viewProjectionMatrix * vec4(WorldPosition, 1.0f);

    vec2 texel = ProjectedToTexel(projectedPosition.xy / projectedPosition.w);
    vec2 gridIndex = TexelToGrid(texel);

    int arrayIndex = GetArrayIndex(gridIndex);

    int lightStart = tileLightData[arrayIndex].start;
    int lightCount = tileLightData[arrayIndex].numberOfLights;

    if(lightCount > MAX_LIGHTS_PER_TILE)
        finalColor = vec3(1.0f, 0.0f, 0.0f);
    else
    {
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

            float diffuse = max(dot(-lightDirection, normalize(Normal)), 0.0f);

            finalColor += light.color * diffuse * attenuation;
        }

        finalColor += ambientStrength;
        finalColor *= materials[materialIndex].diffuseColor;
        finalColor *= textureColor;
    }

    outColor = vec4(finalColor * 0.75f + colors[arrayIndex].xyz * 0.25f, materials[materialIndex].opacity);
}