#ifndef LabelStyle_h__
#define LabelStyle_h__

#include "guiStyle.h"

#include "../characterSet.h"

struct LabelStyle
	: public GUIStyle
{
	LabelStyle()
		: characterSet(nullptr)
		, textColor(1.0f, 1.0f, 1.0f, 1.0f)
	{};
	LabelStyle(CharacterSet* characterSet, const glm::vec4& textColor)
		: characterSet(characterSet)
		, textColor(textColor)
	{ }
	~LabelStyle() = default;

	CharacterSet* characterSet;
	glm::vec4 textColor;
};

#endif // LabelStyle_h__
