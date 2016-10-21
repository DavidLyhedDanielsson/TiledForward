#include "input.h"

#include "logger.h"
#include "window.h"

#if !defined(USE_DX) || defined(USE_GLM)
#include <glm/glm.hpp>
#else
#pragma error "Use DXMath here instead. See MouseButtonEvent"
#endif

OSWindow* Input::listenWindow;

std::function<void(const KeyState&)> Input::keyCallback;
std::function<void(const MouseButtonState&)> Input::mouseButtonCallback;
std::function<void(int)> Input::charCallback;
std::function<void(int)> Input::scrollCallback;

float2 Input::mouseDownPos[(int)MOUSE_BUTTON::COUNT];
float2 Input::mouseUpPos[(int)MOUSE_BUTTON::COUNT];

float2 Input::mousePosition;
float2 Input::oldMousePosition;

MOUSE_BUTTON Input::lastMouseButton;
Timer Input::lastClickTimer;
std::chrono::milliseconds Input::doubleClickDelay;

int Input::xLock;
int Input::yLock;

bool Input::showCursor;
bool Input::ignoreIfPaused;

std::unordered_set<int> Input::keysDown;

Input::Input()
{

}

Input::~Input()
{

}

void Input::Init(OSWindow* window, bool ignoreInputIfWindowsIsPaused)
{
    ignoreIfPaused = ignoreInputIfWindowsIsPaused;

	xLock = -1;
	yLock = -1;

	listenWindow = window;

	for(int i = 0; i < (int)MOUSE_BUTTON::COUNT; ++i)
	{
		mouseDownPos[i] = float2(0.0f, 0.0f);
		mouseUpPos[i] = float2(0.0f, 0.0f);
	}

    int2 queriedPosition = QueryMousePosition();

	mousePosition.x = static_cast<float>(queriedPosition.x);
	mousePosition.y = static_cast<float>(queriedPosition.y);
	oldMousePosition = mousePosition;

	showCursor = true;

	SetDoubleClickDelay(500);

	lastClickTimer.Start();
}

void Input::Update()
{
    if(ignoreIfPaused && listenWindow->IsPaused())
        return;

	oldMousePosition = mousePosition;

	int2 queriedPosition = QueryMousePosition();

	mousePosition.x = static_cast<float>(queriedPosition.x);
	mousePosition.y = static_cast<float>(queriedPosition.y);

	if(xLock != -1)
        SetCursorPosition(xLock, yLock);
}

//////////////////////////////////////////////////////////////////////////
//CALLBACK REGISTER
//////////////////////////////////////////////////////////////////////////
void Input::RegisterKeyCallback(std::function<void(const KeyState&)> callback)
{
	keyCallback = callback;
}

void Input::RegisterCharCallback(std::function<void(int)> callback)
{
	charCallback = callback;
}

void Input::RegisterScrollCallback(std::function<void(int)> callback)
{
	scrollCallback = callback;
}

void Input::RegisterMouseButtonCallback(std::function<void(const MouseButtonState&)> callback)
{
	mouseButtonCallback = callback;
}

void Input::LockCursor(int xPosition, int yPosition)
{
	xLock = xPosition;
	yLock = yPosition;

    oldMousePosition.x = xPosition;
    oldMousePosition.y = yPosition;

    mousePosition.x = xPosition;
    mousePosition.y = yPosition;
    
	if(xLock != -1)
        SetCursorPosition(xLock, yLock);
}

void Input::SetCursorPosition(int xPosition, int yPosition)
{
#ifdef _WIN32
    POINT lockPoint{ xLock, yLock };

        ClientToScreen(listenWindow->GetHWND(), &lockPoint);
        SetCursorPos(lockPoint.x, lockPoint.y);
#else
    XWarpPointer(listenWindow->GetDisplay(), None, listenWindow->GetWindow(), 0, 0, 0, 0, xPosition, yPosition);
#endif
}

void Input::HideCursor()
{
#ifdef _WIN32
	if(showCursor)
		::ShowCursor(false);
#else
    //TODO
#endif

	showCursor = false;
}

void Input::ShowCursor()
{
#ifdef _WIN32
    if(!showCursor)
		::ShowCursor(true);
#else
    //TODO
#endif

	showCursor = true;
}

//////////////////////////////////////////////////////////////////////////
//SETTERS
//////////////////////////////////////////////////////////////////////////
void Input::SetDoubleClickDelay(unsigned int ms)
{
	doubleClickDelay = std::chrono::milliseconds(ms);
}

//////////////////////////////////////////////////////////////////////////
//GETTERS
//////////////////////////////////////////////////////////////////////////
OSWindow* Input::GetListenWindow()
{
    return listenWindow;
}

bool Input::GetAsyncKeyState(KEY_CODE keyCode)
{
#ifdef _WIN32
	return ((::GetAsyncKeyState(KEY_CODEToOSKey(keyCode)) >> 15) & 1) == 1;
#else // _WIN32
    char keys[32];

    XQueryKeymap(listenWindow->GetDisplay(), keys);

    int keyToCheck = XKeysymToKeycode(listenWindow->GetDisplay(), KEY_CODEToOSKey(keyCode));
    return keys[keyToCheck / 8] & (0x1 << keyToCheck % 8) > 0;
#endif // _WIN32
}

float2 Input::GetMouseDelta()
{
    if(ignoreIfPaused && listenWindow->IsPaused())
        return float2(0.0f, 0.0f);

	float2 returnPosition;

	if(xLock == -1)
	{
		returnPosition.x = mousePosition.x - oldMousePosition.x;
		returnPosition.y = mousePosition.y - oldMousePosition.y;
	}
	else
	{
		returnPosition.x = mousePosition.x - xLock;
		returnPosition.y = mousePosition.y - yLock;
	}

	return returnPosition;
}

float2 Input::GetMousePosition()
{
	return mousePosition;
}

bool Input::MouseMoved()
{
	return mousePosition.x != oldMousePosition.x || mousePosition.y != oldMousePosition.y;
}

float2 Input::GetMouseDownPos(MOUSE_BUTTON button)
{
	return mouseDownPos[static_cast<int>(button) - 1];
}

float2 Input::GetMouseUpPos(MOUSE_BUTTON button)
{
	return mouseUpPos[static_cast<int>(button) - 1];
}

void Input::KeyEvent(
#ifdef _WIN32
                     UINT msg, WPARAM wParam, LPARAM lParam)
#else // _WIN32
                     XKeyEvent event)
#endif
{
    if(keyCallback == nullptr
            || (ignoreIfPaused && listenWindow->IsPaused()))
        return;

    KeyState keyState;
#ifdef _WIN32
	if(msg == WM_KEYDOWN)
	{
		if(((lParam >> 30) & 1) == 0)
			keyState.action = KEY_ACTION::DOWN;
		else
			keyState.action = KEY_ACTION::REPEAT;
	}
	else
		keyState.action = KEY_ACTION::UP;

    //Check modifier keys
    keyState.mods = KEY_MODIFIERS::NONE;

	if(GetKeyState(VK_LSHIFT) & 0x8000
	   || GetKeyState(VK_RSHIFT) & 0x8000)
		keyState.mods |= KEY_MODIFIERS::SHIFT;
	if(GetKeyState(VK_LCONTROL) & 0x8000
	   || GetKeyState(VK_RCONTROL) & 0x8000)
		keyState.mods |= KEY_MODIFIERS::CONTROL;
	if(GetKeyState(VK_LMENU) & 0x8000
	   || GetKeyState(VK_RMENU) & 0x8000)
		keyState.mods |= KEY_MODIFIERS::ALT;

    //Set key
	keyState.key = OSKeyToKEY_CODE(wParam);

#else // _WIN32
    //Check modifier keys
    keyState.mods = KEY_MODIFIERS::NONE;

    if((event.state & ShiftMask) > 0)
        keyState.mods |= KEY_MODIFIERS::SHIFT;
    if((event.state & ControlMask) > 0)
        keyState.mods |= KEY_MODIFIERS::CONTROL;
    if((event.state & Mod1Mask) > 0) //Alt, hopefully
        keyState.mods |= KEY_MODIFIERS::ALT;

    int combinedMods = ShiftMask | ControlMask | Mod1Mask;
    if((event.state & ~combinedMods) != 0)
        keyState.mods |= KEY_MODIFIERS::UNKNOWN;

    //Set key
    keyState.key = OSKeyToKEY_CODE(XLookupKeysym(&event, event.state & ShiftMask ? 1 : 0));

    if(event.type == KeyPress)
    {
        keysDown.insert((int)keyState.key);
        keyState.action = KEY_ACTION::DOWN;
    }
    else
    {
        bool isRepeat = false;

        // Check if event is actually a repeat event
        if(XEventsQueued(listenWindow->GetDisplay(), QueuedAfterReading))
        {
            XEvent nextEvent;
            XPeekEvent(listenWindow->GetDisplay(), &nextEvent);

            isRepeat = nextEvent.type == KeyPress
                       && nextEvent.xkey.time == event.time
                       && OSKeyToKEY_CODE(nextEvent.xkey.keycode) == keyState.key;
        }

        if(isRepeat)
        {
            // Event is key repeat, so remove the key press event
            XEvent nextEvent;
            XNextEvent(listenWindow->GetDisplay(), &nextEvent);

            keyState.action = KEY_ACTION::REPEAT;
        }
        else
        {
            keysDown.erase((int)keyState.key);
            keyState.action = KEY_ACTION::UP;
        }
    }
#endif // _WIN32

    keyCallback(keyState);
}

void Input::CharEvent(unsigned int key)
{
    //Outside of ASCII range
    if(key < 32 || key > 126
       || (ignoreIfPaused && listenWindow->IsPaused()))
        return;

    if(charCallback != nullptr)
        charCallback(key);
}

void Input::MouseButtonEvent(
#ifdef _WIN32
                             UINT msg, WPARAM wParam)
#else // _WIN32
                             XButtonEvent event)
#endif // !_WIN32
{
    if(mouseButtonCallback == nullptr
       || (ignoreIfPaused && listenWindow->IsPaused()))
		return;

	MouseButtonState mouseButtonState;

#ifdef _WIN32
    //Get modifier keys
	mouseButtonState.mods = KEY_MODIFIERS::NONE;

	if(GetKeyState(VK_LSHIFT) & 0x8000
	   || GetKeyState(VK_RSHIFT) & 0x8000)
		mouseButtonState.mods |= KEY_MODIFIERS::SHIFT;
	if(GetKeyState(VK_LCONTROL) & 0x8000
	   || GetKeyState(VK_RCONTROL) & 0x8000)
		mouseButtonState.mods |= KEY_MODIFIERS::CONTROL;
	if(GetKeyState(VK_LMENU) & 0x8000
	   || GetKeyState(VK_RMENU) & 0x8000)
		mouseButtonState.mods |= KEY_MODIFIERS::ALT;

    //Set capture if needed,
    //fill keyState,
    //and remember the last pressed button to handle double clicking
    switch(msg)
    {
    case WM_LBUTTONDOWN:
        SetCapture(listenWindow->GetHWND());
		mouseButtonState.action = KEY_ACTION::DOWN;
		mouseButtonState.button = MOUSE_BUTTON::LEFT;
        break;
    case WM_LBUTTONUP:
        SetCapture(0);
		mouseButtonState.action = KEY_ACTION::UP;
		mouseButtonState.button = MOUSE_BUTTON::LEFT;
        break;
    case WM_MBUTTONDOWN:
		mouseButtonState.action = KEY_ACTION::DOWN;
		mouseButtonState.button = MOUSE_BUTTON::MIDDLE;
        break;
    case WM_MBUTTONUP:
		mouseButtonState.action = KEY_ACTION::UP;
		mouseButtonState.button = MOUSE_BUTTON::MIDDLE;
        break;
    case WM_RBUTTONDOWN:
		mouseButtonState.action = KEY_ACTION::DOWN;
		mouseButtonState.button = MOUSE_BUTTON::RIGHT;
        break;
    case WM_RBUTTONUP:
		mouseButtonState.action = KEY_ACTION::UP;
		mouseButtonState.button = MOUSE_BUTTON::RIGHT;
        break;
    default:
        mouseButtonState.action = KEY_ACTION::UNKNOWN;
		mouseButtonState.button = MOUSE_BUTTON::UNKNOWN;
        break;
    }
#else // _WIN32
    mouseButtonState.action = event.type == ButtonPress ? KEY_ACTION::DOWN : KEY_ACTION::UP;

    mouseButtonState.mods = KEY_MODIFIERS::NONE;

    if((event.state & ShiftMask) > 0)
        mouseButtonState.mods |= KEY_MODIFIERS::SHIFT;
    if((event.state & ControlMask) > 0)
        mouseButtonState.mods |= KEY_MODIFIERS::CONTROL;
    if((event.state & Mod1Mask) > 0) //Alt, hopefully
        mouseButtonState.mods |= KEY_MODIFIERS::ALT;

    int combinedMods = ShiftMask | ControlMask | Mod1Mask;
    if((event.state & ~combinedMods) != 0)
        mouseButtonState.mods |= KEY_MODIFIERS::UNKNOWN;

    if(event.button == Button1)
        mouseButtonState.button = MOUSE_BUTTON::LEFT;
    else if(event.button == Button2)
        mouseButtonState.button = MOUSE_BUTTON::MIDDLE;
    else if(event.button == Button3)
        mouseButtonState.button = MOUSE_BUTTON::RIGHT;
    else if(event.button == Button4)
        mouseButtonState.button = MOUSE_BUTTON::MOD_1;
    else if(event.button == Button5)
        mouseButtonState.button = MOUSE_BUTTON::MOD_2;
    else
        mouseButtonState.button = MOUSE_BUTTON::UNKNOWN;
#endif // !_WIN32

	float2 mousePosition = GetMousePosition();

	//Remember mouse up/down position
	//and check whether or not it was a double click
	if(mouseButtonState.action == KEY_ACTION::DOWN
            && mouseButtonState.button != MOUSE_BUTTON::UNKNOWN)
	{
		float2 lastMouseDownPos = (mouseDownPos[static_cast<int>(lastMouseButton) - 1]);

		if(mouseButtonState.button == lastMouseButton
			&& lastClickTimer.GetTime() <= doubleClickDelay
			&& glm::length(mousePosition - lastMouseDownPos) <= maxDoubleClickDistance)
		{
            mouseButtonState.action = KEY_ACTION::REPEAT;
		}

//        DirectX::XMVECTOR xmNewMousePos = DirectX::XMLoadFloat2(&newMousePos);
//        DirectX::XMVECTOR xmLastMouseDownPos = DirectX::XMLoadFloat2(&lastMouseDownPos);
//
//        if(currentMouseButton == lastMouseButton
//           && lastClickTimer.GetTime() <= doubleClickDelay
//           && DirectX::XMVectorGetX(DirectX::XMVector2Length(DirectX::XMVectorSubtract(xmNewMousePos, xmLastMouseDownPos))) <= maxDoubleClickDistance)
//        {
//            keyState.action = KEY_ACTION::REPEAT;
//        }

		lastClickTimer.Reset();

		mouseDownPos[(int)mouseButtonState.button - 1] = mousePosition;
	}
	else
	{
		mouseUpPos[(int)mouseButtonState.button - 1] = mousePosition;
	}

	lastMouseButton = mouseButtonState.button;

	mouseButtonCallback(mouseButtonState);
}

void Input::ScrollEvent(int distance)
{
    if(scrollCallback != nullptr
       || (ignoreIfPaused && listenWindow->IsPaused()))
        scrollCallback(distance);
}

int2 Input::QueryMousePosition()
{
#ifdef _WIN32
	POINT cursorPosition;

	GetCursorPos(&cursorPosition);
	ScreenToClient(listenWindow->GetHWND(), &cursorPosition);

	return { cursorPosition.x, cursorPosition.y };
#else
    Window rootReturn;
    Window childReturn;

    int2 rootPosition;
    int2 windowPosition;

    unsigned int maskReturn;

    XQueryPointer(listenWindow->GetDisplay()
                  , listenWindow->GetWindow()
                  , &rootReturn
                  , &childReturn
                  , &rootPosition.x
                  , &rootPosition.y
                  , &windowPosition.x
                  , &windowPosition.y
                  , &maskReturn);

    return windowPosition;
#endif
}

KEY_CODE Input::OSKeyToKEY_CODE(
#ifdef _WIN32
	WPARAM wParam
#else // WIN32
        KeySym keySym
#endif // WIN32
    )
{
#ifdef _WIN32
	switch(wParam)
	{
		case VK_SPACE:
			return KEY_CODE::SPACE;
		case VK_LEFT:
			return KEY_CODE::LEFT;
		case VK_RIGHT:
			return KEY_CODE::RIGHT;
		case VK_UP:
			return KEY_CODE::UP;
		case VK_DOWN:
			return KEY_CODE::DOWN;
		case 'A':
			return KEY_CODE::A;
		case 'B':
			return KEY_CODE::B;
		case 'C':
			return KEY_CODE::C;
		case 'D':
			return KEY_CODE::D;
		case 'E':
			return KEY_CODE::E;
		case 'F':
			return KEY_CODE::F;
		case 'G':
			return KEY_CODE::G;
		case 'H':
			return KEY_CODE::H;
		case 'I':
			return KEY_CODE::I;
		case 'J':
			return KEY_CODE::J;
		case 'K':
			return KEY_CODE::K;
		case 'L':
			return KEY_CODE::L;
		case 'M':
			return KEY_CODE::M;
		case 'N':
			return KEY_CODE::N;
		case 'O':
			return KEY_CODE::O;
		case 'P':
			return KEY_CODE::P;
		case 'Q':
			return KEY_CODE::Q;
		case 'R':
			return KEY_CODE::R;
		case 'S':
			return KEY_CODE::S;
		case 'T':
			return KEY_CODE::T;
		case 'U':
			return KEY_CODE::U;
		case 'V':
			return KEY_CODE::V;
		case 'W':
			return KEY_CODE::W;
		case 'X':
			return KEY_CODE::X;
		case 'Y':
			return KEY_CODE::Y;
		case 'Z':
			return KEY_CODE::Z;
        default:
			return KEY_CODE::UNKNOWN;
	}
#else // _WIN32
	switch(keySym)
    {
        case XK_space:
            return KEY_CODE::SPACE;
        case XK_Up:
            return KEY_CODE::UP;
        case XK_Down:
            return KEY_CODE::DOWN;
        case XK_Left:
            return KEY_CODE::LEFT;
        case XK_Right:
            return KEY_CODE::RIGHT;
        case XK_0:
            return KEY_CODE::NUM_0;
        case XK_1:
            return KEY_CODE::NUM_1;
        case XK_2:
            return KEY_CODE::NUM_2;
        case XK_3:
            return KEY_CODE::NUM_3;
        case XK_4:
            return KEY_CODE::NUM_4;
        case XK_5:
            return KEY_CODE::NUM_5;
        case XK_6:
            return KEY_CODE::NUM_6;
        case XK_7:
            return KEY_CODE::NUM_7;
        case XK_8:
            return KEY_CODE::NUM_8;
        case XK_9:
            return KEY_CODE::NUM_9;
        case XK_KP_0:
            return KEY_CODE::NUM_KP_0;
        case XK_KP_1:
            return KEY_CODE::NUM_KP_1;
        case XK_KP_2:
            return KEY_CODE::NUM_KP_2;
        case XK_KP_3:
            return KEY_CODE::NUM_KP_3;
        case XK_KP_4:
            return KEY_CODE::NUM_KP_4;
        case XK_KP_5:
            return KEY_CODE::NUM_KP_5;
        case XK_KP_6:
            return KEY_CODE::NUM_KP_6;
        case XK_KP_7:
            return KEY_CODE::NUM_KP_7;
        case XK_KP_8:
            return KEY_CODE::NUM_KP_8;
        case XK_KP_9:
            return KEY_CODE::NUM_KP_9;
        case XK_A:
        case XK_a:
            return KEY_CODE::A;
        case XK_B:
        case XK_b:
            return KEY_CODE::B;
        case XK_C:
        case XK_c:
            return KEY_CODE::C;
        case XK_D:
        case XK_d:
            return KEY_CODE::D;
        case XK_E:
        case XK_e:
            return KEY_CODE::E;
        case XK_F:
        case XK_f:
            return KEY_CODE::F;
        case XK_G:
        case XK_g:
            return KEY_CODE::G;
        case XK_H:
        case XK_h:
            return KEY_CODE::H;
        case XK_I:
        case XK_i:
            return KEY_CODE::I;
        case XK_J:
        case XK_j:
            return KEY_CODE::J;
        case XK_K:
        case XK_k:
            return KEY_CODE::K;
        case XK_L:
        case XK_l:
            return KEY_CODE::L;
        case XK_M:
        case XK_m:
            return KEY_CODE::M;
        case XK_N:
        case XK_n:
            return KEY_CODE::N;
        case XK_O:
        case XK_o:
            return KEY_CODE::O;
        case XK_P:
        case XK_p:
            return KEY_CODE::P;
        case XK_Q:
        case XK_q:
            return KEY_CODE::Q;
        case XK_R:
        case XK_r:
            return KEY_CODE::R;
        case XK_S:
        case XK_s:
            return KEY_CODE::S;
        case XK_T:
        case XK_t:
            return KEY_CODE::T;
        case XK_U:
        case XK_u:
            return KEY_CODE::U;
        case XK_V:
        case XK_v:
            return KEY_CODE::V;
        case XK_W:
        case XK_w:
            return KEY_CODE::W;
        case XK_X:
        case XK_x:
            return KEY_CODE::X;
        case XK_Y:
        case XK_y:
            return KEY_CODE::Y;
        case XK_Z:
        case XK_z:
            return KEY_CODE::Z;
        case XK_Control_L:
        case XK_Control_R:
            return KEY_CODE::CONTROL;
        case XK_Alt_L:
        case XK_Alt_R:
            return KEY_CODE::ALT;
        case XK_Super_L:
        case XK_Super_R:
            return KEY_CODE::SUPER;
        case XK_F1:
            return KEY_CODE::F1;
        case XK_F2:
            return KEY_CODE::F2;
        case XK_F3:
            return KEY_CODE::F3;
        case XK_F4:
            return KEY_CODE::F4;
        case XK_F5:
            return KEY_CODE::F5;
        case XK_F6:
            return KEY_CODE::F6;
        case XK_F7:
            return KEY_CODE::F7;
        case XK_F8:
            return KEY_CODE::F8;
        case XK_F9:
            return KEY_CODE::F9;
        case XK_F10:
            return KEY_CODE::F10;
        case XK_F11:
            return KEY_CODE::F11;
        case XK_F12:
            return KEY_CODE::F12;
        default:
            return KEY_CODE::UNKNOWN;
    }
#endif // _WIN32
}

#ifdef _WIN32
WPARAM
#else // WIN32
KeySym
#endif // WIN32
Input::KEY_CODEToOSKey(KEY_CODE keyCode)
{
#ifdef _WIN32
	switch(keyCode)
	{
		case KEY_CODE::SPACE:
			return VK_SPACE;
		case KEY_CODE::LEFT:
			return VK_LEFT;
		case KEY_CODE::RIGHT:
			return VK_RIGHT;
		case KEY_CODE::UP:
			return VK_UP;
		case KEY_CODE::DOWN:
			return VK_DOWN;
		case KEY_CODE::A:
			return 'A';
		case KEY_CODE::B:
			return 'B';
		case KEY_CODE::C:
			return 'C';
		case KEY_CODE::D:
			return 'D';
		case KEY_CODE::E:
			return 'E';
		case KEY_CODE::F:
			return 'F';
		case KEY_CODE::G:
			return 'G';
		case KEY_CODE::H:
			return 'H';
		case KEY_CODE::I:
			return 'I';
		case KEY_CODE::J:
			return 'J';
		case KEY_CODE::K:
			return 'K';
		case KEY_CODE::L:
			return 'L';
		case KEY_CODE::M:
			return 'M';
		case KEY_CODE::N:
			return 'N';
		case KEY_CODE::O:
			return 'O';
		case KEY_CODE::P:
			return 'P';
		case KEY_CODE::Q:
			return 'Q';
		case KEY_CODE::R:
			return 'R';
		case KEY_CODE::S:
			return 'S';
		case KEY_CODE::T:
			return 'T';
		case KEY_CODE::U:
			return 'U';
		case KEY_CODE::V:
			return 'V';
		case KEY_CODE::W:
			return 'W';
		case KEY_CODE::X:
			return 'X';
		case KEY_CODE::Y:
			return 'Y';
		case KEY_CODE::Z:
			return 'Z';
	}
#else // WIN32
    switch(keyCode)
    {
        case KEY_CODE::SPACE:
            return XK_space;
        case KEY_CODE::UP:
            return XK_Up;
        case KEY_CODE::DOWN:
            return XK_Down;
        case KEY_CODE::LEFT:
            return XK_Left;
        case KEY_CODE::RIGHT:
            return XK_Right;
        case KEY_CODE::NUM_0:
            return XK_0;
        case KEY_CODE::NUM_1:
            return XK_1;
        case KEY_CODE::NUM_2:
            return XK_2;
        case KEY_CODE::NUM_3:
            return XK_3;
        case KEY_CODE::NUM_4:
            return XK_4;
        case KEY_CODE::NUM_5:
            return XK_5;
        case KEY_CODE::NUM_6:
            return XK_6;
        case KEY_CODE::NUM_7:
            return XK_7;
        case KEY_CODE::NUM_8:
            return XK_8;
        case KEY_CODE::NUM_9:
            return XK_9;
        case KEY_CODE::NUM_KP_0:
            return XK_KP_0;
        case KEY_CODE::NUM_KP_1:
            return XK_KP_1;
        case KEY_CODE::NUM_KP_2:
            return XK_KP_2;
        case KEY_CODE::NUM_KP_3:
            return XK_KP_3;
        case KEY_CODE::NUM_KP_4:
            return XK_KP_4;
        case KEY_CODE::NUM_KP_5:
            return XK_KP_5;
        case KEY_CODE::NUM_KP_6:
            return XK_KP_6;
        case KEY_CODE::NUM_KP_7:
            return XK_KP_7;
        case KEY_CODE::NUM_KP_8:
            return XK_KP_8;
        case KEY_CODE::NUM_KP_9:
            return XK_KP_9;
        case KEY_CODE::A:
            return XK_A;
        case KEY_CODE::B:
            return XK_B;
        case KEY_CODE::C:
            return XK_C;
        case KEY_CODE::D:
            return XK_D;
        case KEY_CODE::E:
            return XK_E;
        case KEY_CODE::F:
            return XK_F;
        case KEY_CODE::G:
            return XK_G;
        case KEY_CODE::H:
            return XK_H;
        case KEY_CODE::I:
            return XK_I;
        case KEY_CODE::J:
            return XK_J;
        case KEY_CODE::K:
            return XK_K;
        case KEY_CODE::L:
            return XK_L;
        case KEY_CODE::M:
            return XK_M;
        case KEY_CODE::N:
            return XK_N;
        case KEY_CODE::O:
            return XK_O;
        case KEY_CODE::P:
            return XK_P;
        case KEY_CODE::Q:
            return XK_Q;
        case KEY_CODE::R:
            return XK_R;
        case KEY_CODE::S:
            return XK_S;
        case KEY_CODE::T:
            return XK_T;
        case KEY_CODE::U:
            return XK_U;
        case KEY_CODE::V:
            return XK_V;
        case KEY_CODE::W:
            return XK_W;
        case KEY_CODE::X:
            return XK_X;
        case KEY_CODE::Y:
            return XK_Y;
        case KEY_CODE::Z:
            return XK_Z;
        case KEY_CODE::CONTROL:
            return XK_Control_L;
        case KEY_CODE::ALT:
            return XK_Alt_L;
        case KEY_CODE::SUPER:
            return XK_Super_L;
        case KEY_CODE::F1:
            return XK_F1;
        case KEY_CODE::F2:
            return XK_F2;
        case KEY_CODE::F3:
            return XK_F3;
        case KEY_CODE::F4:
            return XK_F4;
        case KEY_CODE::F5:
            return XK_F5;
        case KEY_CODE::F6:
            return XK_F6;
        case KEY_CODE::F7:
            return XK_F7;
        case KEY_CODE::F8:
            return XK_F8;
        case KEY_CODE::F9:
            return XK_F9;
        case KEY_CODE::F10:
            return XK_F10;
        case KEY_CODE::F11:
            return XK_F11;
        case KEY_CODE::F12:
            return XK_F12;
    }
#endif // WIN32

    throw std::runtime_error("Unknown KEY_CODE requested");
}

