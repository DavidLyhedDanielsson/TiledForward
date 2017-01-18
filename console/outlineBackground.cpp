#include "outlineBackground.h"
#include "outlineBackgroundStyle.h"

#include "common.h"

#include "../spriteRenderer.h"

OutlineBackground::OutlineBackground()
	: outlineRect(nullptr)
	, workAreaRect(nullptr)
{

}

void OutlineBackground::Init(const std::shared_ptr<GUIStyle>& style, const Rect* area)
{
	this->style = CastStyleTo<OutlineBackgroundStyle>(style);
	this->area = area;

	if(this->style->inclusiveBorder)
	{
		outlineRect = area;
		workAreaRect = &rect;
	}
	else
	{
		outlineRect = &rect;
		workAreaRect = area;
	}

	UpdateOutlineRect();
}

void OutlineBackground::Draw(SpriteRenderer* spriteRenderer)
{
	if(outlineRect != nullptr)
		spriteRenderer->Draw(*outlineRect, style->outlineColors[preset]);

	spriteRenderer->Draw(*workAreaRect, style->backgroundColors[preset]);
}

void OutlineBackground::AreaChanged()
{
	GUIBackground::AreaChanged();
	UpdateOutlineRect();
}

void OutlineBackground::UpdateOutlineRect()
{
	if(style->outlineThickness == 0.0f)
	{
		outlineRect = nullptr;
		workAreaRect = this->area;
		return;
	}

	if(style->inclusiveBorder)
	{
		outlineRect = area;
		workAreaRect = &rect;

		rect = *area;

		glm::vec2 positionModifier(0.0f, 0.0f);
		glm::vec2 sizeModifier(0.0f, 0.0f);

		//Update positionModifier and sizeModifier depending on style exclusions
		if((style->outlineSides & DIRECTIONS::Y_BITS & DIRECTIONS::TOP) == DIRECTIONS::TOP)
		{
			positionModifier.y += style->outlineThickness;
			sizeModifier.y -= style->outlineThickness;
		}
		if((style->outlineSides & DIRECTIONS::Y_BITS & DIRECTIONS::BOTTOM) == DIRECTIONS::BOTTOM)
		{
			sizeModifier.y -= style->outlineThickness;
		}
		if((style->outlineSides & DIRECTIONS::X_BITS & DIRECTIONS::LEFT) == DIRECTIONS::LEFT)
		{
			positionModifier.x += style->outlineThickness;
			sizeModifier.x -= style->outlineThickness;
		}
		if((style->outlineSides & DIRECTIONS::X_BITS & DIRECTIONS::RIGHT) == DIRECTIONS::RIGHT)
		{
			sizeModifier.x -= style->outlineThickness;
		}

		rect += positionModifier;
		
		glm::vec2 newSize = rect.GetSize();

		newSize.x += sizeModifier.x;
		newSize.y += sizeModifier.y;

		rect.SetSize(newSize);
	}
	else
	{
		outlineRect = &rect;
		workAreaRect = area;

		rect = *area;

		glm::vec2 positionModifier(0.0f, 0.0f);
		glm::vec2 sizeModifier(0.0f, 0.0f);

		//Update positionModifier and sizeModifier depending on style exclusions
		if((style->outlineSides & DIRECTIONS::Y_BITS & DIRECTIONS::TOP) == DIRECTIONS::TOP)
		{
			positionModifier.y -= style->outlineThickness;
			sizeModifier.y += style->outlineThickness;
		}
		if((style->outlineSides & DIRECTIONS::Y_BITS & DIRECTIONS::BOTTOM) == DIRECTIONS::BOTTOM)
		{
			sizeModifier.y += style->outlineThickness;
		}
		if((style->outlineSides & DIRECTIONS::X_BITS & DIRECTIONS::LEFT) == DIRECTIONS::LEFT)
		{
			positionModifier.x -= style->outlineThickness;
			sizeModifier.x += style->outlineThickness;
		}
		if((style->outlineSides & DIRECTIONS::X_BITS & DIRECTIONS::RIGHT) == DIRECTIONS::RIGHT)
		{
			sizeModifier.x += style->outlineThickness;
		}

		rect += positionModifier;

		glm::vec2 newSize = rect.GetSize();

		newSize.x += sizeModifier.x;
		newSize.y += sizeModifier.y;

		rect.SetSize(newSize);
	}
}

std::unique_ptr<GUIBackground> OutlineBackground::Clone()
{
	return std::unique_ptr<GUIBackground>(new OutlineBackground(*this));
}

Rect OutlineBackground::GetWorkArea() const
{
	return *workAreaRect;
}

Rect OutlineBackground::GetFullArea() const
{
	return *outlineRect;
}
