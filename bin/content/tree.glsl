layout(std430) buffer Tree
{
    int tree[];
};

int GetTreeIndex(int x, int y)
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

void PutTreeData(int screenX, int screenY, int depth, int data)
{
    int depthOffset = -1;
    for(int i = 0; i < depth; ++i)
        depthOffset += pow(2, i) * pow(2, i);

    vec2 range = vec2(float(screenWidth), float(screenHeight));
    range /= pow(2, depth);

    int x = int(screenX / range.x);
    int y = int(screenY / range.y);

    int index = GetTreeIndex(x, y);

    tree[depthOffset + index] = data;
}

int GetTreeData(int screenX, int screenY)
{
    int depthOffset = -1;

    vec2 range = vec2(float(screenWidth), float(screenHeight));

    for(int i = 0; i < 3; ++i)
    {
        //depthOffset += int(pow(2, i)) * int(pow(2, i));
        depthOffset += int(pow(4, i));

        range /= 2.0f;

        int x = int(screenX / range.x);
        int y = int(screenY / range.y);

        int index = GetTreeIndex(x, y);

        int potentialIndex = tree[depthOffset + index];
        if(potentialIndex != -1)
            return potentialIndex;
    }

    return 0;
}