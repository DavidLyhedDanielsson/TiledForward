#include "shared.glsh"

layout(std140) buffer Lights
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
    int lightIndices[];
};

// Accessed once per tile
layout(std140) buffer TileLights
{
    TileLightData tileLightData[23][40];
};