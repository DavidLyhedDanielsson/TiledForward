#include "guiContainer.h"

GUIContainer::GUIContainer()
	: receiveAllEvents(false)
	, update(false)
	, draw(true)
	, clip(true)
	, mouseInside(false)
{
}

bool GUIContainer::Init(Rect area, GUI_ANCHOR_POINT anchorPoint /*= GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT*/)
{
	return Init(area.GetMinPosition(), area.GetSize(), anchorPoint);
}

bool GUIContainer::Init(glm::vec2 position, glm::vec2 size, GUI_ANCHOR_POINT anchorPoint /*= GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT*/)
{
	area.Set(CalculatePosition(position, size, anchorPoint), size);

	return true;
}

//////////////////////////////////////////////////////////////////////////
//SETTERS
//////////////////////////////////////////////////////////////////////////

void GUIContainer::SetPosition(const glm::vec2& newPosition)
{
	SetPosition(newPosition.x, newPosition.y, GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT);
}

void GUIContainer::SetPosition(const glm::vec2& newPosition, GUI_ANCHOR_POINT anchorPoint)
{
	SetPosition(newPosition.x, newPosition.y, anchorPoint);
}

void GUIContainer::SetPosition(float x, float y)
{
	SetPosition(x, y, GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT);
}

void GUIContainer::SetPosition(float x, float y, GUI_ANCHOR_POINT anchorPoint)
{
	area.SetPos(CalculatePosition(glm::vec2(x, y), anchorPoint));
}

void GUIContainer::SetSize(const glm::vec2& newSize)
{
	SetSize(newSize.x, newSize.y);
}

void GUIContainer::SetSize(const glm::vec2& newSize, GUI_ANCHOR_POINT anchorPoint)
{
	SetSize(newSize.x, newSize.y, anchorPoint);
}

void GUIContainer::SetSize(float x, float y)
{
	SetSize(x, y, GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT);
}

void GUIContainer::SetSize(float x, float y, GUI_ANCHOR_POINT anchorPoint)
{
	if(anchorPoint != (GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT))
	{
		//SetPosition needs to be called
		glm::vec2 newPosition = area.GetMinPosition();
		glm::vec2 sizeDelta = area.GetSize() - glm::vec2(x, y);

		if(anchorPoint & GUI_ANCHOR_POINT::LEFT)
			newPosition.x -= sizeDelta.x;
		if(anchorPoint & GUI_ANCHOR_POINT::TOP)
			newPosition.y -= sizeDelta.y;
		
		SetPosition(newPosition + sizeDelta);
	}

	area.SetSize(x, y);
}

void GUIContainer::SetArea(const Rect& newArea)
{
	area = newArea;
}

void GUIContainer::SetDraw(bool draw)
{
	this->draw = draw;
}

void GUIContainer::SetUpdate(bool update)
{
	this->update = update;
}

void GUIContainer::SetReceiveAllEvents(bool active)
{
	this->receiveAllEvents = active;
}

Rect GUIContainer::GetArea() const
{
	return area;
}

glm::vec2 GUIContainer::GetPosition() const
{
	return area.GetMinPosition();
}

glm::vec2 GUIContainer::GetSize() const
{
	return area.GetSize();
}

bool GUIContainer::GetDraw() const
{
	return draw;
}

bool GUIContainer::GetUpdate() const
{
	return update;
}

bool GUIContainer::GetReceiveAllEvents() const
{
	return receiveAllEvents;
}

glm::vec2 GUIContainer::CalculatePosition(glm::vec2 position, GUI_ANCHOR_POINT anchorPoint) const
{
	return CalculatePosition(position, area.GetSize(), anchorPoint);
}

glm::vec2 GUIContainer::CalculatePosition(glm::vec2 position, glm::vec2 size, GUI_ANCHOR_POINT anchorPoint) const
{
	if(anchorPoint & (GUI_ANCHOR_POINT::LEFT | GUI_ANCHOR_POINT::RIGHT))
		position.x -= size.x * 0.5f;
	else if(anchorPoint & GUI_ANCHOR_POINT::RIGHT)
		position.x -= size.x;

	if(anchorPoint & (GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::BOTTOM))
		position.y -= size.y * 0.5f;
	else if(anchorPoint & GUI_ANCHOR_POINT::BOTTOM)
		position.y -= size.y;

	return position;
}

bool GUIContainer::IsMouseInside() const
{
	return mouseInside;
}