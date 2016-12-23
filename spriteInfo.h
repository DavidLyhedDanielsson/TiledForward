#ifndef SpriteInfo_h__
#define SpriteInfo_h__

#include "Rect.h"

#include <DirectXMath.h>

struct SpriteInfo
{
	SpriteInfo() {};
	SpriteInfo(Rect position, Rect clipRect, DirectX::XMFLOAT4 color)
		: position(position)
		, clipRect(clipRect)
		, color(color)
	{};
	~SpriteInfo() {};

	Rect position;
	Rect clipRect;
	DirectX::XMFLOAT4 color;
};

#endif // SpriteInfo_h__
