#ifndef GUIManager_h__
#define GUIManager_h__

#include <vector>

#include "guiContainer.h"

class SpriteRenderer;

class GUIManager
{
public:
	GUIManager();
	~GUIManager();

	void AddContainer(GUIContainer* container);
	void SetContainers(std::vector<GUIContainer*> containers);

	void SendToFront(GUIContainer* container);
	void SendToBack(GUIContainer* container);
	void SendToLayer(GUIContainer* container, int layer);

	void KeyEvent(const KeyState& keyState);
	void MouseEvent(const MouseButtonState& keyState);
	void CharEvent(unsigned int keyCode);
	void ScrollEvent(int distance);

	void Update(std::chrono::nanoseconds delta);
	void Draw(SpriteRenderer* spriteRenderer);

	void ClearContainers();

private:
	std::vector<GUIContainer*> containers;

	//Whenever a container received an OnMouseDown event it will become locked
	//When it receives a OnMouseUp event it will be unlocked
	//Only the locked container receives OnMouseEnter and OnMouseExit events
	GUIContainer* lockedContainer;
	GUIContainer* lastActiveContainer;
};

#endif // GUIManager_h__
