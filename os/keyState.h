#ifndef KeyState_h__
#define KeyState_h__

//Do not change before finding usages in Input
enum class KEY_ACTION : int
{
	UP = 0x1
	, DOWN = 0x2
	, REPEAT = 0x4
	, UNKNOWN = 0x0
};

enum class MOUSE_BUTTON : int
{
	NONE = 0x0
	, LEFT = 0x1
	, RIGHT = 0x2
	, MIDDLE = 0x4
    , MOD_1 = 0x8
    , MOD_2 = 0x10
	, UNKNOWN = 0x20
	, COUNT = 5
};

enum class KEY_MODIFIERS : int
{
	NONE = 0x0
	, SHIFT = 0x1
	, CONTROL = 0x2
	, ALT = 0x4
	, UNKNOWN = 0x8
};

enum class KEY_CODE : int
{
    SPACE = 0
	, SECTION
	, BACKSPACE
	, DELETE
	, ENTER
	, KP_ENTER
	, END
	, HOME
	, TAB
    , SHIFT_L
    , SHIFT_R
    , CONTROL_L
    , CONTROL_R
    , ALT_L
    , ALT_R
	, UP
    , DOWN
    , LEFT
    , RIGHT
    , NUM_0
    , NUM_1
    , NUM_2
    , NUM_3
    , NUM_4
    , NUM_5
    , NUM_6
    , NUM_7
    , NUM_8
    , NUM_9
    , NUM_KP_0
    , NUM_KP_1
    , NUM_KP_2
    , NUM_KP_3
    , NUM_KP_4
    , NUM_KP_5
    , NUM_KP_6
    , NUM_KP_7
    , NUM_KP_8
    , NUM_KP_9
    , A
    , B
    , C
    , D
    , E
    , F
    , G
    , H
    , I
    , J
    , K
    , L
    , M
    , N
    , O
    , P
    , Q
    , R
    , S
    , T
    , U
    , V
    , W
    , X
    , Y
    , Z
    , SUPER
    , F1
    , F2
    , F3
    , F4
    , F5
    , F6
    , F7
    , F8
    , F9
    , F10
    , F11
    , F12
    , UNKNOWN
};

class KeyState
{
public:
	KeyState()
		: key(KEY_CODE::UNKNOWN)
		, action(KEY_ACTION::UNKNOWN)
		, mods(KEY_MODIFIERS::UNKNOWN)
	{}

    KeyState(KEY_CODE key, KEY_ACTION action)
            : key(key)
              , action(action)
              , mods(KEY_MODIFIERS::UNKNOWN)
    {}

	KEY_CODE key;
	KEY_ACTION action;
	KEY_MODIFIERS mods;
};

class MouseButtonState
{
public:
    MouseButtonState()
            : button(MOUSE_BUTTON::UNKNOWN)
              , action(KEY_ACTION::UNKNOWN)
              , mods(KEY_MODIFIERS::UNKNOWN)
    {}

    MouseButtonState(MOUSE_BUTTON button, KEY_ACTION action)
            : button(button)
              , action(action)
              , mods(KEY_MODIFIERS::UNKNOWN)
    {}

    MOUSE_BUTTON button;
    KEY_ACTION action;
    KEY_MODIFIERS mods;
};

inline bool operator==(const KeyState& lhs, const KeyState& rhs)
{
	//GLFW_RELEASE = 0
	//Custom "not pressed" = -1
	//Make sure keys and mods match and that they are both pressed (or clicked)
	return lhs.key == rhs.key && lhs.action == rhs.action;
}

inline bool operator!=(const KeyState& lhs, const KeyState& rhs)
{
	return !operator==(lhs, rhs);
}

inline bool operator==(int lhs, MOUSE_BUTTON rhs)
{
	return lhs == static_cast<int>(rhs);
}

inline bool operator!=(int lhs, MOUSE_BUTTON rhs)
{
	return !(lhs == static_cast<int>(rhs));
}

inline KEY_MODIFIERS operator|(KEY_MODIFIERS lhs, KEY_MODIFIERS rhs)
{
	return static_cast<KEY_MODIFIERS>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

inline KEY_MODIFIERS& operator|=(KEY_MODIFIERS& lhs, KEY_MODIFIERS rhs)
{
	lhs = static_cast<KEY_MODIFIERS>(static_cast<int>(lhs) | static_cast<int>(rhs));

	return lhs;
}

#endif // KeyState_h__