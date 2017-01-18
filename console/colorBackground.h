#ifndef colorBackground_h__
#define colorBackground_h__

#include "guiBackground.h"

struct ColorBackgroundStyle;

class ColorBackground
	: public GUIBackground
{
public:
	ColorBackground();
	~ColorBackground();

	virtual void Init(const std::shared_ptr<GUIStyle>& style, const Rect* area) override;
	virtual void Draw(SpriteRenderer* spriteRenderer) override;

	virtual std::unique_ptr<GUIBackground> Clone() override;

private:
	std::shared_ptr<ColorBackgroundStyle> style;
};

#endif // colorBackground_h__