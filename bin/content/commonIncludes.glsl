#include "shared.glsh"

layout(std140) buffer ScreenSize
{
    int screenWidth;
    int screenHeight;
};

////////////////////////////////////////////////////////////
// Global light buffer, contains every light in the world
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

////////////////////////////////////////////////////////////
// tileLightData keeps track of which indices in lightIndices affect a given tile

// Memory is "allocated" by increasing (atomically) occupiedIndices
// Accessed once to increase occupiedIndices and then again to put lights into lightIndices
layout(std430) buffer LightIndices
{
    int occupiedIndices; // Needs to be initialized to 0!
    int lightIndices[];
};

struct TileLightData
{
    int start;
    int numberOfLights;
    ivec2 padding;
};

// Accessed once per tile
layout(std430) buffer TileLights
{
    TileLightData tileLightData[];
};

////////////////////////////////////////////////////////////
// Maps from a given pixel to an index in tileLightData

// Accessed once per pixel
layout(std430) buffer PixelToTile
{
    int startIndex[];
};