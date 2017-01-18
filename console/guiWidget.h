#ifndef guiWidget_h__
#define guiWidget_h__

#include "guiContainer.h"

class ContentManager;

class GUIWidget :
	public GUIContainer
{
public:
	GUIWidget();
	virtual ~GUIWidget();

	GUIWidget(GUIWidget&&) = default;
	GUIWidget& operator=(GUIWidget&&) = default;

	virtual void Update(std::chrono::nanoseconds delta) override;

	/**
	* \See SubDraw
	*/
	virtual void Draw(SpriteRenderer* spriteRenderer) override;

	/**
	* Called instead of Draw when inheriting from GUIWidget.
	*
	* This method only draws some small indicators (when hovering over the container)
	* To show the user that the container can be resized
	*/
	virtual void SubDraw(SpriteRenderer* spriteRenderer);

	virtual bool HasChanged();

	/**
	* \see SubOnMouseDown
	*/
	virtual bool OnMouseDown(const MouseButtonState& keyState, const glm::vec2& mousePosition) override;
	/**
	* \see SubOnMouseUp
	*/
	virtual void OnMouseUp(const MouseButtonState& keyState, const glm::vec2& mousePosition) override;
	/**
	* \see SubOnKeyDown
	*/
	virtual bool OnKeyDown(const KeyState& keyState) override;
	/**
	* \see SubOnKeyUp
	*/
	virtual void OnKeyUp(const KeyState& keyState) override;

	/**
	* Called when the user starts moving this container
	*/
	virtual void OnMoveBegin();
	/**
	* Called when the user releases this container after moving it
	*/
	virtual void OnMoveEnd();

	/**
	* Called when the user starts resizing this container
	*/
	virtual void OnResizeBegin();
	/**
	* Called when the user releases this container after resizing it
	*/
	virtual void OnResizeEnd();

	bool GetUpdateWhenMoving() const;
	bool GetUpdateWhenResizing() const;
	glm::vec2 GetMinPosition() const;
	glm::vec2 GetMaxPosition() const;
	glm::vec2 GetMinSize() const;
	glm::vec2 GetMaxSize() const;

	void SetDraw(bool draw) override;
	void SetUpdateWhenMoving(bool updateWhenMoving);
	void SetUpdateWhenResizing(bool updateWhenResizing);
	void SetMinPosition(glm::vec2 minPosition);
	void SetMaxPosition(glm::vec2 maxPosition);
	void SetMinSize(glm::vec2 minSize);
	void SetMaxSize(glm::vec2 maxSize);

protected:
	bool allowMove;
	bool allowResize;
	bool hasChanged;

	virtual bool Init(ContentManager* contentManager
					  , Rect area
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

	/**
	* Called instead of Update when inheriting from GUIWidget.
	*
	* Only gets called when the window isn't resizing or moving, unless #updateWhenMoving or #updateWhenResizing is set to true
	*
	* \param delta
	*/
	virtual void SubUpdate(std::chrono::nanoseconds delta);
	/**
	* Called as usual when any mouse button is pressed, except for when
	* the event is related to resizing or moving this container
	*
	* \param keyState
	* \param mousePosition
	*/
	virtual bool SubOnMouseDown(const MouseButtonState& keyState, const glm::vec2& mousePosition);
	/**
	* Called as usual when any mouse button is released, except for when
	* the event is related to resizing or moving this container
	*
	* \param keyState
	* \param mousePosition
	*/
	virtual void SubOnMouseUp(const MouseButtonState& keyState, const glm::vec2& mousePosition);
	/**
	* Called as usual when any button is pressed, except for when
	* the event is related to resizing or moving this container
	*
	* \param keyState
	*/
	virtual bool SubOnKeyDown(const KeyState& keyState);
	/**
	* Called as usual when any button is released, except for when
	* the event is related to resizing or moving this container
	*
	* \param keyState
	*/
	virtual void SubOnKeyUp(const KeyState& keyState);

	/**
	* Sets #receiveAllEvents to true if #draw is true
	*/
	virtual bool OnMouseEnter() override;
	/**
	* Sets #receiveAllEvents to false if #isMoving is false
	*/
	virtual void OnMouseExit() override;

	bool IsMoving() const;
	bool IsResizing() const;

	virtual void Activate() override;
	virtual void Deactivate() override;

	GUI_ANCHOR_POINT GetResizeAnchorPoint(const glm::vec2& mousePosition) const;

	using GUIContainer::SetSize;
	virtual void SetSize(float x, float y, GUI_ANCHOR_POINT anchorPoint) override;

private:
	MOUSE_BUTTON moveResizeButton;
	KEY_MODIFIERS moveResizeModifierKeys;

	bool moveResizeModifierKeysHeld;

	//////////////////////////////////////////////////
	//Moving
	//////////////////////////////////////////////////
	bool isMoving;
	bool updateWhenMoving;

	glm::vec2 minPosition;
	glm::vec2 maxPosition;
	glm::vec2 relativeGrabPosition;

	//////////////////////////////////////////////////
	//Resizing
	//////////////////////////////////////////////////
	bool isResizing;
	bool updateWhenResizing;

	glm::vec2 minSize;
	glm::vec2 maxSize;
	glm::vec2 grabPosition;

	GUI_ANCHOR_POINT resizeAnchorPoint;

};

#endif // guiWidget_h__
