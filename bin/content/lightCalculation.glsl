void CalculateLighting(vec3 worldPosition, vec3 normal, vec3 lightPosition, float lightStrength, out float attenuation, out float diffuse)
{
    vec3 lightDirection = worldPosition - lightPosition;
    float lightDistance = length(lightDirection);

    if(lightDistance > lightStrength)
    {
        attenuation = 0.0f;
        diffuse = 0.0f;
    }
    else
    {
        ////////////////////////////////////////////////////////////
        // "Realistic" lighting
        /*float linearFactor = 2.0f / lightStrength;
        float quadraticFactor = 1.0f / (lightStrength * lightStrength);

        float tempAttenuation = 1.0f / (1.0f + linearFactor * lightDistance + quadraticFactor * lightDistance * lightDistance);
        tempAttenuation *= max((lightStrength - lightDistance) / lightStrength, 0.0f);

        attenuation = tempAttenuation;*/

        ////////////////////////////////////////////////////////////
        // Squared lighting
        attenuation = 1.0f - (lightDistance / lightStrength) * (lightDistance / lightStrength);

        ////////////////////////////////////////////////////////////
        // Diffuse
        // Always the same
        lightDirection = normalize(lightDirection);
        diffuse = max(dot(-lightDirection, normalize(normal)), 0.0f);
    }
}