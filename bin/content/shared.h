cconst WORK_GROUP_SIZE_X;
cconst WORK_GROUP_SIZE_Y;
cconst THREADS_PER_GROUP_X;
cconst THREADS_PER_GROUP_Y;
cconst WORK_GROUP_COUNT_X;
cconst WORK_GROUP_COUNT_Y;

cconst MAX_LIGHTS_PER_TILE;

cconst MSAA_COUNT;

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