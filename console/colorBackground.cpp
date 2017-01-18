#include "colorBackground.h"
#include "colorBackgroundStyle.h"

#include "../spriteRenderer.h"

ColorBackground::ColorBackground()
{

}

ColorBackground::~ColorBackground()
{

}

void ColorBackground::Init(const std::shared_ptr<GUIStyle>& style, const Rect* area)
{
	this->style = CastStyleTo<ColorBackgroundStyle>(style);
	this->area = area;
}


void ColorBackground::Draw(SpriteRenderer* spriteRenderer)
{
	spriteRenderer->Draw(*area, style->colors[preset]);
}

std::unique_ptr<GUIBackground> ColorBackground::Clone()
{
	return std::unique_ptr<GUIBackground>(new ColorBackground(*this));
}