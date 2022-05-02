#version 450 core

#include "commonIncludes.glsl"
#include "tree.glsl"
#include "../lightCalculation.glsl"

uniform mat4 projectionMatrix;

uniform sampler2D tex;
uniform int materialIndex;

in vec3 ViewPosition;
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
    vec4 projectedPosition = projectionMatrix * vec4(ViewPosition, 1.0f);
    vec2 texel = ProjectedToTexel(projectedPosition.xy / projectedPosition.w);

    const int arrayIndex = GetTreeDataScreen(int(texel.x), int(texel.y));

    vec3 finalColor = vec3(0.0f);
    vec3 textureColor = texture(tex, TexCoord).xyz;

    const TileLightData data = GetTileLightData(arrayIndex);

    const int lightStart = data.start;
    const int lightCount = data.numberOfLights;

    if(lightCount > 0)
    {
        for(int i = lightStart; i < lightStart + lightCount; ++i)
        {
            LightData light = lights[GetLightIndex(i)];

        float attenuation;
        float diffuse;

        CalculateLighting(ViewPosition, Normal, light.position, light.strength, attenuation, diffuse);

            finalColor += light.color * diffuse * attenuation;
        }

        finalColor += ambientStrength;
        finalColor *= materials[materialIndex].diffuseColor;
        finalColor *= textureColor;

        const int index = arrayIndex;
        //finalColor = vec3(finalColor * 0.5 + colors[index].xyz * 0.5f);
        finalColor = vec3(colors[index].xyz);
    }
    else
        finalColor = vec3(textureColor * ambientStrength);

    outColor = vec4(finalColor, materials[materialIndex].opacity);
}