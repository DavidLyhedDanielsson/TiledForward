#ifndef Scrollbar_h__
#define Scrollbar_h__

#include "guiContainerStyled.h"

#include "scrollbarStyle.h"
#include "guiManager.h"
#include "guiBackground.h"

#include <memory>

class ContentManager;

class Scrollbar :
	public GUIContainerStyled
{
friend class GUIManager;
public:
	Scrollbar();
	~Scrollbar() = default;

	void Init(Rect area
			  , const std::shared_ptr<ScrollbarStyle>& style
			  , const std::shared_ptr<GUIBackgroundStyle>& backgroundStyle
			  , std::function<void()> scrollCallback = nullptr
			  , GUI_ANCHOR_POINT anchorPoint = GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT);

	void Init(glm::vec2 position
			  , glm::vec2 size
			  , const std::shared_ptr<ScrollbarStyle>& style
			  , const std::shared_ptr<GUIBackgroundStyle>& backgroundStyle
			  , std::function<void()> scrollCallback = nullptr
			  , GUI_ANCHOR_POINT anchorPoint = GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT);

	void Update(std::chrono::nanoseconds delta) override;
	void Draw(SpriteRenderer* spriteRenderer) override;

	void Scroll(int index);
	void ScrollTo(int index);
	void ScrollTo(int index, int downOffset);

	void SetVisibleItems(int visibleItems);
	void SetMaxItems(int maxItems);

	int GetVisibleItems() const;
	int GetMaxItems() const;

	int GetMinIndex() const;
	int GetMaxIndex() const;

	void SetPosition(const glm::vec2& newPosition) override;
	void SetPosition(float x, float y) override;
	void SetSize(const glm::vec2& newSize) override;
	void SetSize(float x, float y) override;
	void SetArea(const Rect& newArea) override;

	bool OnMouseDown(const MouseButtonState& keyState, const glm::vec2& mousePosition) override;
	void OnMouseUp(const MouseButtonState& keyState, const glm::vec2& mousePosition) override;
	bool OnScroll(int distance) override;

	bool Contains(const glm::vec2& position) const;

	static std::shared_ptr<ScrollbarStyle> GenerateDefaultStyle(ContentManager* contentManager);
	static std::unique_ptr<GUIBackground> GenerateDefaultBackground(ContentManager* contentManager);
	static std::shared_ptr<GUIBackgroundStyle> GenerateDefaultBackgroundStyle(ContentManager* contentManager);
protected:
	std::function<void()> scrollCallback;

	bool mouseDown;
	bool lockedToBottom;

	int beginIndex;
	int endIndex;

	int visibleItems;
	int maxItems;

	//////////////////////////////////////////////////////////////////////////
	//THUMB
	//////////////////////////////////////////////////////////////////////////
	Rect thumb;

	int grabY;

	std::shared_ptr<ScrollbarStyle> style;

	//************************************
	// Method:      UpdateThumbSize
	// FullName:    Scrollbar::UpdateThumbSize
	// Access:      protected 
	// Returns:     void
	// Qualifier:  
	// Description: Updates the thumb size based on visible items and max items
	//************************************
	void UpdateThumbSize();
	//************************************
	// Method:      UpdateThumbIndex
	// FullName:    Scrollbar::UpdateThumbIndex
	// Access:      protected 
	// Returns:     void
	// Qualifier:  
	// Description: Updates the thumb blockIndex based on its position
	//************************************
	void UpdateThumbIndex();
	//************************************
	// Method:      UpdateThumbPosition
	// FullName:    Scrollbar::UpdateThumbPosition
	// Access:      protected 
	// Returns:     void
	// Qualifier:  
	// Description: Updates the thumb position based on its blockIndex
	//************************************
	void UpdateThumbPosition();

	void ClampThumbPosition();

	int GetEffectiveHeight();
	float GetThumbRelativeY();
};

#endif // Scrollbar_h__
