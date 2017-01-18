#ifndef ButtonStyle_h__
#define ButtonStyle_h__

#include "guiStyle.h"

class CharacterSet;

struct ButtonStyle
	: public GUIStyle
{
	ButtonStyle()
		: characterSet(nullptr)
		, textOffset(0.0f, 0.0f)
		, toggle(false)
	{};

	~ButtonStyle()
	{};

	enum class BUTTON_STATES { NORMAL = 0, HOVER, CLICK, SIZE };

	glm::vec4 textColors[static_cast<int>(BUTTON_STATES::SIZE)];

	CharacterSet* characterSet;

	glm::vec2 textOffset;

	bool toggle;
};

#endif // ButtonStyle_h__
