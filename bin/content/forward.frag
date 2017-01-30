#version 330

uniform sampler2D tex;

in vec3 WorldPosition;
in vec3 Normal;
in vec2 TexCoord;

out vec4 outColor;

layout (shared) uniform LightData
{
    vec3 lightPosition;
    vec3 lightColor[8];
};

void main()
{
    vec3 lightDirection = WorldPosition - lightPosition;
    float lightDistance = length(lightDirection);
    lightDirection = normalize(lightDirection);

    lightDistance = max(lightDistance, 0.001f); // Prevent division by zero
    float attenuation = min(1.0f, 1.0f / (0.5f * lightDistance + 0.0f * lightDistance * lightDistance));

    float diffuse = max(dot(-lightDirection, Normal), 0.0f);

    outColor = vec4(texture(tex, TexCoord).xyz * lightColor[1] * attenuation * diffuse, 1.0f);
}