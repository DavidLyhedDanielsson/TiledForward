#ifndef colorBackgroundStyle_h__
#define colorBackgroundStyle_h__

#include "guiBackgroundStyle.h"

#include <vector>
#include <memory>

#include "colorBackground.h"

struct ColorBackgroundStyle
	: public GUIBackgroundStyle
{
	std::vector<glm::vec4> colors;

	virtual std::unique_ptr<GUIBackground> CreateBackground() const override
	{
		return std::make_unique<ColorBackground>();
	}
};


#endif // colorBackgroundStyle_h__