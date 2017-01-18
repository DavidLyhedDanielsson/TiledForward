#ifndef Label_h__
#define Label_h__

#include "guiContainerStyled.h"

#include "labelStyle.h"



class Label
	: public GUIContainerStyled
{
public:
	Label();
	~Label() = default;

	void Init(Rect area
			  , const std::shared_ptr<LabelStyle>& style
			  , const std::shared_ptr<GUIBackgroundStyle>& backgroundStyle
			  , const std::string& text
			  , GUI_ANCHOR_POINT anchorPoint = GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT
			  , GUI_ANCHOR_POINT textAnchorPoint = GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT);
	void Init(glm::vec2 position
			  , glm::vec2 size
			  , const std::shared_ptr<LabelStyle>& style
			  , const std::shared_ptr<GUIBackgroundStyle>& backgroundStyle
			  , const std::string& text
			  , GUI_ANCHOR_POINT anchorPoint = GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT
			  , GUI_ANCHOR_POINT textAnchorPoint = GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT);

	void Draw(SpriteRenderer* spriteRenderer) override;

	void SetText(const std::string& text);

	int GetTextWidth() const;

protected:
	std::shared_ptr<LabelStyle> style;

	std::string text;

	glm::vec2 textOffset;
	GUI_ANCHOR_POINT textAnchorPoint;

	void SetTextOffset();
};

#endif // Label_h__
