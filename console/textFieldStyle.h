#ifndef TextFieldStyle_h__
#define TextFieldStyle_h__

#include "guiStyle.h"

#include "scrollbarStyle.h"
#include "../content/characterSet.h"

struct TextFieldStyle
	: public GUIStyle
{
	TextFieldStyle()
			: textHighlightColor(38.0f / 255.0f, 79.0f / 255.0f, 128 / 255.0f, 0.2f)
			, textColorNormal(0.0f, 0.0f, 0.0f, 1.0f)
			, cursorSize(2.0f, 32.0f)
			, cursorColorNormal(0.0f, 0.0f, 0.0f, 1.0f)
			, cursorColorBlink(0.0f, 0.0f, 0.0f, 0.0f)
			, cursorOffset(0.0f, 0.0f)
			, scrollBarPadding(5)
			, characterSet(nullptr)
			, scrollBarStyle(nullptr)
			, scrollbarBackground(nullptr)
			, scrollbarBackgroundStyle(nullptr)
	{};
	virtual ~TextFieldStyle() {};

	glm::vec4 textHighlightColor;
	glm::vec4 textColorNormal;

	glm::vec2 cursorSize;
	glm::vec4 cursorColorNormal;
	glm::vec4 cursorColorBlink;
	glm::vec2 cursorOffset;

	//Padding between the text and the scrollbar
	int scrollBarPadding;

	const CharacterSet* characterSet;

	std::shared_ptr<ScrollbarStyle> scrollBarStyle;
	std::unique_ptr<GUIBackground> scrollbarBackground;
	std::shared_ptr<GUIBackgroundStyle> scrollbarBackgroundStyle;
};

#endif // TextFieldStyle_h__
