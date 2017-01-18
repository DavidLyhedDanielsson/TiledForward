#ifndef OPENGLWINDOW_COLOROUTLINEBACKGROUND_H
#define OPENGLWINDOW_COLOROUTLINEBACKGROUND_H

#include "guiBackground.h"



struct OutlineBackgroundStyle;

class OutlineBackground
		: public GUIBackground
{
public:
	OutlineBackground();
	virtual ~OutlineBackground() = default;

	virtual void Init(const std::shared_ptr<GUIStyle>& style, const Rect* area) override;
	virtual void Draw(SpriteRenderer* spriteRenderer) override;

	virtual void AreaChanged() override;

	virtual Rect GetWorkArea() const override;
	virtual Rect GetFullArea() const override;
	virtual void UpdateOutlineRect();

	virtual std::unique_ptr<GUIBackground> Clone() override;
protected:
	//If style->inclusiveBorder this is the workArea
	//Otherwise this is the outline
	Rect rect;

	const Rect* outlineRect;
	const Rect* workAreaRect;

	std::shared_ptr<OutlineBackgroundStyle> style;
};

#endif //OPENGLWINDOW_COLOROUTLINEBACKGROUND_H
