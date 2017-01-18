#ifndef SpriteInfo_h__
#define SpriteInfo_h__

#include "Rect.h"

#include <DirectXMath.h>

struct SpriteInfo
{
	SpriteInfo() {};
	SpriteInfo(Rect position, Rect clipRect, glm::vec4 color)
		: position(position)
		, clipRect(clipRect)
		, color(color)
	{};
	~SpriteInfo() {};

	Rect position;
	Rect clipRect;
	glm::vec4 color;
};

#endif // SpriteInfo_h__
