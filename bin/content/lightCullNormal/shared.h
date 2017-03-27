cconst THREADS_PER_GROUP_X;
cconst THREADS_PER_GROUP_Y;

cconst MAX_LIGHTS_PER_TILE;

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
    return gridY * 256 + gridX; // TODO
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