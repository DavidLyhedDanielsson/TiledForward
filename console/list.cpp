#include "list.h"
#include "../os/input.h"

#include "colors.h"
#include "outlineBackground.h"

#include "../spriteRenderer.h"
#include <algorithm>
#include "outlineBackgroundStyle.h"

List::List()
	: focusOn(nullptr)
	, drawBegin(0)
	, drawEnd(0)
	//, elementHeight(0)
	, highlitElement(-1)
	, ignoreMouse(false)
	, scrolling(false)
{ }

void List::Init(Rect area
				, const std::shared_ptr<ListStyle>& style
				, const std::shared_ptr<GUIBackgroundStyle>& backgroundStyle
				, GUI_ANCHOR_POINT anchorPoint /*= GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT*/)
{
	Init(area.GetMinPosition(), area.GetSize(), style, backgroundStyle, anchorPoint);
}

void List::Init(glm::vec2 position
				, glm::vec2 size
				, const std::shared_ptr<ListStyle>& style
				, const std::shared_ptr<GUIBackgroundStyle>& backgroundStyle
				, GUI_ANCHOR_POINT anchorPoint /*= GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT*/)
{
		this->style = style;

	GUIContainerStyled::Init(position
							 , size
							 , style
							 , backgroundStyle
							 , anchorPoint);

	scrollbar.Init(this->background->GetWorkArea()
				   , this->style->scrollbarStyle
				   , this->style->scrollbarBackgroundStyle
				   , std::bind(&List::ScrollFunction, this));
}

void List::Update(std::chrono::nanoseconds delta)
{
	glm::vec2 mousePosition = Input::GetMousePosition();

	if(!scrollbar.Contains(mousePosition))
	{
		if(!ignoreMouse
		   && !scrolling)
		{
			GUIContainer* currentFocus = focusOn;

			if(focusOn != nullptr)
			{
				if(!focusOn->GetArea().Contains(mousePosition))
				{
					UnHighlightElement();
					focusOn = nullptr;
				}
			}
			
			if(focusOn == nullptr)
			{
				for(int i = drawBegin; i < drawEnd; i++)
				{
					if(elements[i]->GetArea().Contains(mousePosition))
					{
						if(focusOn != elements[i])
						{
							HighlightElement(i);
							focusOn = elements[i];
						}

						break;
					}
				}
			}

			if(focusOn == nullptr)
				focusOn = currentFocus;

			for(int i = drawBegin; i < drawEnd; i++)
			{
				if(elements[i]->GetUpdate())
					elements[i]->Update(delta);
			}
		}
	}

	scrollbar.Update(delta);
}

void List::Draw(SpriteRenderer* spriteRenderer)
{
	background->Draw(spriteRenderer);

	for(int i = drawEnd - 1; i >= drawBegin; --i)
	{
		GUIContainer* container = elements[i];

		if(container->GetDraw())
			container->Draw(spriteRenderer);
	}

	scrollbar.Draw(spriteRenderer);
}

void List::SetPosition(float x, float y, GUI_ANCHOR_POINT anchorPoint)
{
	GUIContainerStyled::SetPosition(x, y, anchorPoint);

	UpdatePositions();

	scrollbar.SetPosition(background->GetWorkArea().GetMaxPosition().x, background->GetWorkArea().GetMinPosition().y);
}

void List::SetSize(float x, float y, GUI_ANCHOR_POINT anchorPoint)
{
	GUIContainerStyled::SetSize(x, y, anchorPoint);

	UpdatePositions();

	scrollbar.SetSize(static_cast<float>(style->scrollbarStyle->thumbWidth), background->GetWorkArea().GetHeight());
}

void List::SetElements(const std::vector<GUIContainer*> elements)
{
	UnHighlightElement();

	this->elements = elements;

	float totalHeight = 0.0f;
	for(GUIContainer* container : elements)
		totalHeight += container->GetSize().y;

	if(this->elements.size() > 0)
	{
		//elementHeight = static_cast<int>(this->elements.back()->GetSize().y);

		scrollbar.SetVisibleItems(static_cast<int>(background->GetWorkArea().GetHeight()));
		scrollbar.SetMaxItems(static_cast<int>(totalHeight));
	}
	else
	{
		//elementHeight = 1;
		scrollbar.SetVisibleItems(0);
		scrollbar.SetMaxItems(0);
	}

	UpdatePositions();
}

std::vector<GUIContainer*> List::GetElements()
{
	return elements;
}

void List::ClearElements()
{
	focusOn = nullptr;

	UnHighlightElement();
	elements.clear();

	scrollbar.SetMaxItems(0);
	drawBegin = 0;
	drawEnd = 0;
}

void List::HighlightElement(int element)
{
	if(highlitElement != -1)
		elements[highlitElement]->UnHighlight();

	highlitElement = element;
	elements[element]->Highlight();

	int scrollTo = 0;
	int downOffset = 0;

	for(int i = 0, end = std::min(static_cast<int>(elements.size()), element); i < end; ++i)
	{
		downOffset = static_cast<int>(elements[i]->GetSize().y);
		scrollTo += static_cast<int>(elements[i]->GetSize().y);
	}

	scrollbar.ScrollTo(scrollTo, downOffset);
	UpdatePositions();
}

void List::UnHighlightElement()
{
	if(highlitElement != -1)
		elements[highlitElement]->UnHighlight();

	highlitElement = -1;
}

void List::ClearHighlights()
{
	for(auto element : elements)
		element->UnHighlight();
}

int List::GetHighlitElementIndex()
{
	return highlitElement;
}

GUIContainer* List::GetHighlitElement()
{
	return elements[highlitElement];
}

void List::SetIgnoreMouse(bool ignore)
{
	ignoreMouse = ignore;
}

bool List::OnMouseEnter()
{
	update = true;
	receiveAllEvents = true;

	UnHighlightElement();

	return true;
}

void List::OnMouseExit()
{
	UnHighlightElement();
}

bool List::OnMouseDown(const MouseButtonState& keyState, const glm::vec2& mousePosition)
//TODO: Update highlit element?
{
	if(area.Contains(mousePosition))
	{
		if(!scrollbar.Contains(mousePosition))
		{
			focusOn = nullptr;

			for(GUIContainer* container : elements)
			{
				if(container->GetDraw()
				   && container->GetArea().Contains(mousePosition))
				{
					focusOn = container;
					container->OnMouseDown(keyState, mousePosition);
					break;
				}
			}
		}
		else
		{
			scrollbar.OnMouseDown(keyState, mousePosition);
			scrolling = true;
		}

		return true;
	}
	else
	{
		update = false;
		receiveAllEvents = false;

		return false;
	}
}

void List::OnMouseUp(const MouseButtonState& keyState, const glm::vec2& mousePosition)
{
	scrolling = false;

	if(focusOn != nullptr)
		focusOn->OnMouseUp(keyState, mousePosition);

	scrollbar.OnMouseUp(keyState, mousePosition);
}

bool List::OnKeyDown(const KeyState& keyState)
{
	if(focusOn != nullptr)
	{
		focusOn->OnKeyDown(keyState);

		return true;
	}

	return false;
}

void List::OnKeyUp(const KeyState& keyState)
{
	if(focusOn != nullptr)
		focusOn->OnKeyUp(keyState);
}

bool List::OnChar(unsigned int keyCode)
{
	if(focusOn != nullptr)
	{
		focusOn->OnChar(keyCode);

		return true;
	}

	return false;
}

bool List::OnScroll(int distance)
{
	if(focusOn != nullptr)
		focusOn->OnScroll(-distance * style->scrollDistance);

	scrollbar.OnScroll(-distance * style->scrollDistance);
	UpdatePositions();

	return true;
}

void List::UpdatePositions()
{
	drawBegin = 0;
	drawEnd = 0;

	if(elements.size() > 0)
	{
		glm::vec2 newPosition = background->GetWorkArea().GetMinPosition();

		float height = 0.0f;
		int i = 0;
		for(int end = static_cast<int>(elements.size()); i < end; ++i)
		{
			if(height + elements[i]->GetSize().y > scrollbar.GetMinIndex())
			{
				drawBegin = i;
				drawEnd = i;
				break;
			}

			height += elements[i]->GetSize().y;
		}

		for(int end = static_cast<int>(elements.size()); i < end; ++i)
		{
			if(height >= scrollbar.GetMaxIndex())
				break;

			drawEnd = i + 1;
			height += elements[i]->GetSize().y;
		}

		i = 0;
		for(int end = drawBegin; i < end; i++)
		{
			elements[i]->SetReceiveAllEvents(false);
			elements[i]->SetDraw(false);
		}

		i = drawBegin;
		for(int end = drawEnd; i < end; i++)
		{
			elements[i]->SetPosition(newPosition);
			elements[i]->SetReceiveAllEvents(true);
			elements[i]->SetDraw(true);

			newPosition.y += elements[i]->GetSize().y;
		}

		i = drawEnd;
		for(int end = static_cast<int>(elements.size()); i < end; i++)
		{
			elements[i]->SetReceiveAllEvents(false);
			elements[i]->SetDraw(false);
		}
	}
}

void List::ScrollFunction()
{
	UpdatePositions();
}

std::shared_ptr<ListStyle> List::GenerateDefaultStyle(ContentManager* contentManager)
{
	auto style = std::make_shared<ListStyle>();

	style->scrollbarStyle = Scrollbar::GenerateDefaultStyle(contentManager);
	style->scrollbarBackgroundStyle = Scrollbar::GenerateDefaultBackgroundStyle(contentManager);

	return style;
}

std::unique_ptr<GUIBackground> List::GenerateDefaultBackground(ContentManager* contentManager)
{
	return std::make_unique<OutlineBackground>();
}

std::shared_ptr<GUIBackgroundStyle> List::GenerateDefaultBackgroundStyle(ContentManager* contentManager)
{
	auto style = std::make_shared<OutlineBackgroundStyle>();

	style->AddColor(COLORS::snow4, COLORS::snow3);
	style->outlineThickness = 1.0f;
	style->inclusiveBorder = false;

	return style;
}

int List::GetElementsSize() const
{
	return static_cast<int>(elements.size());
}

bool List::GetIsScrolling() const
{
	return scrolling;
}

int List::GetScrollbarWidth() const
{
	return style->scrollbarStyle->thumbWidth;
}
