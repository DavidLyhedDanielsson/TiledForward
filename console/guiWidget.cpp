#include "guiWidget.h"

#include "../input.h"

#include "../contentManager.h"

#include <limits>
#include <algorithm>
#include "../spriteRenderer.h"

using floatLimits = std::numeric_limits<float>;

GUIWidget::GUIWidget()
	: GUIContainer()
	, allowMove(false)
	, allowResize(false)
	, hasChanged(false)
	, moveResizeButton(MOUSE_BUTTON::NONE)
	, moveResizeModifierKeys(KEY_MODIFIERS::NONE)
	, moveResizeModifierKeysHeld(false)
	//Moving
	, isMoving(false)
	, updateWhenMoving(false)
	, minPosition(floatLimits::min(), floatLimits::min())
	, maxPosition(floatLimits::max(), floatLimits::max())
	, relativeGrabPosition(0.0f, 0.0f)
	//Resizing
	, isResizing(false)
	, updateWhenResizing(false)
	, minSize(floatLimits::min(), floatLimits::min())
	, maxSize(floatLimits::max(), floatLimits::max())
	, grabPosition(0.0f, 0.0f)
	, resizeAnchorPoint(GUI_ANCHOR_POINT::TOP)
{ }

GUIWidget::~GUIWidget()
{ }

bool GUIWidget::Init(ContentManager* contentManager
					, Rect area
					, bool allowMove
					, bool updateWhenMoving
					, bool allowResize
					, bool updateWhenResizing
					, MOUSE_BUTTON moveResizeButton /*= MOUSE_BUTTON::LEFT */
					, KEY_MODIFIERS moveResizeModifierKeys /*= KEY_MODIFIERS::CONTROL */
					, GUI_ANCHOR_POINT anchorPoint /*= GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT*/
					, glm::vec2 minPosition /*= glm::vec2(std::numeric_limits<float>::min(), std::numeric_limits<float>::min()) */
					, glm::vec2 maxPosition /*= glm::vec2(std::numeric_limits<float>::max(), std::numeric_limits<float>::max()) */
					, glm::vec2 minSize /*= glm::vec2(std::numeric_limits<float>::min(), std::numeric_limits<float>::min()) */
					, glm::vec2 maxSize /*= glm::vec2(std::numeric_limits<float>::max(), std::numeric_limits<float>::max()) */)
{
	return Init(contentManager
				, area.GetMinPosition()
				, area.GetSize()
				, allowMove
				, updateWhenMoving
				, allowResize
				, updateWhenResizing
				, moveResizeButton
				, moveResizeModifierKeys
				, anchorPoint
				, minPosition
				, maxPosition
				, minSize
				, maxSize
	);
}

bool GUIWidget::Init(ContentManager* contentManager
					, glm::vec2 position
					, glm::vec2 size
					, bool allowMove
					, bool updateWhenMoving
					, bool allowResize
					, bool updateWhenResizing
					, MOUSE_BUTTON moveResizeButton /*= MOUSE_BUTTON::LEFT */
					, KEY_MODIFIERS moveResizeModifierKeys /*= KEY_MODIFIERS::CONTROL */
					, GUI_ANCHOR_POINT anchorPoint /*= GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT*/
					, glm::vec2 minPosition /*= glm::vec2(std::numeric_limits<float>::min(), std::numeric_limits<float>::min()) */
					, glm::vec2 maxPosition /*= glm::vec2(std::numeric_limits<float>::max(), std::numeric_limits<float>::max()) */
					, glm::vec2 minSize /*= glm::vec2(std::numeric_limits<float>::min(), std::numeric_limits<float>::min()) */
					, glm::vec2 maxSize /*= glm::vec2(std::numeric_limits<float>::max(), std::numeric_limits<float>::max()) */)
{
	if(size.x < minSize.x)
		size.x = minSize.x;
	else if(size.x > maxSize.x)
		size.x = maxSize.x;

	if(size.y < minSize.y)
		size.y = minSize.y;
	else if(size.y > maxSize.y)
		size.y = maxSize.y;

	GUIContainer::Init(position, size, anchorPoint);

	this->allowMove = allowMove;
	this->updateWhenMoving = updateWhenMoving;
	this->allowResize = allowResize;
	this->updateWhenResizing = updateWhenResizing;
	this->moveResizeButton = moveResizeButton;
	this->moveResizeModifierKeys = moveResizeModifierKeys;
	this->minPosition = minPosition;
	this->maxPosition = maxPosition;
	this->minSize = minSize;
	this->maxSize = maxSize;

	return true;
}

void GUIWidget::Update(std::chrono::nanoseconds delta)
{
	if(isMoving)
	{
		glm::vec2 mousePosition = Input::GetMousePosition();

		glm::vec2 newPosition = mousePosition - relativeGrabPosition;

		if(newPosition.x < minPosition.x)
			newPosition.x = minPosition.x;
		else if(newPosition.x + area.GetWidth() > maxPosition.x)
			newPosition.x = maxPosition.x - area.GetWidth();

		if(newPosition.y < minPosition.y)
			newPosition.y = minPosition.y;
		else if(newPosition.y + area.GetHeight() > maxPosition.y)
			newPosition.y = maxPosition.y - area.GetHeight();

		SetPosition(newPosition);

		if(updateWhenMoving)
			SubUpdate(delta);
	}
	else if(isResizing)
	{
		glm::vec2 mousePosition = Input::GetMousePosition();

		glm::vec2 sizeDelta = mousePosition - grabPosition;

		GUI_ANCHOR_POINT anchor;

		if(resizeAnchorPoint & GUI_ANCHOR_POINT::LEFT)
		{
			anchor = GUI_ANCHOR_POINT::RIGHT;
			sizeDelta.x *= -1;
		}
		else if(resizeAnchorPoint & GUI_ANCHOR_POINT::RIGHT)
			anchor = GUI_ANCHOR_POINT::LEFT;
		else
		{
			anchor = GUI_ANCHOR_POINT::LEFT;
			sizeDelta.x *= 0.0f;
		}

		if(resizeAnchorPoint & GUI_ANCHOR_POINT::TOP)
		{
			anchor |= GUI_ANCHOR_POINT::BOTTOM;
			sizeDelta.y *= -1;
		}
		else if(resizeAnchorPoint & GUI_ANCHOR_POINT::BOTTOM)
			anchor |= GUI_ANCHOR_POINT::TOP;
		else
		{
			anchor |= GUI_ANCHOR_POINT::TOP;
			sizeDelta.y *= 0.0f;
		}

		glm::vec2 newSize = GetSize() + sizeDelta;

		if(newSize.x < minSize.x)
			newSize.x = minSize.x;
		else if(newSize.x > maxSize.x)
			newSize.x = maxSize.x;

		if(newSize.y < minSize.y)
			newSize.y = minSize.y;
		else if(newSize.y > maxSize.y)
			newSize.y = maxSize.y;

		glm::vec2 oldSize = area.GetSize();

		SetSize(newSize, anchor);

		sizeDelta = area.GetSize() - oldSize;
		if(resizeAnchorPoint & GUI_ANCHOR_POINT::LEFT)
			grabPosition.x -= sizeDelta.x;
		else if(resizeAnchorPoint & GUI_ANCHOR_POINT::RIGHT)
			grabPosition.x += sizeDelta.x;

		if(resizeAnchorPoint & GUI_ANCHOR_POINT::TOP)
			grabPosition.y -= sizeDelta.y;
		else if(resizeAnchorPoint & GUI_ANCHOR_POINT::BOTTOM)
			grabPosition.y += sizeDelta.y;

		if(updateWhenResizing)
			SubUpdate(delta);
	}
	else
		SubUpdate(delta);
}

void GUIWidget::Draw(SpriteRenderer* spriteRenderer)
{
	SubDraw(spriteRenderer);

	if(IsMouseInside()
	   && moveResizeModifierKeysHeld
	   && (allowMove || allowResize))
	{
		glm::vec2 mousePosition = Input::GetMousePosition();

		GUI_ANCHOR_POINT resizePoint = resizeAnchorPoint;
		
		if(resizeAnchorPoint == GUI_ANCHOR_POINT::NONE)
			resizePoint = GetResizeAnchorPoint(mousePosition);

		float subWidth = std::min(area.GetWidth() * 0.2f, 32.0f);
		float subHeight = std::min(area.GetHeight() * 0.2f, 32.0f);

		float minX = area.GetMinPosition().x;
		float maxX = area.GetMaxPosition().x;

		float minY = area.GetMinPosition().y;
		float maxY = area.GetMaxPosition().y;

		if(allowResize)
		{
			switch(static_cast<int>(resizePoint))
			{
				case 0b1: //TOP
					spriteRenderer->Draw(Rect(minX + subWidth, minY, area.GetWidth() - subWidth * 2.0f, subHeight), glm::vec4(1.0f, 1.0f, 1.0f, IsResizing() ? 0.9f : 0.65f));
					break;
				case 0b10: //BOTTOM
					spriteRenderer->Draw(Rect(minX + subWidth, maxY - subHeight, area.GetWidth() - subWidth * 2.0f, subHeight), glm::vec4(1.0f, 1.0f, 1.0f, IsResizing() ? 0.9f : 0.65f));
					break;
				case 0b100: //LEFT
					spriteRenderer->Draw(Rect(minX, minY + subHeight, subWidth, area.GetHeight() - subHeight * 2.0f), glm::vec4(1.0f, 1.0f, 1.0f, IsResizing() ? 0.9f : 0.65f));
					break;
				case 0b1000: //RIGHT
					spriteRenderer->Draw(Rect(maxX - subWidth, minY + subHeight, subWidth, area.GetHeight() - subHeight * 2.0f), glm::vec4(1.0f, 1.0f, 1.0f, IsResizing() ? 0.9f : 0.65f));
					break;
				case 0b101: //TOP/LEFT
					spriteRenderer->Draw(Rect(minX, minY, subWidth, subHeight), glm::vec4(1.0f, 1.0f, 1.0f, IsResizing() ? 0.9f : 0.65f));
					break;
				case 0b1001: //TOP/RIGHT
					spriteRenderer->Draw(Rect(maxX - subWidth, minY, subWidth, subHeight), glm::vec4(1.0f, 1.0f, 1.0f, IsResizing() ? 0.9f : 0.65f));
					break;
				case 0b110: //BOTTOM/LEFT
					spriteRenderer->Draw(Rect(minX, maxY - subHeight, subWidth, subHeight), glm::vec4(1.0f, 1.0f, 1.0f, IsResizing() ? 0.9f : 0.65f));
					break;
				case 0b1010: //BOTTOM/RIGHT
					spriteRenderer->Draw(Rect(maxX - subWidth, maxY - subHeight, subWidth, subHeight), glm::vec4(1.0f, 1.0f, 1.0f, IsResizing() ? 0.9f : 0.65f));
					break;
				default:
					spriteRenderer->Draw(Rect(minX + subWidth, minY, area.GetWidth() - subWidth * 2.0f, subHeight), glm::vec4(1.0f, 1.0f, 1.0f, 0.65f));
					spriteRenderer->Draw(Rect(minX + subWidth, maxY - subHeight, area.GetWidth() - subWidth * 2.0f, subHeight), glm::vec4(1.0f, 1.0f, 1.0f, 0.65f));
					spriteRenderer->Draw(Rect(minX, minY + subHeight, subWidth, area.GetHeight() - subHeight * 2.0f), glm::vec4(1.0f, 1.0f, 1.0f, 0.65f));
					spriteRenderer->Draw(Rect(maxX - subWidth, minY + subHeight, subWidth, area.GetHeight() - subHeight * 2.0f), glm::vec4(1.0f, 1.0f, 1.0f, 0.65f));
					spriteRenderer->Draw(Rect(minX, minY, subWidth, subHeight), glm::vec4(1.0f, 1.0f, 1.0f, 0.65f));
					spriteRenderer->Draw(Rect(maxX - subWidth, minY, subWidth, subHeight), glm::vec4(1.0f, 1.0f, 1.0f, 0.65f));
					spriteRenderer->Draw(Rect(minX, maxY - subHeight, subWidth, subHeight), glm::vec4(1.0f, 1.0f, 1.0f, 0.65f));
					spriteRenderer->Draw(Rect(maxX - subWidth, maxY - subHeight, subWidth, subHeight), glm::vec4(1.0f, 1.0f, 1.0f, 0.65f));
					break;
			}
		}
		else
		{
			spriteRenderer->Draw(Rect(minX + subWidth, minY, area.GetWidth() - subWidth * 2.0f, subHeight), glm::vec4(1.0f, 1.0f, 1.0f, 0.65f));
			spriteRenderer->Draw(Rect(minX + subWidth, maxY - subHeight, area.GetWidth() - subWidth * 2.0f, subHeight), glm::vec4(1.0f, 1.0f, 1.0f, 0.65f));
			spriteRenderer->Draw(Rect(minX, minY + subHeight, subWidth, area.GetHeight() - subHeight * 2.0f), glm::vec4(1.0f, 1.0f, 1.0f, 0.65f));
			spriteRenderer->Draw(Rect(maxX - subWidth, minY + subHeight, subWidth, area.GetHeight() - subHeight * 2.0f), glm::vec4(1.0f, 1.0f, 1.0f, 0.65f));
			spriteRenderer->Draw(Rect(minX, minY, subWidth, subHeight), glm::vec4(1.0f, 1.0f, 1.0f, 0.65f));
			spriteRenderer->Draw(Rect(maxX - subWidth, minY, subWidth, subHeight), glm::vec4(1.0f, 1.0f, 1.0f, 0.65f));
			spriteRenderer->Draw(Rect(minX, maxY - subHeight, subWidth, subHeight), glm::vec4(1.0f, 1.0f, 1.0f, 0.65f));
			spriteRenderer->Draw(Rect(maxX - subWidth, maxY - subHeight, subWidth, subHeight), glm::vec4(1.0f, 1.0f, 1.0f, 0.65f));
		}
	}
}

GUI_ANCHOR_POINT GUIWidget::GetResizeAnchorPoint(const glm::vec2& mousePosition) const
{
	GUI_ANCHOR_POINT resizePoint = static_cast<GUI_ANCHOR_POINT>(0);

	float subWidth = std::min(area.GetWidth() * 0.2f, 32.0f);
	float subHeight = std::min(area.GetHeight() * 0.2f, 32.0f);

	float minX = area.GetMinPosition().x;
	float maxX = area.GetMaxPosition().x;

	//Left resize
	if(mousePosition.x - minX >= 0.0f && mousePosition.x - minX <= subWidth)
		resizePoint = GUI_ANCHOR_POINT::LEFT;
	else if(maxX - mousePosition.x >= 0.0f && maxX - mousePosition.x <= subWidth)
		resizePoint = GUI_ANCHOR_POINT::RIGHT;

	float minY = area.GetMinPosition().y;
	float maxY = area.GetMaxPosition().y;

	//Top resize
	if(mousePosition.y - minY >= 0.0f && mousePosition.y - minY <= subHeight)
		resizePoint |= GUI_ANCHOR_POINT::TOP;
	else if(maxY - mousePosition.y >= 0.0f && maxY - mousePosition.y <= subHeight)
		resizePoint |= GUI_ANCHOR_POINT::BOTTOM;

	return resizePoint;
}

void GUIWidget::SetSize(float x, float y, GUI_ANCHOR_POINT anchorPoint)
{
	glm::vec2 position = area.GetMinPosition();

	if(anchorPoint != (GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT))
	{
		//SetPosition needs to be called
		glm::vec2 newPosition = area.GetMinPosition();
		glm::vec2 sizeDelta = area.GetSize() - glm::vec2(x, y);

		if(anchorPoint & GUI_ANCHOR_POINT::LEFT)
			newPosition.x -= sizeDelta.x;
		if(anchorPoint & GUI_ANCHOR_POINT::TOP)
			newPosition.y -= sizeDelta.y;

		//Cap min point
		if(newPosition.x + sizeDelta.x < minPosition.x)
		{
			x = area.GetMaxPosition().x - minPosition.x;

			newPosition.x = minPosition.x - sizeDelta.x;
		}
		if(newPosition.y + sizeDelta.y < minPosition.y)
		{
			y = area.GetMaxPosition().y - minPosition.y;

			newPosition.y = minPosition.y - sizeDelta.y;
		}

		SetPosition(newPosition + sizeDelta);

		position = area.GetMinPosition();
	}

	//Cap max point
	if(position.x + x > maxPosition.x)
		x = maxPosition.x - position.x;

	if(position.y + y > maxPosition.y)
		y = maxPosition.y - position.y;

	area.SetSize(x, y);
}

void GUIWidget::SubDraw(SpriteRenderer* spriteRenderer)
{

}

bool GUIWidget::HasChanged()
{
	bool returnValue = hasChanged;

	hasChanged = false;

	return returnValue;
}

bool GUIWidget::OnMouseDown(const MouseButtonState& keyState, const glm::vec2& mousePosition)
{
	if(!isMoving
	   && !isResizing
	   && keyState.button == moveResizeButton
	   && keyState.mods == moveResizeModifierKeys)
	{
		resizeAnchorPoint = GetResizeAnchorPoint(mousePosition);
		
		if(resizeAnchorPoint == GUI_ANCHOR_POINT::NONE
		   || !allowResize)
		{
			if(allowMove)
			{
				update = true;
				isMoving = true;

				relativeGrabPosition = mousePosition - area.GetMinPosition();

				OnMoveBegin();
			}
			else
				return SubOnMouseDown(keyState, mousePosition);
		}
		else
		{
			if(allowResize)
			{
				update = true;
				isResizing = true;

				grabPosition = mousePosition;

				OnResizeBegin();
			}
			else
				return SubOnMouseDown(keyState, mousePosition);
		}

		return true;
	}
	else
		return SubOnMouseDown(keyState, mousePosition);
}

void GUIWidget::OnMouseUp(const MouseButtonState& keyState, const glm::vec2& mousePosition)
{
	if(isMoving && keyState.button == moveResizeButton)
	{
		isMoving = false;

		OnMoveEnd();

		if(!IsMouseInside())
			Deactivate();
	}
	else if(isResizing && keyState.button == moveResizeButton)
	{
		isResizing = false;

		OnResizeEnd();

		this->resizeAnchorPoint = GUI_ANCHOR_POINT::NONE;

		if(!IsMouseInside())
			Deactivate();
	}
	else
		SubOnMouseUp(keyState, mousePosition);
}

bool GUIWidget::OnKeyDown(const KeyState& keyState)
{
	if(keyState.mods == moveResizeModifierKeys
	   && allowMove
	   && allowResize)
	{
		moveResizeModifierKeysHeld = true;

		return true;
	}
	else
		return SubOnKeyDown(keyState);
}

void GUIWidget::OnKeyUp(const KeyState& keyState)
{
	if(keyState.mods != moveResizeModifierKeys)
		moveResizeModifierKeysHeld = false;

	if(isMoving 
	   && (keyState.mods != moveResizeModifierKeys))
	{
		isMoving = false;

		OnMoveEnd();

		if(!IsMouseInside())
			Deactivate();
	}
	else if(isResizing
			&& (keyState.mods != moveResizeModifierKeys))
	{
		isResizing = false;

		OnResizeEnd();

		this->resizeAnchorPoint = GUI_ANCHOR_POINT::NONE;

		if(!IsMouseInside())
			Deactivate();
	}
	else
		SubOnKeyUp(keyState);
}

void GUIWidget::OnMoveBegin()
{ }

void GUIWidget::OnMoveEnd()
{ }

void GUIWidget::OnResizeBegin()
{ }

void GUIWidget::OnResizeEnd()
{ }

bool GUIWidget::GetUpdateWhenMoving() const
{
	return updateWhenMoving;
}

bool GUIWidget::GetUpdateWhenResizing() const
{
	return updateWhenResizing;
}

glm::vec2 GUIWidget::GetMinPosition() const
{
	return minPosition;
}

glm::vec2 GUIWidget::GetMaxPosition() const
{
	return maxPosition;
}

glm::vec2 GUIWidget::GetMinSize() const
{
	return minSize;
}

glm::vec2 GUIWidget::GetMaxSize() const
{
	return maxSize;
}

void GUIWidget::SetDraw(bool draw)
{
	GUIContainer::SetDraw(draw);

	if(!draw)
		Deactivate();
}

void GUIWidget::SetUpdateWhenMoving(bool updateWhenMoving)
{
	this->updateWhenMoving = updateWhenMoving;
}

void GUIWidget::SetUpdateWhenResizing(bool updateWhenResizing)
{
	this->updateWhenResizing = updateWhenResizing;
}

void GUIWidget::SetMinPosition(glm::vec2 minPosition)
{
	this->minPosition = minPosition;
}

void GUIWidget::SetMaxPosition(glm::vec2 maxPosition)
{
	this->maxPosition = maxPosition;
}

void GUIWidget::SetMinSize(glm::vec2 minSize)
{
	this->minSize = minSize;
}

void GUIWidget::SetMaxSize(glm::vec2 maxSize)
{
	this->maxSize = maxSize;
}

void GUIWidget::SubUpdate(std::chrono::nanoseconds delta)
{ }

bool GUIWidget::SubOnMouseDown(const MouseButtonState& keyState, const glm::vec2& mousePosition)
{
	return false;
}

void GUIWidget::SubOnMouseUp(const MouseButtonState& keyState, const glm::vec2& mousePosition)
{ }

bool GUIWidget::SubOnKeyDown(const KeyState& keyState)
{
	return false;
}

void GUIWidget::SubOnKeyUp(const KeyState& keyState)
{ }

bool GUIWidget::OnMouseEnter()
{
	if(this->draw)
	{
		Activate();
		return true;
	}

	return false;
}

void GUIWidget::OnMouseExit()
{
	if(!isMoving && !isResizing)
		Deactivate();
}

bool GUIWidget::IsMoving() const
{
	return isMoving;
}

bool GUIWidget::IsResizing() const
{
	return isResizing;
}

void GUIWidget::Activate()
{
	this->receiveAllEvents = true;
	this->moveResizeModifierKeysHeld = Input::GetAsyncKeyModifierState(moveResizeModifierKeys);
}

void GUIWidget::Deactivate()
{
	this->receiveAllEvents = false;
	this->update = false;
	this->hasChanged = false;
	this->isMoving = false;
	this->isResizing = false;
	this->moveResizeModifierKeysHeld = false;
	this->resizeAnchorPoint = GUI_ANCHOR_POINT::NONE;
}
