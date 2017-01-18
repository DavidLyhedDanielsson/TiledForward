#include "emptyBackground.h"

EmptyBackground::EmptyBackground()
{

}

EmptyBackground::~EmptyBackground()
{

}

void EmptyBackground::Init(const std::shared_ptr<GUIStyle>& style, const Rect* area)
{
	this->area = area;
}

void EmptyBackground::Draw(SpriteRenderer* spriteRenderer)
{

}

std::unique_ptr<GUIBackground> EmptyBackground::Clone()
{
	return std::unique_ptr<GUIBackground>(new EmptyBackground);
}
