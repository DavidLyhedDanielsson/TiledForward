#ifndef DropdownButton_h__
#define DropdownButton_h__


#include "guiContainer.h"
#include "button.h"
#include "list.h"
#include "textBox.h"

#include <vector>

class DropdownMenu
	: public Button
{
	friend class GUIManager;
public:

	DropdownMenu();
	virtual ~DropdownMenu() = default;

	bool Init(Rect area
			  , std::shared_ptr<ButtonStyle>& buttonStyle
			  , std::shared_ptr<GUIBackgroundStyle>& buttonBackgroundStyle
			  , std::vector<std::string> alternatives
			  , ContentManager* contentManager
			  , std::function<void(std::string)> callbackFunction
			  , const std::string& initialText = ""
			  , GUI_ANCHOR_POINT anchorPoint = GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT);
	bool Init(glm::vec2 position
			  , glm::vec2 size
			  , std::shared_ptr<ButtonStyle>& buttonStyle
			  , std::shared_ptr<GUIBackgroundStyle>& buttonBackgroundStyle
			  , std::vector<std::string> alternatives
			  , ContentManager* contentManager
			  , std::function<void(std::string)> callbackFunction
			  , const std::string& initialText = ""
			  , GUI_ANCHOR_POINT anchorPoint = GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT);

	void Update(std::chrono::nanoseconds delta) override;

	void Draw(SpriteRenderer* spriteRenderer) override;

	bool OnMouseEnter() override;
	void OnMouseExit() override;

	bool OnMouseDown(const MouseButtonState& keyState, const glm::vec2& mousePosition) override;
	void OnMouseUp(const MouseButtonState& keyState, const glm::vec2& mousePosition) override;

	using Button::SetPosition;
	void SetPosition(float x, float y, GUI_ANCHOR_POINT anchorPoint) override;

	using Button::SetSize;
	void SetSize(float x, float y, GUI_ANCHOR_POINT anchorPoint) override;

	void Highlight() override;
	void UnHighlight() override;

	void Activate() override;
	void Deactivate() override;

	void SetCallbackFunction(std::function<void(std::string)> callbackFunction) override;

private:
	Rect originalArea;

	List list;
	std::vector<std::unique_ptr<Button>> buttons;

	std::shared_ptr<GUIStyle> buttonStyle;

	std::function<void(std::string)> callbackFunction;

	void ListButtonPressed(const std::string& buttonText);

	void ShowList();
	void HideList();
};


#endif // DropdownButton_h__
