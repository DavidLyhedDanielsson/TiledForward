cconst WORK_GROUP_WIDTH;
cconst WORK_GROUP_HEIGHT;
cconst MAX_LIGHTS_PER_TILE;

cconst MSAA_COUNT;

cconst WORK_GROUP_COUNT_X;
cconst WORK_GROUP_COUNT_Y;

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

int GetArrayIndex(int gridX, int gridY)
{
    return gridY * WORK_GROUP_COUNT_X + gridX;
}

int GetArrayIndex(ivec2 grid)
{
    return GetArrayIndex(grid.x, grid.y);
}

int GetArrayIndex(uvec2 grid)
{
    return GetArrayIndex(int(grid.x), int(grid.y));
}

int GetArrayIndex(vec2 grid)
{
    return GetArrayIndex(int(grid.x), int(grid.y));
}