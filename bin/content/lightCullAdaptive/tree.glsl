layout(std430) buffer Tree
{
    int tree[];
};

layout(std140) uniform TreeDepthData
{
    int treeStartDepth;
    int treeMaxDepth;
};

int GetTreeLinearIndex(int x, int y)
{
    int index = 0;

    for(int i = 0; i < 32; ++i)
    {
        int extractedBit;

        if(i % 2 == 0)
            extractedBit = ((x >> (i / 2)) & 1);
        else
            extractedBit = (y >> (i / 2)) & 1;

        index |= (extractedBit << i);
    }

    return index;
}

int GetTreeLinearIndex(int x, int y, int oldDepth, int newDepth)
{
    int oldWidth = screenWidth / int(pow(2, oldDepth));
    int oldHeight = screenHeight / int(pow(2, oldDepth));

    int oldScreenX = x * oldWidth;
    int oldScreenY = y * oldHeight;

    int newTileSizeX = screenWidth / int(pow(2, newDepth));
    int newTileSizeY = screenHeight / int(pow(2, newDepth));

    int newTileX = oldScreenX / newTileSizeX;
    int newTileY = oldScreenY / newTileSizeY;

    return GetTreeLinearIndex(newTileX, newTileY);
}

void PutTreeDataTile(int x, int y, int oldDepth, int newDepth, int data)
{
    int depthOffset = -1;
    for(int i = 0; i < newDepth; ++i)
        depthOffset += int(pow(4, i));

    int index = GetTreeLinearIndex(x, y, oldDepth, newDepth);

    tree[depthOffset + index] = data;
}

void PutTreeDataTile(int x, int y, int depth, int data)
{
    int depthOffset = -1;
    for(int i = 0; i < depth; ++i)
        depthOffset += int(pow(4, i));

    int index = GetTreeLinearIndex(x, y);

    tree[depthOffset + index] = data;
}

void PutTreeDataTile(int x, int y, int data)
{
    int depthOffset = -1;
    for(int i = 0; i < treeMaxDepth; ++i)
        depthOffset += int(pow(4, i));

    int index = GetTreeLinearIndex(x, y);

    tree[depthOffset + index] = data;
}

int GetTreeDataScreen(int screenX, int screenY)
{
    /*int lastIndex = -1;

    int depthOffset = 0;

    vec2 range = vec2(float(screenWidth), float(screenHeight));

    for(int i = 1; i < treeStartDepth; ++i)
    {
        range /= 2.0f;
        depthOffset += int(pow(4, i));
    }

    for(int i = treeStartDepth; i <= treeMaxDepth; ++i)
    {
        range /= 2.0f;

        int x = int(screenX / range.x);
        int y = int(screenY / range.y);

        int index = GetTreeLinearIndex(x, y);

        int potentialIndex = tree[depthOffset + index];
        if(potentialIndex < 0)
            return lastIndex;
        else
            lastIndex = potentialIndex;

        depthOffset += int(pow(4, i));
    }*/

    int depthOffset = 0;

    vec2 range = vec2(float(screenWidth), float(screenHeight)) / 2.0f;

    for(int i = 1; i < treeMaxDepth; ++i)
    {
        range /= 2.0f;
        depthOffset += int(pow(4, i));
    }

    int x = int(screenX / range.x);
    int y = int(screenY / range.y);

    int index = GetTreeLinearIndex(x, y);

    return tree[depthOffset + index];
}

int GetTreeDataGrid(int x, int y, int depth)
{
    int depthOffset = -1;

    for(int i = 0; i < depth; ++i)
        depthOffset += int(pow(4, i));

    int index = GetTreeLinearIndex(x, y);
    return tree[depthOffset + index];
}

void PutTreeDataScreen(uint x, uint y, int depth, int data)
{
    PutTreeDataScreen(int(x), int(y), depth, data);
}

void PutTreeDataScreen(ivec2 tile, int depth, int data)
{
    PutTreeDataScreen(int(tile.x), int(tile.y), depth, data);
}

void PutTreeDataScreen(uvec2 tile, int depth, int data)
{
    PutTreeDataScreen(int(tile.x), int(tile.y), depth, data);
}