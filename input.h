#ifndef Input_h__
#define Input_h__

#include <functional>
#include <chrono>
#include <vector>
#include <unordered_set>

#include "keyState.h"
#include "timer.h"

#if defined(USE_DX) && !defined(USE_GLM)
#include <windows.h>

typedef glm::vec2 float2
typedef DirectX::XMINT2 int2
#else
#include <glm/vec2.hpp>

typedef glm::vec2 float2;
typedef glm::ivec2 int2;
#endif

#ifdef _WIN32
#include <windows.h>
#else // _WIN32
#include <X11/X.h>
#include <X11/Xlib.h>
#endif // _WIN32

class OSWindow;

class Input
{
public:
     /**
      * Initializes input and sets up the OSWindow to listen to
      *
      * @param window
      * @param ignoreInputIfWindowsIsPaused if this is true, no events will be
      * sent if window->IsPaused(), also, GetMouseDelta will return (0.0f, 0.0f)
      */
	static void Init(OSWindow* window, bool ignoreInputIfWindowsIsPaused);
	/**
	* Updates mouse delta and locks the cursor if cursor locking is on
	* 
	* \see LockCursor
	*/
	static void Update();

	/**@{*/
	
	/**
	* Sets the callback function for a given action
	*
	* @param callback
	*/
	static void RegisterKeyCallback(std::function<void(const KeyState&)> callback);
	static void RegisterMouseButtonCallback(std::function<void(const MouseButtonState&)> callback);
	static void RegisterCharCallback(std::function<void(int)> callback);
	static void RegisterScrollCallback(std::function<void(int)> callback);
	/**@}*/

	/**
	* Locks the cursor to the given position.
	*
	* Position is relative to the window. Set xPosition and yPosition to -1 to stop locking
	* 
	* @param xPosition x position to lock to or -1 to stop locking
	* @param yPosition y position to lock to or -1 to stop locking
	*/
	static void LockCursor(int xPosition, int yPosition);

	static void SetCursorPosition(int xPosition, int yPosition);

	/**
	* Hides the cursor
	*/
	static void HideCursor();
	/**
	* Shows the cursor
	*/
	static void ShowCursor();

    /**
	* Sets the maximum time (in milliseconds) for two consecutive clicks to count as double clicks
	*
	* @param ms
	*/
    static void SetDoubleClickDelay(unsigned int ms);

	/**
	* Gets the position of the mouse when the given button was pressed
	*
	* @param button
	*
	* @return cursor position
	*/
	static float2 GetMouseDownPos(MOUSE_BUTTON button);
	/**
	* Gets the position of the mouse when the given button was released
	*
	* @param button
	*
	* @return cursor position
	*/
	static float2 GetMouseUpPos(MOUSE_BUTTON button);

	/**
	* Gets the current mouse position
	*
	* @return the current position of the mouse
	*/
	static float2 GetMousePosition();
	/**
	* Gets how far the mouse has moved since the last time Update() was called
	*
	* @return the distance the mouse has moved
	*/
	static float2 GetMouseDelta();
	/**
	* Whether or not the mouse has moved since the last time Update() was called
	*
	* @return whether or not the mouse has moved
	*/
	static bool MouseMoved();

	/**
	* Gets a pointer to the window that Input currently listens to
	*
	* @return
	*/
	static OSWindow* GetListenWindow();

	/**
	 * Checks whether or not the given key is currently down
	 *
	 * @param keyCode
	 * @return
	 */
	static bool GetAsyncKeyState(KEY_CODE keyCode);

	/**@{*/
    static void KeyEvent(
#ifdef _WIN32
                         UINT msg, WPARAM wParam, LPARAM lParam);
#else // _WIN32
                         XKeyEvent event);
#endif // _WIN32


    static void MouseButtonEvent(
#ifdef _WIN32
                                 UINT msg, WPARAM wParam);
#else // _WIN32
                                 XButtonEvent event);
#endif // _WIN32

    static void CharEvent(unsigned int key);
	static void ScrollEvent(int distance);
	/**@}*/
	static bool GetAsyncKeyModifierState(KEY_MODIFIERS modifier);
private:
	Input();
	~Input();

    static KEY_CODE OSKeyToKEY_CODE(
#ifdef _WIN32
		WPARAM wParam
#else // WIN32
    KeySym keySym
#endif // WIN32
    );

    static
#ifdef _WIN32
    WPARAM
#else // WIN32
    KeySym
#endif // WIN32
    KEY_CODEToOSKey(KEY_CODE keyCode);

	static OSWindow* listenWindow;

	static std::function<void(const KeyState&)> keyCallback;
	static std::function<void(const MouseButtonState&)> mouseButtonCallback;
	static std::function<void(int)> charCallback;
	static std::function<void(int)> scrollCallback;

	////////////////////////////////////////////////////////////
	//MOUSE
	////////////////////////////////////////////////////////////
	static float2 mouseDownPos[(int)MOUSE_BUTTON::COUNT];
	static float2 mouseUpPos[(int)MOUSE_BUTTON::COUNT];
	static float2 mousePosition;
	static float2 oldMousePosition;

	static MOUSE_BUTTON lastMouseButton;
	static Timer lastClickTimer;
	static std::chrono::milliseconds doubleClickDelay;

	const static int maxDoubleClickDistance = 2;

	static int xLock;
	static int yLock;

	static bool showCursor;
    static bool ignoreIfPaused;

    static int2 QueryMousePosition();

#ifndef _WIN32
    static std::unordered_set<int> keysDown;
#endif // _WIN32
};

#endif // Input_h__