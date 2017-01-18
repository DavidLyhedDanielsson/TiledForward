#ifndef OPENGLWINDOW_LISTSTYLE_H
#define OPENGLWINDOW_LISTSTYLE_H

#include "guiStyle.h"

#include "scrollbarStyle.h"

struct ListStyle
	: public GUIStyle
{
	ListStyle()
		: scrollDistance(1)
		, scrollbarStyle(nullptr)
		, scrollbarBackgroundStyle(nullptr)
	{}
	~ListStyle() {}

	int scrollDistance;

	std::shared_ptr<ScrollbarStyle> scrollbarStyle;
	std::shared_ptr<GUIBackgroundStyle> scrollbarBackgroundStyle;
};

#endif //OPENGLWINDOW_LISTSTYLE_H
