#include "dropdownMenu.h"

#include "emptyBackground.h"

#include <algorithm>
#include "colorBackgroundStyle.h" //TODO: Remove

DropdownMenu::DropdownMenu()
	: Button()
{
	list.SetDraw(false);
	list.SetUpdate(false);
	list.SetReceiveAllEvents(false);

	this->clip = false;
}

bool DropdownMenu::Init(Rect area
						, std::shared_ptr<ButtonStyle>& buttonStyle
						, std::shared_ptr<GUIBackgroundStyle>& buttonBackgroundStyle
						, std::vector<std::string> alternatives
						, ContentManager* contentManager
						, std::function<void(std::string)> callbackFunction
						, const std::string& initialText /*= ""*/
						, GUI_ANCHOR_POINT anchorPoint /*= GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT*/)
{
	return Init(area.GetMinPosition(), area.GetSize(), buttonStyle, buttonBackgroundStyle, alternatives, contentManager, callbackFunction, initialText, anchorPoint);
}


bool DropdownMenu::Init(glm::vec2 position
						, glm::vec2 size
						, std::shared_ptr<ButtonStyle>& buttonStyle
						, std::shared_ptr<GUIBackgroundStyle>& buttonBackgroundStyle
						, std::vector<std::string> alternatives
						, ContentManager* contentManager
						, std::function<void(std::string)> callbackFunction
						, const std::string& initialText /*= ""*/
						, GUI_ANCHOR_POINT anchorPoint /*= GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT*/)
{
	if(!Button::Init(position
					 , size
					 , buttonStyle
					 , buttonBackgroundStyle
					 , [&](const std::string&)->void
	{
		Activate();
	}
					 , initialText.empty() ? alternatives[0] : initialText
		, anchorPoint))
		return false;

	this->callbackFunction = callbackFunction;

	//Create button list
	auto buttonListRect = Rect(position.x, position.y + size.y, size.x, size.y * alternatives.size());

	auto style = std::shared_ptr<ColorBackgroundStyle>(new ColorBackgroundStyle());
	style->colors.push_back(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
	style->colors.push_back(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
	style->colors.push_back(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));

	list.Init(buttonListRect, list.GenerateDefaultStyle(contentManager), style);

	buttonStyle->toggle = true;

	//Create buttons
	for(const auto& text : alternatives)
	{
		Button* newButton = new Button();

		Rect newButtonRect = Rect(0.0f, 0.0f, size.x, size.y);
		newButton->Init(newButtonRect, buttonStyle, nullptr, std::bind(&DropdownMenu::ListButtonPressed, this, std::placeholders::_1), text);

		buttons.emplace_back(newButton);
	}

	//Set elements
	std::vector<GUIContainer*> tempButtons;
	for(const auto& uniquePtr : buttons)
		tempButtons.push_back(uniquePtr.get());
	list.SetElements(tempButtons);

	originalArea = area;

	return true;
}

void DropdownMenu::Update(std::chrono::nanoseconds delta)
{
	if(list.GetUpdate())
		list.Update(delta);
}

void DropdownMenu::Draw(SpriteRenderer* spriteRenderer)
{
	Button::Draw(spriteRenderer);

	if(list.GetDraw())
		list.Draw(spriteRenderer);
}

bool DropdownMenu::OnMouseDown(const MouseButtonState& keyState, const glm::vec2& mousePosition)
{
	SetUpdate(true);

	if(!originalArea.Contains(mousePosition))
	{
		if(!list.GetArea().Contains(mousePosition))
			Button::Deactivate();
		else
			list.OnMouseDown(keyState, mousePosition);
	}
	else
	{
		return Button::OnMouseDown(keyState, mousePosition);
	}

	return true;
}

void DropdownMenu::OnMouseUp(const MouseButtonState& keyState, const glm::vec2& mousePosition)
{
	Button::OnMouseUp(keyState, mousePosition);
	list.OnMouseUp(keyState, mousePosition);
}

void DropdownMenu::SetPosition(float x, float y, GUI_ANCHOR_POINT anchorPoint)
{
	Button::SetPosition(x, y, anchorPoint);

	list.SetPosition(GetPosition().x, GetPosition().y + GetSize().y);

	originalArea = area;
}

void DropdownMenu::SetSize(float x, float y, GUI_ANCHOR_POINT anchorPoint)
{
	Button::SetSize(x, y, anchorPoint);

	list.SetPosition(GetPosition().x, GetPosition().y + GetSize().y);

	originalArea = area;
}

void DropdownMenu::ListButtonPressed(const std::string& buttonText)
{
	Button::SetText(buttonText);

	HideList();

	if(callbackFunction != nullptr)
		callbackFunction(buttonText);
}

void DropdownMenu::ShowList()
{
	originalArea = area;

	list.SetDraw(true);
	list.SetUpdate(true);

	area.SetSize(std::max(area.GetWidth(), list.GetArea().GetWidth()), area.GetHeight() + list.GetArea().GetHeight());
}

void DropdownMenu::HideList()
{
	area = originalArea;

	list.SetDraw(false);
	list.SetUpdate(false);
}

void DropdownMenu::Highlight()
{
	originalArea = area;

	Button::Highlight();
}

void DropdownMenu::UnHighlight()
{
	HideList();

	Button::UnHighlight();

	SetUpdate(false);
}

void DropdownMenu::Activate()
{
	if(list.GetDraw())
		HideList();
	else
		ShowList();
}

void DropdownMenu::Deactivate()
{
	HideList();

	Button::Deactivate();
}

void DropdownMenu::SetCallbackFunction(std::function<void(std::string)> callbackFunction)
{
	this->callbackFunction = callbackFunction;
}

bool DropdownMenu::OnMouseEnter()
{
	Button::OnMouseEnter();

	SetUpdate(true);

	return true;
}

void DropdownMenu::OnMouseExit()
{
	if(!list.GetDraw()
	   && buttonState != ButtonStyle::BUTTON_STATES::CLICK)
	{
		UnHighlight();
		SetReceiveAllEvents(false);
		SetUpdate(false);
	}
	else
		list.SetUpdate(true);
}