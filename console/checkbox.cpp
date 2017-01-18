#include "checkbox.h"

#include "../spriteRenderer.h"

Checkbox::Checkbox()
	: GUIContainerStyled()
	, toggled(false)
	, state(STATES::UNTOGGLED)
{

}

bool Checkbox::Init(Rect area
					, const std::shared_ptr<GUIBackgroundStyle>& backgroundStyle
					, GUI_ANCHOR_POINT anchorPoint /*= GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT*/)
{
	return Init(area.GetMinPosition(), area.GetSize(), backgroundStyle, anchorPoint);
}

bool Checkbox::Init(glm::vec2 position
					, glm::vec2 size
					, const std::shared_ptr<GUIBackgroundStyle>& backgroundStyle
					, GUI_ANCHOR_POINT anchorPoint /*= GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINTLEFT*/)
{

	GUIContainerStyled::Init(position
							 , size
							 , nullptr
							 , backgroundStyle
							 , anchorPoint);

	this->callbackFunction = callbackFunction;

	return true;
}

void Checkbox::Draw(SpriteRenderer* spriteRenderer)
{
	background->Draw(spriteRenderer);
}

bool Checkbox::OnMouseEnter()
{
	Highlight();
	SetReceiveAllEvents(true);

	return true;
}

void Checkbox::OnMouseExit()
{
	if(state != STATES::HELD)
		SetReceiveAllEvents(false);
}

bool Checkbox::OnMouseDown(const MouseButtonState& keyState, const glm::vec2& mousePosition)
{
	if(keyState.button == MOUSE_BUTTON::LEFT)
	{
		state = STATES::HELD;
		background->ChangePreset(static_cast<int>(STATES::HELD));
	}

	return true;
}

void Checkbox::OnMouseUp(const MouseButtonState& keyState, const glm::vec2& mousePosition)
{
	if(keyState.button == MOUSE_BUTTON::LEFT)
	{
		if(area.Contains(mousePosition))
		{
			if(state == STATES::HELD)
			{
				Toggle();

				if(callbackFunction != nullptr)
					callbackFunction("asdf");
			}
		}
		else
		{
			SetToggled(toggled);

			SetReceiveAllEvents(false);
		}
	}
}

void Checkbox::SetCallbackFunction(std::function<void(std::string)> callbackFunction)
{
	this->callbackFunction = callbackFunction;
}

void Checkbox::SetToggled(bool toggled)
{
	this->toggled = toggled;
	state = toggled ? STATES::TOGGLED : STATES::UNTOGGLED;

	background->ChangePreset(static_cast<int>(state));
}

bool Checkbox::IsToggled()
{
	return toggled;
}

void Checkbox::Toggle()
{
	SetToggled(!toggled);
}