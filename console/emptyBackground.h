#ifndef OPENGLWINDOW_EMPTYBACKGROUND_H
#define OPENGLWINDOW_EMPTYBACKGROUND_H

#include "guiBackground.h"



class EmptyBackground
	: public GUIBackground
{
public:
	EmptyBackground();
	~EmptyBackground();

	void Init(const std::shared_ptr<GUIStyle>& style, const Rect* area) override;
	void Draw(SpriteRenderer* spriteRenderer) override;

	virtual std::unique_ptr<GUIBackground> Clone() override;
};

#endif //OPENGLWINDOW_EMPTYBACKGROUND_H
