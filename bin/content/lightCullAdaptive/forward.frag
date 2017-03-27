#version 450 core

#include "commonIncludes.glsl"
#include "tree.glsl"

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

#include "tileLights.glsl"

layout(std430) buffer ColorBuffer
{
    vec4 colors[];
};

vec2 ProjectedToTexel(vec2 projectedPosition)
{
    return vec2((projectedPosition.x + 1.0f) * 0.5f * screenWidth, (projectedPosition.y + 1.0f) * 0.5f * screenHeight);
}

void main()
{
    vec4 projectedPosition = viewProjectionMatrix * vec4(WorldPosition, 1.0f);
    vec2 texel = ProjectedToTexel(projectedPosition.xy / projectedPosition.w);

    const int arrayIndex = GetTreeDataScreen(int(texel.x), int(texel.y));

    vec3 textureColor = texture(tex, TexCoord).xyz;
    int index = arrayIndex / MAX_LIGHTS_PER_TILE;
    //float checkersColor = 1.0f;

    if(arrayIndex >= 0)
        //outColor = vec4(vec4(textureColor * 0.5f + vec3(checkersColor) * 0.5f, 1.0f));
        outColor = vec4(vec4(textureColor, 1.0f) * 0.5f + colors[index] * 0.5f);
    else
        outColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);

    /*const TileLightData data = GetTileLightData(arrayIndex);

    const int lightStart = data.start;
    const int lightCount = data.numberOfLights;

    vec3 finalColor = vec3(0.0f);
    vec3 textureColor = texture(tex, TexCoord).xyz;

    if(lightCount > MAX_LIGHTS_PER_TILE)
        finalColor = vec3(1.0f, 0.0f, 0.0f);
    else
    {
        for(int i = lightStart; i < lightStart + lightCount; ++i)
        {
            //LightData light = lights[0];
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

    outColor = vec4(finalColor, materials[materialIndex].opacity);*/
}