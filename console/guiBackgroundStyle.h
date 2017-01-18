#ifndef guiBackgroundStyle_h__
#define guiBackgroundStyle_h__

#include "guiStyle.h"

class GUIBackground;

struct GUIBackgroundStyle
	: GUIStyle
{
	GUIBackgroundStyle()
	{ }
	~GUIBackgroundStyle()
	{ }

	virtual std::unique_ptr<GUIBackground> CreateBackground() const = 0;
};

#endif // guiBackgroundStyle_h__
