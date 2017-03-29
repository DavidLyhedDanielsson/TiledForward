struct TileLightData
{
    int start;
    int numberOfLights;
    ivec2 padding;
};

layout(std430) buffer LightIndices
{
    int lightIndices[];
};

layout(std430) buffer TileLights
{
    TileLightData tileLightData[];
};

layout(std140) uniform ReadWriteOffsets
{
    int lightIndicesDataReadOffset;
    int lightIndicesDataWriteOffset;
    int tileLightDataReadOffset;
    int tileLightDataWriteOffset;
};

int GetLightIndex(int index)
{
    return lightIndices[lightIndicesDataReadOffset + index];
}

void SetLightIndex(int index, int data)
{
    lightIndices[lightIndicesDataWriteOffset + index] = data;
}

TileLightData GetTileLightData(int index)
{
    return tileLightData[tileLightDataReadOffset + index];
}

void SetTileLightData(int index, TileLightData data)
{
    tileLightData[tileLightDataWriteOffset + index] = data;
}