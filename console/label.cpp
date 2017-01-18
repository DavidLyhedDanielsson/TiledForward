#include "label.h"

#include "../spriteRenderer.h"

Label::Label()
	: textOffset(0.0f, 0.0f)
{
	
}

void Label::Init(Rect area
				 , const std::shared_ptr<LabelStyle>& style
				 , const std::shared_ptr<GUIBackgroundStyle>& backgroundStyle
				 , const std::string& text
				 , GUI_ANCHOR_POINT anchorPoint /*= GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT*/
				 , GUI_ANCHOR_POINT textAnchorPoint /*= GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT*/)
{
	Init(area.GetMinPosition(), area.GetSize(), style, backgroundStyle, text, anchorPoint, textAnchorPoint);
}


void Label::Init(glm::vec2 position
				 , glm::vec2 size
				 , const std::shared_ptr<LabelStyle>& style
				 , const std::shared_ptr<GUIBackgroundStyle>& backgroundStyle
				 , const std::string& text
				 , GUI_ANCHOR_POINT anchorPoint /*= GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT*/
				 , GUI_ANCHOR_POINT textAnchorPoint /*= GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT*/)
{
	this->style = style;
	this->textAnchorPoint = textAnchorPoint;

	GUIContainerStyled::Init(position, size, style, backgroundStyle, anchorPoint);

	SetText(text);
}

void Label::Draw(SpriteRenderer* spriteRenderer)
{
	background->Draw(spriteRenderer);

	spriteRenderer->DrawString(style->characterSet, text, background->GetWorkArea().GetMinPosition() + textOffset, style->textColor);
}

void Label::SetText(const std::string& text)
{
	this->text = text;

	this->area.SetSize(this->style->characterSet->GetWidthAtIndex(this->text.c_str(), this->text.length()), this->area.GetHeight());
	background->AreaChanged();

	SetTextOffset();
}

int Label::GetTextWidth() const
{
	return static_cast<int>(style->characterSet->GetWidthAtIndex(text.c_str(), static_cast<int>(text.size())));
}

void Label::SetTextOffset()
{
	int halfWidth = static_cast<int>(this->style->characterSet->GetWidthAtIndex(text.c_str(), -1) * 0.5f);

	if(textAnchorPoint & GUI_ANCHOR_POINT::LEFT && textAnchorPoint & GUI_ANCHOR_POINT::RIGHT)
		textOffset.x = static_cast<float>(static_cast<int>(this->area.GetWidth() * 0.5f) - halfWidth);
	else if(textAnchorPoint & GUI_ANCHOR_POINT::RIGHT)
		textOffset.x = static_cast<float>(static_cast<int>(this->area.GetWidth()) - halfWidth * 2);

	if(textAnchorPoint & GUI_ANCHOR_POINT::TOP && textAnchorPoint & GUI_ANCHOR_POINT::BOTTOM)
		textOffset.y = static_cast<float>(static_cast<int>(this->area.GetHeight() * 0.5f - this->style->characterSet->GetLineHeight() * 0.5f));
	else if(textAnchorPoint & GUI_ANCHOR_POINT::BOTTOM)
		textOffset.y = this->area.GetHeight() - this->style->characterSet->GetLineHeight();
}