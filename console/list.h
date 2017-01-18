#ifndef List_h__
#define List_h__

#include "guiContainerStyled.h"
#include "guiManager.h"
#include "scrollbar.h"

#include "listStyle.h"

#include <vector>

class List :
	public GUIContainerStyled
{
friend class GUIManager;
public:
	List();
	~List() = default;

	List(List&&) = default;
	List& operator=(List&&) = default;

	void Init(Rect area
			  , const std::shared_ptr<ListStyle>& style
			  , const std::shared_ptr<GUIBackgroundStyle>& backgroundStyle
			  , GUI_ANCHOR_POINT anchorPoint = GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT);

	void Init(glm::vec2 position
			  , glm::vec2 size
			  , const std::shared_ptr<ListStyle>& style
			  , const std::shared_ptr<GUIBackgroundStyle>& backgroundStyle
			  , GUI_ANCHOR_POINT anchorPoint = GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT);

	void Update(std::chrono::nanoseconds delta) override;

	void Draw(SpriteRenderer* spriteRenderer) override;

	using GUIContainerStyled::SetPosition;
	void SetPosition(float x, float y, GUI_ANCHOR_POINT anchorPoint) override;
	using GUIContainerStyled::SetSize;
	void SetSize(float x, float y, GUI_ANCHOR_POINT anchorPoint) override; //TODO: Crashes if size is bigger than there are elements (height-wise) because of UpdatePositions
	//TODO: If this is ever needed make sure to pay attention to shared_ptr
	//void AddElement(GUIContainer* element); 
	void SetElements(const std::vector<GUIContainer*> elements); //TODO: UpdateElements? Same container getting multiple MouseEnter events?
	std::vector<GUIContainer*> GetElements();
	void ClearElements();

	void HighlightElement(int index);
	void UnHighlightElement();
	void ClearHighlights();
	int GetHighlitElementIndex();
	GUIContainer* GetHighlitElement();

	void SetIgnoreMouse(bool ignore);

	bool OnMouseDown(const MouseButtonState& keyState, const glm::vec2& mousePosition) override;
	void OnMouseUp(const MouseButtonState& keyState, const glm::vec2& mousePosition) override;
	bool OnKeyDown(const KeyState& keyState) override;
	void OnKeyUp(const KeyState& keyState) override;
	bool OnChar(unsigned int keyCode) override;
	bool OnScroll(int distance) override;

	int GetElementsSize() const;
	bool GetIsScrolling() const;

	int GetScrollbarWidth() const;

	static std::shared_ptr<ListStyle> GenerateDefaultStyle(ContentManager* contentManager);
	static std::unique_ptr<GUIBackground> GenerateDefaultBackground(ContentManager* contentManager);
	static std::shared_ptr<GUIBackgroundStyle> GenerateDefaultBackgroundStyle(ContentManager* contentManager);
private:
	GUIManager guiManager;

	std::vector<GUIContainer*> elements;

	GUIContainer* focusOn;

	std::shared_ptr<ListStyle> style;

	Scrollbar scrollbar;
	int drawBegin;
	int drawEnd;

	//int elementHeight;

	//TODO: Support multiple highlights
	int highlitElement;

	bool ignoreMouse;
	bool scrolling;

	bool OnMouseEnter() override;
	void OnMouseExit() override;

	void UpdatePositions();

	void ScrollFunction();
};

#endif // List_h__
