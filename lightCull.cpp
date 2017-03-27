#include "lightCull.h"

LightCull::LightCull()
    : threadsPerGroup(2, 2)
{}

LightCull::~LightCull()
{}

void LightCull::InitShaderConstants(int screenWidth, int screenHeight)
{
    this->screenWidth = screenWidth;
    this->screenHeight = screenHeight;
}