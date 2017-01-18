#ifndef Button_h__
#define Button_h__

#include "guiContainerStyled.h"

#include <functional>

#include "buttonStyle.h"
#include "guiBackground.h"

class Button :
	public GUIContainerStyled
{
public:
	Button();
	virtual ~Button() = default;

	bool Init(const Rect& area
			  , const std::shared_ptr<ButtonStyle>& style
			  , const std::shared_ptr<GUIBackgroundStyle>& backgroundStyle
			  , std::function<void(std::string)> callbackFunction = nullptr
			  , const std::string& text = ""
			  , GUI_ANCHOR_POINT anchorPoint = GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT
			  , GUI_ANCHOR_POINT textAnchorPoint = GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT);

	bool Init(glm::vec2 position
			  , glm::vec2 size
			  , const std::shared_ptr<ButtonStyle>& style
			  , const std::shared_ptr<GUIBackgroundStyle>& backgroundStyle
			  , std::function<void(std::string)> callbackFunction = nullptr
			  , const std::string& text = ""
			  , GUI_ANCHOR_POINT anchorPoint = GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT
			  , GUI_ANCHOR_POINT textAnchorPoint = GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT);

	virtual void Draw(SpriteRenderer* spriteRenderer) override;

	virtual bool OnMouseEnter() override;
	virtual void OnMouseExit() override;
	virtual bool OnMouseDown(const MouseButtonState& keyState, const glm::vec2& mousePosition) override;
	virtual void OnMouseUp(const MouseButtonState& keyState, const glm::vec2& mousePosition) override;

	virtual void Highlight() override;
	virtual void UnHighlight() override;

	virtual void Activate() override;
	virtual void Deactivate() override;

	virtual void SetCallbackFunction(std::function<void(std::string)> callbackFunction);
	void SetText(const std::string& text);
	std::string GetText() const;

	using GUIContainerStyled::SetSize;
	virtual void SetSize(float x, float y, GUI_ANCHOR_POINT anchorPoint) override;

	bool IsDown() const;

protected:
	std::shared_ptr<ButtonStyle> style;


	std::string text;
	GUI_ANCHOR_POINT textAnchorPoint;

	int textXOffset;
	int textYOffset;

	ButtonStyle::BUTTON_STATES buttonState;

	void SetTextOffset();

	void SetState(ButtonStyle::BUTTON_STATES state);

private:
	std::function<void(std::string)> callbackFunction;
};

#endif // Button_h__
