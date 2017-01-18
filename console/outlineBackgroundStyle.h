#ifndef OPENGLWINDOW_IMAGECOLOROUTLINEBACKGROUNDSTYLE_H
#define OPENGLWINDOW_IMAGECOLOROUTLINEBACKGROUNDSTYLE_H

#include "guiBackgroundStyle.h"
#include "outlineBackground.h"

#include <vector>
#include <memory>

#include "common.h"

#include <glm/vec4.hpp>

struct OutlineBackgroundStyle
		: public GUIBackgroundStyle
{
public:
	OutlineBackgroundStyle()
			: inclusiveBorder(true)
			, outlineThickness(0.0f)
			, outlineSides(DIRECTIONS::TOP_BOTTOM_LEFT_RIGHT)
	{ };
	~OutlineBackgroundStyle()
	{ };

	virtual std::unique_ptr<GUIBackground> CreateBackground() const override
	{
		return std::make_unique<OutlineBackground>();
	}

	//If this is false, the area passed to OutlineBackground::Init()
	//will be the work area
	//Otherwise the border will be included in the area passed to OutlineBackground::Init()
	bool inclusiveBorder;

	//Has to be float because only vec4 + float is valid
	float outlineThickness;

	DIRECTIONS outlineSides;

	void AddColor(glm::vec4 backgroundColor, glm::vec4 outlineColor)
	{
		backgroundColors.push_back(backgroundColor);
		outlineColors.push_back(outlineColor);
	}

private:
	std::vector<glm::vec4> outlineColors;
	std::vector<glm::vec4> backgroundColors;

	friend class OutlineBackground;
};

#endif //OPENGLWINDOW_IMAGECOLOROUTLINEBACKGROUNDSTYLE_H
