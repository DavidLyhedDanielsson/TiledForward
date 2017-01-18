#include "button.h"

#include "../spriteRenderer.h"

Button::Button()
	: textXOffset(0)
	, textYOffset(0)
	, buttonState(ButtonStyle::BUTTON_STATES::NORMAL)
{
	this->update = false;
}

bool Button::Init(const Rect& area
				  , const std::shared_ptr<ButtonStyle>& style
				  , const std::shared_ptr<GUIBackgroundStyle>& backgroundStyle
				  , std::function<void(std::string)> callbackFunction /*= nullptr */
				  , const std::string& text /*= ""*/
				  , GUI_ANCHOR_POINT anchorPoint /*= GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT*/
				  , GUI_ANCHOR_POINT textAnchorPoint /*= GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT*/)
{
	return Init(area.GetMinPosition(), area.GetSize(), style, backgroundStyle, callbackFunction, text, anchorPoint, textAnchorPoint);
}

bool Button::Init(glm::vec2 position
				  , glm::vec2 size
				  , const std::shared_ptr<ButtonStyle>& style
				  , const std::shared_ptr<GUIBackgroundStyle>& backgroundStyle
				  , std::function<void(std::string)> callbackFunction /*= nullptr */
				  , const std::string& text /*= "" */
				  , GUI_ANCHOR_POINT anchorPoint /*= GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT*/
				  , GUI_ANCHOR_POINT textAnchorPoint /*= GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT*/)
{
	this->style = style;
	this->textAnchorPoint = textAnchorPoint;

	if(this->style == nullptr)
		return false;
	else if(this->style->characterSet == nullptr)
	{
		Logger::LogLine(LOG_TYPE::FATAL, "ButtonStyle->characterSet is nullptr!");
		return false;
	}

	GUIContainerStyled::Init(position
							 , size
							 , style
							 , backgroundStyle
							 , anchorPoint);

	this->callbackFunction = callbackFunction;
	SetText(text);
	SetTextOffset();

	return true;
}

void Button::Draw(SpriteRenderer* spriteRenderer)
{
	background->Draw(spriteRenderer);

	glm::vec2 drawPosition(area.GetMinPosition());

	drawPosition.x += style->textOffset.x + textXOffset;
	drawPosition.y += style->textOffset.y + textYOffset;

	spriteRenderer->DrawString(style->characterSet, text, drawPosition, static_cast<int>(area.GetWidth()), style->textColors[static_cast<int>(buttonState)]);
}

bool Button::OnMouseEnter()
{
	Highlight();
	SetReceiveAllEvents(true);

	return true;
}

void Button::OnMouseExit()
{
	if(buttonState != ButtonStyle::BUTTON_STATES::CLICK)
		Deactivate();
}

bool Button::OnMouseDown(const MouseButtonState& keyState, const glm::vec2& mousePosition)
{
	if(keyState.button == MOUSE_BUTTON::LEFT)
		Activate();

	return buttonState != ButtonStyle::BUTTON_STATES::NORMAL;
}

void Button::OnMouseUp(const MouseButtonState& keyState, const glm::vec2& mousePosition)
{
	if(keyState.button == MOUSE_BUTTON::LEFT)
	{
		if(area.Contains(mousePosition))
		{
			if(!style->toggle)
			{
				if(callbackFunction != nullptr)
					callbackFunction(text);

				SetState(ButtonStyle::BUTTON_STATES::HOVER);
			}
		}
		else
			Deactivate();
	}
}

void Button::Highlight()
{
	SetState(ButtonStyle::BUTTON_STATES::HOVER);
}

void Button::UnHighlight()
{
	SetState(ButtonStyle::BUTTON_STATES::NORMAL);
}

void Button::Activate()
{
	if(buttonState == ButtonStyle::BUTTON_STATES::HOVER)
	{
		SetState(ButtonStyle::BUTTON_STATES::CLICK);

		if(style->toggle)
		{
			if(callbackFunction != nullptr)
				callbackFunction(text);
		}
	}
	else if(buttonState == ButtonStyle::BUTTON_STATES::CLICK)
	{
		if(style->toggle)
			SetState(ButtonStyle::BUTTON_STATES::HOVER);
		else
		{
			if(callbackFunction != nullptr)
				callbackFunction(text);
		}
	}
}

void Button::Deactivate()
{
	UnHighlight();
	SetState(ButtonStyle::BUTTON_STATES::NORMAL);

	SetReceiveAllEvents(false);
}

void Button::SetCallbackFunction(std::function<void(std::string)> callbackFunction)
{
	this->callbackFunction = callbackFunction;
}

void Button::SetText(const std::string& text)
{
	this->text = text;
}

std::string Button::GetText() const
{
	return text;
}

void Button::SetSize(float x, float y, GUI_ANCHOR_POINT anchorPoint)
{
	GUIContainerStyled::SetSize(x, y, anchorPoint);

	SetTextOffset();
}

bool Button::IsDown() const
{
	return buttonState == ButtonStyle::BUTTON_STATES::CLICK;
}

void Button::SetTextOffset()
{
	int halfWidth = static_cast<int>(this->style->characterSet->GetWidthAtIndex(text.c_str(), -1) * 0.5f);

	if(textAnchorPoint & GUI_ANCHOR_POINT::LEFT && textAnchorPoint & GUI_ANCHOR_POINT::RIGHT)
		textXOffset = static_cast<int>(this->area.GetWidth() * 0.5f) - halfWidth;
	else if(textAnchorPoint & GUI_ANCHOR_POINT::RIGHT)
		textXOffset = static_cast<int>(this->area.GetWidth()) - halfWidth * 2;

	if(textAnchorPoint & GUI_ANCHOR_POINT::TOP && textAnchorPoint & GUI_ANCHOR_POINT::BOTTOM)
		textYOffset = static_cast<int>(this->area.GetHeight() * 0.5f - this->style->characterSet->GetLineHeight() * 0.5f);
	else if(textAnchorPoint & GUI_ANCHOR_POINT::BOTTOM)
		textYOffset = static_cast<int>(this->area.GetHeight() - this->style->characterSet->GetLineHeight());
}

void Button::SetState(ButtonStyle::BUTTON_STATES state)
{
	buttonState = state;
	background->ChangePreset(static_cast<int>(state));
}
