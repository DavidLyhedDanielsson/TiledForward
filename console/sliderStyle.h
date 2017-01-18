#ifndef sliderStyle_h__
#define sliderStyle_h__

#include "guiStyle.h"

struct SliderStyle :
	public GUIStyle
{
	enum class DIRECTIONS
	{
		HORIZONTAL, VERTICAL
	};

	SliderStyle()
		: direction(DIRECTIONS::VERTICAL)
		, thumbColor(0.75f, 0.75f, 0.75f, 1.0f)
	{ };

	~SliderStyle() {};
	
	DIRECTIONS direction;

	glm::vec4 thumbColor;
};
#endif // sliderStyle_h__
