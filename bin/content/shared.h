cconst WORK_GROUP_WIDTH;
cconst WORK_GROUP_HEIGHT;
cconst MAX_LIGHTS_PER_TILE;

cconst MSAA_COUNT;

struct LightData
{
    vec3 position;
    float strength;
    vec3 color;
    float padding;
};

struct TileLightData
{
    int start;
    int numberOfLights;
    ivec2 padding;
};