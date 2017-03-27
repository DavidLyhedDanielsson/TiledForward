#ifndef GUIContainer_h__
#define GUIContainer_h__

#include <chrono>

#include "../rect.h"
#include "../os/keyState.h"

#include "../logger.h"
#include <glm/glm.hpp>

#include "guiStyle.h"

class SpriteRenderer;

enum class GUI_ANCHOR_POINT : int
{
	NONE = 0
	, TOP = 0b1
	, BOTTOM = 0b10
	, LEFT = 0b100
	, RIGHT = 0b1000
	, MIDDLE = 0b1111
};

inline GUI_ANCHOR_POINT operator|(const GUI_ANCHOR_POINT lhs, const GUI_ANCHOR_POINT rhs)
{
	return static_cast<GUI_ANCHOR_POINT>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

inline GUI_ANCHOR_POINT& operator|=(GUI_ANCHOR_POINT& lhs, const GUI_ANCHOR_POINT rhs)
{
	lhs = static_cast<GUI_ANCHOR_POINT>(static_cast<int>(lhs) | static_cast<int>(rhs));

	return lhs;
}

/**
* Returns whether or not lhs contains lhs
*
* Example:
* if(someAnchorPoint & GUI_ANCHOR_POINT::LEFT)
*	//someAnchorPoint is (hopefully) either TOP | LEFT or BOTTOM | LEFT
* 
* \param lhs
* \param rhs
*/
inline bool operator&(GUI_ANCHOR_POINT lhs, GUI_ANCHOR_POINT rhs)
{
	return static_cast<GUI_ANCHOR_POINT>(static_cast<int>(lhs) & static_cast<int>(rhs)) == rhs;
}

class GUIContainer
{
	friend class GUIManager;
public:
	GUIContainer();
	virtual ~GUIContainer() = default;

	GUIContainer(GUIContainer&&) = default;
	GUIContainer& operator=(GUIContainer&&) = default;

	virtual bool Init(Rect area, GUI_ANCHOR_POINT anchorPoint = GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT);
	virtual bool Init(glm::vec2 position, glm::vec2 size, GUI_ANCHOR_POINT anchorPoint = GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT);

	/**
	* Called automatically by the GUIManager if #update is true
	*
	* \param delta
	*/
	virtual void Update(std::chrono::nanoseconds delta) {};
	/**
	* Called automatically by the GUIManager if #draw is true
	*
	* \param spriteRenderer
	* \param deviceContext
	*/
	virtual void Draw(SpriteRenderer* spriteRenderer) {};

	/**@{*/
	/**
	* Sets this GUIContainer's position.
	*
	* Each method eventually calls SetPosition(float, float, GUI_ANCHOR_POINT).
	*/
	virtual void SetPosition(const glm::vec2& newPosition);
	virtual void SetPosition(const glm::vec2& newPosition, GUI_ANCHOR_POINT anchorPoint);
	virtual void SetPosition(float x, float y);
	virtual void SetPosition(float x, float y, GUI_ANCHOR_POINT anchorPoint);
	/**@}*/
	/**@{*/
	/**
	* Sets this GUIContainer's size.
	*
	* Each method eventually calls SetSize(float, float, GUI_ANCHOR_POINT).
	* If \p anchorPoint is not set to top left, SetPosition(float, float) will also be called
	*/
	virtual void SetSize(const glm::vec2& newSize);
	virtual void SetSize(const glm::vec2& newSize, GUI_ANCHOR_POINT anchorPoint);
	virtual void SetSize(float x, float y);
	virtual void SetSize(float x, float y, GUI_ANCHOR_POINT anchorPoint);
	/**@}*/

	/**@{*/
	/**
	* General setters
	*/
	virtual void SetArea(const Rect& newArea);
	virtual void SetDraw(bool draw);
	virtual void SetUpdate(bool update);
	virtual void SetReceiveAllEvents(bool active);
	/**@}*/
	
	/**@{*/
	/**
	* General getters
	*/
	virtual Rect GetArea() const;
	virtual glm::vec2 GetPosition() const;
	virtual glm::vec2 GetSize() const;

	virtual bool GetDraw() const;
	virtual bool GetUpdate() const;
	virtual bool GetReceiveAllEvents() const;
	/**@}*/

	/**
	* Highlights this object.
	*
	* By default this does nothing, it has to be overriden
	*/
	virtual void Highlight()
	{ };
	/**
	* Un-highlights this object.
	*
	* By default this does nothing, it has to be overriden.
	*
	* I am fully aware that un-highlight probably isn't a word
	*/
	virtual void UnHighlight() {};

	/**
	* Activates this object. E.g. activating a text box should make it accept input.
	*
	* By default this does nothing, it has to be overriden
	*/
	virtual void Activate() {};
	/**
	* Deactivates this object. E.g. deactivating a text box should make it stop accepting input.
	*
	* By default this does nothing, it has to be overriden
	*/
	virtual void Deactivate() {};

	/**
	* Called by the GUIManager when \p receiveAllEvents is true and the user presses any mouse button
	*
	* \param keyState
	* \param mousePosition
	*/
	virtual bool OnMouseDown(const MouseButtonState& keyState, const glm::vec2& mousePosition)
	{
		return false;
	};
	/**
	* Called by the GUIManager when \p receiveAllEvents is true and the user releases any mouse button
	*
	* \param keyState
	* \param mousePosition
	*/
	virtual void OnMouseUp(const MouseButtonState& keyState, const glm::vec2& mousePosition)
	{};
	/**
	* Called by the GUIManager when \p receiveAllEvents is true and the user presses any key
	*
	* \param keyState
	*/
	virtual bool OnKeyDown(const KeyState& keyState)
	{
		return false;
	};
	/**
	* Called by the GUIManager when \p receiveAllEvents is true and the user releases any key
	*
	* \param keyState
	*/
	virtual void OnKeyUp(const KeyState& keyState)
	{};
	/**
	* Called by the GUIManager when \p receiveAllEvents is true and the user writes something
	*
	* \param keyCode
	*/
	virtual bool OnChar(unsigned int keyCode)
	{
		return false;
	};
	/**
	* Called by the GUIManager when \p receiveAllEvents is true and the user scrolls
	* 
	* \param distance
	*/
	virtual bool OnScroll(int distance)
	{
		return false;
	};

protected:
	/**
	* Called by the GUIManager whenever the cursor enters \p area.
	*/
	virtual bool OnMouseEnter() 
	{
		return false;
	};
	/**
	* Called by the GUIManager whenever the cursor exits \p area.
	*/
	virtual void OnMouseExit() {};

	/**
	* If this is true this object will receive mouse/key events,
	* otherwise it will only receive OnMouseEnter and OnMouseExit events
	*/
	bool receiveAllEvents;
	/**
	* Controls whether or not this object's update should be called
	*/
	bool update;
	/**
	* Controls whether or not this object's draw methods should be called
	*/
	bool draw;

	/**
	* The hitbox of this object
	*/
	Rect area;

	/**
	* Whether or not to clip (see GUIManager::Draw), TODO: improve
	*/
	bool clip;

	glm::vec2 CalculatePosition(glm::vec2 position, GUI_ANCHOR_POINT anchorPoint) const;
	glm::vec2 CalculatePosition(glm::vec2 position, glm::vec2 size, GUI_ANCHOR_POINT anchorPoint) const;

	/**
	* Returns whether or not the cursor is currently inside #area
	*/
	bool IsMouseInside() const;

private:
	bool mouseInside;
};

#endif // GUIContainer_h__
