#ifndef guiWidgetStyled_h__
#define guiWidgetStyled_h__

#include "guiWidget.h"
#include "guiBackground.h"
#include "guiBackgroundStyle.h"

#include <memory>



class ContentManager;

class GUIWidgetStyled :
	public GUIWidget
{
public:
	GUIWidgetStyled();
	virtual ~GUIWidgetStyled();

	using GUIWidget::SetPosition;
	virtual void SetPosition(float x, float y, GUI_ANCHOR_POINT anchorPoint) override;
	using GUIWidget::SetSize;
	virtual void SetSize(float x, float y, GUI_ANCHOR_POINT anchorPoint) override;

protected:
	virtual bool Init(ContentManager* contentManager
					  , Rect area
					  , const std::shared_ptr<GUIBackgroundStyle>& backgroundStyle
					  , bool allowMove
					  , bool updateWhenMoving
					  , bool allowResize
					  , bool updateWhenResizing
					  , MOUSE_BUTTON moveResizeButton = MOUSE_BUTTON::LEFT
					  , KEY_MODIFIERS moveResizeModifierKeys = KEY_MODIFIERS::CONTROL
					  , GUI_ANCHOR_POINT anchorPoint = GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT
					  , glm::vec2 minPosition = glm::vec2(std::numeric_limits<float>::min(), std::numeric_limits<float>::min())
					  , glm::vec2 maxPosition = glm::vec2(std::numeric_limits<float>::max(), std::numeric_limits<float>::max())
					  , glm::vec2 minSize = glm::vec2(std::numeric_limits<float>::min(), std::numeric_limits<float>::min())
					  , glm::vec2 maxSize = glm::vec2(std::numeric_limits<float>::max(), std::numeric_limits<float>::max()));

	virtual bool Init(ContentManager* contentManager
					  , glm::vec2 position
					  , glm::vec2 size
					  , const std::shared_ptr<GUIBackgroundStyle>& backgroundStyle
					  , bool allowMove
					  , bool updateWhenMoving
					  , bool allowResize
					  , bool updateWhenResizing
					  , MOUSE_BUTTON moveResizeButton = MOUSE_BUTTON::LEFT
					  , KEY_MODIFIERS moveResizeModifierKeys = KEY_MODIFIERS::CONTROL
					  , GUI_ANCHOR_POINT anchorPoint = GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT
					  , glm::vec2 minPosition = glm::vec2(std::numeric_limits<float>::min(), std::numeric_limits<float>::min())
					  , glm::vec2 maxPosition = glm::vec2(std::numeric_limits<float>::max(), std::numeric_limits<float>::max())
					  , glm::vec2 minSize = glm::vec2(std::numeric_limits<float>::min(), std::numeric_limits<float>::min())
					  , glm::vec2 maxSize = glm::vec2(std::numeric_limits<float>::max(), std::numeric_limits<float>::max()));

	std::unique_ptr<GUIBackground> background;
};

#endif // guiWidgetStyled_h__
