#include "slider.h"

#include <algorithm>

#include "colors.h"
#include "outlineBackground.h"

#include "../input.h"
#include "../spriteRenderer.h"

Slider::Slider()
	: mouseDown(false)
	, numberOfValues(0)
	, currentValue(0)
	, snapPosition(false)
	, snapValue(0.0f)
	, minValue(0.0f)
	, maxValue(0.0f)
	, startValue(0.0f)
	, grabOffset(0)
{ }

void Slider::Init(Rect area
				  , const std::shared_ptr<SliderStyle>& style
				  , const std::shared_ptr<GUIBackgroundStyle>& backgroundStyle
				  , float minValue
				  , float maxValue
				  , float startValue
				  , float snapValue
				  , bool snapPosition
				  , std::function<void(float)> valueChangedCallback /*= nullptr */
				  , GUI_ANCHOR_POINT anchorPoint /*= GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT*/)
{
	Init(area.GetMinPosition()
		 , area.GetSize()
		 , style
		 , backgroundStyle
		 , minValue
		 , maxValue
		 , startValue
		 , snapValue
		 , snapPosition
		 , valueChangedCallback
		 , anchorPoint);
}

void Slider::Init(glm::vec2 position
				  , glm::vec2 size
				  , const std::shared_ptr<SliderStyle>& style
				  , const std::shared_ptr<GUIBackgroundStyle>& backgroundStyle
				  , float minValue
				  , float maxValue
				  , float startValue
				  , float snapValue
				  , bool snapPosition
				  , std::function<void(float)> valueChangedCallback /* = nullptr*/
				  , GUI_ANCHOR_POINT anchorPoint /*= GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT*/)
{
		this->style = std::static_pointer_cast<SliderStyle>(style);
	this->valueChangedCallback = valueChangedCallback;

	this->minValue = minValue;
	this->maxValue = maxValue;
	this->startValue = startValue;
	this->snapValue = snapValue;
	this->snapPosition = snapPosition;

	GUIContainerStyled::Init(position, size, style, backgroundStyle, anchorPoint);

	numberOfValues = static_cast<int>((maxValue - minValue) / snapValue);
	currentValue = static_cast<int>((startValue - minValue) / (maxValue - minValue) * numberOfValues);

	UpdateThumbSize();
	UpdateThumbPosition();
	UpdateCurrentValue();
}

void Slider::Update(std::chrono::nanoseconds delta)
{
	if(mouseDown && Input::MouseMoved())
	{
		if(style->direction == SliderStyle::DIRECTIONS::VERTICAL)
			thumb.SetPos(thumb.GetMinPosition().x, Input::GetMousePosition().y - grabOffset);
		else
			thumb.SetPos(Input::GetMousePosition().x - grabOffset, thumb.GetMinPosition().y);

		ClampThumbPosition();

		if(style->direction == SliderStyle::DIRECTIONS::VERTICAL)
		{
			if(snapPosition)
			{
				float thumbYPosition = thumb.GetMidPosition().y - area.GetMinPosition().y;

				float snapTo = area.GetHeight() / (numberOfValues + 1);

				thumb.SetPos(area.GetMinPosition().x, area.GetMinPosition().y + thumbYPosition - std::fmod(thumbYPosition, snapTo));
			}
		}
		else
		{
			if(snapPosition)
			{
				float thumbXPosition = thumb.GetMidPosition().x - area.GetMinPosition().x;

				float snapTo = area.GetWidth() / (numberOfValues + 1);

				thumb.SetPos(area.GetMinPosition().x +  thumbXPosition - std::fmod(thumbXPosition, snapTo), area.GetMinPosition().y);
			}
		}

		UpdateCurrentValue();

		if(valueChangedCallback != nullptr)
			valueChangedCallback(GetValue());
	}
}

void Slider::Draw(SpriteRenderer* spriteRenderer)
{
	background->Draw(spriteRenderer);

	spriteRenderer->Draw(thumb, style->thumbColor);
}


bool Slider::OnMouseDown(const MouseButtonState& keyState, const glm::vec2& mousePosition)
{
	if(keyState.button == MOUSE_BUTTON::LEFT)
	{
		if(thumb.Contains(mousePosition))
		{
			SetUpdate(true);

			mouseDown = true;

			if(style->direction == SliderStyle::DIRECTIONS::VERTICAL)
				grabOffset = static_cast<int>(mousePosition.y - thumb.GetMinPosition().y);
			else
				grabOffset = static_cast<int>(mousePosition.x - thumb.GetMinPosition().x);

			return true;
		}
		else //TODO: Jump up/down?
			grabOffset = -1;
	}

	return false;
}

void Slider::OnMouseUp(const MouseButtonState& keyState, const glm::vec2& mousePosition)
{
	mouseDown = false;
	grabOffset = -1;

	if(!IsMouseInside())
	{
		SetUpdate(false);
		SetReceiveAllEvents(false);
	}
}

float Slider::GetValue()
{
	return (currentValue * snapValue) + minValue;
}

void Slider::UpdateThumbSize()
{
	if(style->direction == SliderStyle::DIRECTIONS::VERTICAL)
		thumb.SetSize(area.GetWidth(), area.GetHeight() / (numberOfValues + 1));
	else
		thumb.SetSize(area.GetWidth() / (numberOfValues + 1), area.GetHeight());
}

void Slider::UpdateCurrentValue()
{
	if(style->direction == SliderStyle::DIRECTIONS::VERTICAL)
		currentValue = static_cast<int>(std::roundf((thumb.GetMinPosition().y - area.GetMinPosition().y) / (area.GetHeight() - thumb.GetHeight()) * numberOfValues));
	else
		currentValue = static_cast<int>(std::roundf((thumb.GetMinPosition().x - area.GetMinPosition().x) / (area.GetWidth() - thumb.GetWidth()) * numberOfValues));
}

bool Slider::OnMouseEnter()
{
	SetReceiveAllEvents(true);

	return true;
}

void Slider::OnMouseExit()
{
	if(!mouseDown)
		SetReceiveAllEvents(false);
}

void Slider::UpdateThumbPosition()
{
	if(style->direction == SliderStyle::DIRECTIONS::VERTICAL)
		thumb.SetPos(area.GetMinPosition().x, area.GetMinPosition().y + (currentValue / static_cast<float>(numberOfValues + 1)) * area.GetHeight());
	else
		thumb.SetPos(area.GetMinPosition().x + (currentValue / static_cast<float>(numberOfValues + 1)) * area.GetWidth(), area.GetMinPosition().y);
}

void Slider::ClampThumbPosition()
{
	if(style->direction == SliderStyle::DIRECTIONS::VERTICAL)
	{
		if(thumb.GetMinPosition().y < area.GetMinPosition().y)
			thumb.SetPos(area.GetMinPosition().x, area.GetMinPosition().y);
		else if(thumb.GetMaxPosition().y > area.GetMaxPosition().y)
			thumb.SetPos(area.GetMinPosition().x, area.GetMaxPosition().y - thumb.GetHeight());
	}
	else
	{
		if(thumb.GetMinPosition().x < area.GetMinPosition().x)
			thumb.SetPos(area.GetMinPosition().x, area.GetMinPosition().y);
		else if(thumb.GetMaxPosition().x > area.GetMaxPosition().x)
			thumb.SetPos(area.GetMaxPosition().x - thumb.GetWidth(), area.GetMinPosition().y);
	}
}

void Slider::SetPosition(float x, float y, GUI_ANCHOR_POINT anchorPoint)
{
	GUIContainerStyled::SetPosition(x, y, anchorPoint);

	UpdateThumbPosition();
}

void Slider::SetSize(float x, float y, GUI_ANCHOR_POINT anchorPoint)
{
	GUIContainerStyled::SetSize(x, y, anchorPoint);

	UpdateThumbSize();
	UpdateThumbPosition();
}