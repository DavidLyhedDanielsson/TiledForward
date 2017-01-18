#ifndef checkbox_h__
#define checkbox_h__

#include "guiContainerStyled.h"

#include <functional>

#include "buttonStyle.h"
#include "guiBackground.h"

class Checkbox :
	public GUIContainerStyled
{
public:
	Checkbox();
	~Checkbox() = default;

	virtual bool Init(Rect area
					  , const std::shared_ptr<GUIBackgroundStyle>& backgroundStyle
					  , GUI_ANCHOR_POINT anchorPoint = GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT);
	virtual bool Init(glm::vec2 position
					  , glm::vec2 size
					  , const std::shared_ptr<GUIBackgroundStyle>& backgroundStyle
					  , GUI_ANCHOR_POINT anchorPoint = GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT);

	void Draw(SpriteRenderer* spriteRenderer) override;

	bool OnMouseEnter() override;
	void OnMouseExit() override;
	bool OnMouseDown(const MouseButtonState& keyState, const glm::vec2& mousePosition) override;
	void OnMouseUp(const MouseButtonState& keyState, const glm::vec2& mousePosition) override;

	void SetCallbackFunction(std::function<void(std::string)> callbackFunction);

	void SetToggled(bool toggled);
	bool IsToggled();

	enum class STATES
	{
		UNTOGGLED = 0, TOGGLED, HELD, SIZE
	};
protected:
	bool toggled;
	STATES state;

	std::function<void(std::string)> callbackFunction;

	void Toggle();
};

#endif // checkbox_h__
