#include "guiWidgetStyled.h"

#include "../content/contentManager.h"


using floatLimits = std::numeric_limits<float>;

GUIWidgetStyled::GUIWidgetStyled()
	: GUIWidget()
{ }

GUIWidgetStyled::~GUIWidgetStyled()
{ }

bool GUIWidgetStyled::Init(ContentManager* contentManager
							, Rect area
							, const std::shared_ptr<GUIBackgroundStyle>& backgroundStyle
							, bool allowMove
							, bool updateWhenMoving
							, bool allowResize
							, bool updateWhenResizing
							, MOUSE_BUTTON moveResizeButton /*= MOUSE_BUTTON::LEFT */
							, KEY_MODIFIERS moveResizeModifierKeys /*= KEY_MODIFIERS::CONTROL */
							, GUI_ANCHOR_POINT anchorPoint /*= GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT */
							, glm::vec2 minPosition /*= glm::vec2(std::numeric_limits<float>::min(), std::numeric_limits<float>::min()) */
							, glm::vec2 maxPosition /*= glm::vec2(std::numeric_limits<float>::max(), std::numeric_limits<float>::max()) */
							, glm::vec2 minSize /*= glm::vec2(std::numeric_limits<float>::min(), std::numeric_limits<float>::min()) */
							, glm::vec2 maxSize /*= glm::vec2(std::numeric_limits<float>::max(), std::numeric_limits<float>::max())*/)
{
	return Init(contentManager
				, area.GetMinPosition()
				, area.GetSize()
				, backgroundStyle
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

bool GUIWidgetStyled::Init(ContentManager* contentManager
							, glm::vec2 position
							, glm::vec2 size
							, const std::shared_ptr<GUIBackgroundStyle>& backgroundStyle
							, bool allowMove
							, bool updateWhenMoving
							, bool allowResize
							, bool updateWhenResizing
							, MOUSE_BUTTON moveResizeButton /*= MOUSE_BUTTON::LEFT */
							, KEY_MODIFIERS moveResizeModifierKeys /*= KEY_MODIFIERS::CONTROL */
							, GUI_ANCHOR_POINT anchorPoint /*= GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT */
							, glm::vec2 minPosition /*= glm::vec2(std::numeric_limits<float>::min(), std::numeric_limits<float>::min()) */
							, glm::vec2 maxPosition /*= glm::vec2(std::numeric_limits<float>::max(), std::numeric_limits<float>::max()) */
							, glm::vec2 minSize /*= glm::vec2(std::numeric_limits<float>::min(), std::numeric_limits<float>::min()) */
							, glm::vec2 maxSize /*= glm::vec2(std::numeric_limits<float>::max(), std::numeric_limits<float>::max())*/)
{
	GUIWidget::Init(contentManager
					, position
					, size
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

	this->background = backgroundStyle->CreateBackground();
	this->background->Init(backgroundStyle, &this->area);

	return true;
}

void GUIWidgetStyled::SetPosition(float x, float y, GUI_ANCHOR_POINT anchorPoint)
{
	GUIContainer::SetPosition(x, y, anchorPoint);

	background->AreaChanged();
}

void GUIWidgetStyled::SetSize(float x, float y, GUI_ANCHOR_POINT anchorPoint)
{
	GUIContainer::SetSize(x, y, anchorPoint);

	background->AreaChanged();
}