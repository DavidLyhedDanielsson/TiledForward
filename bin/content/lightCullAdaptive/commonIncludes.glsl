#include "shared.glsh"

layout(std430) buffer ScreenSize
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
