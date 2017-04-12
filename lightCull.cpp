#include "lightCull.h"

LightCull::LightCull()
{}

LightCull::~LightCull()
{}

void LightCull::InitShaderConstants(int screenWidth, int screenHeight)
{
    this->screenWidth = screenWidth;
    this->screenHeight = screenHeight;
}

int LightCull::GetTileCountX() const
{
    return -1;
}
