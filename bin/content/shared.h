cconst WORK_GROUP_SIZE_X;
cconst WORK_GROUP_SIZE_Y;
cconst THREADS_PER_GROUP_X;
cconst THREADS_PER_GROUP_Y;
cconst WORK_GROUP_COUNT_X;
cconst WORK_GROUP_COUNT_Y;

cconst MAX_LIGHTS_PER_TILE;

cconst MSAA_COUNT;

int GetArrayIndex(int gridX, int gridY, int workGroupCount)
{
    return gridY * workGroupCount + gridX;
}

int GetArrayIndex(ivec2 grid, int workGroupCount)
{
    return GetArrayIndex(grid.x, grid.y, workGroupCount);
}

int GetArrayIndex(uvec2 grid, int workGroupCount)
{
    return GetArrayIndex(int(grid.x), int(grid.y), workGroupCount);
}

int GetArrayIndex(vec2 grid, int workGroupCount)
{
    return GetArrayIndex(int(grid.x), int(grid.y), workGroupCount);
}