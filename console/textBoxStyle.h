#ifndef TextBoxStyle_h__
#define TextBoxStyle_h__

#include "guiStyle.h"

struct TextBoxStyle 
	: public GUIStyle
{
	TextBoxStyle()
			: textHighlightColor(173.0f / 255.0f, 214.0f / 255.0f, 1.0f, 1.0f)
			, textColorNormal(0.0f, 0.0f, 0.0f, 1.0f)
			, textColorSelected(0.0f, 0.0f, 0.0f, 1.0f)
			, cursorSize(0.0f, 0.0f)
			, cursorColorNormal(0.0f, 0.0f, 0.0f, 1.0f)
			, cursorColorBlink(0.0f, 0.0f, 0.0f, 0.0f)
			, cursorOffset(0.0f, 0.0f)
			, characterSet(nullptr)
	{};
	virtual ~TextBoxStyle() {};

	glm::vec4 textColorNormal; //Color text
	glm::vec4 textColorSelected; //Color text when text is selected (highlit)
	glm::vec4 textHighlightColor; //Color behind selected text

	glm::vec2 cursorSize;
	glm::vec4 cursorColorNormal;
	glm::vec4 cursorColorBlink;
	glm::vec2 cursorOffset;

	const CharacterSet* characterSet;
};

#endif // TextBoxStyle_h__
