cconst THREADS_PER_GROUP_X;
cconst THREADS_PER_GROUP_Y;

cconst THREAD_GROUP_SIZE_X;
cconst THREAD_GROUP_SIZE_Y;

cconst THREAD_GROUP_COUNT_X;
cconst THREAD_GROUP_COUNT_Y;

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
    return gridY * THREAD_GROUP_COUNT_X + gridX;
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