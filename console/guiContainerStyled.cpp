#include "guiContainerStyled.h"
#include "emptyBackground.h"

GUIContainerStyled::GUIContainerStyled()
	: GUIContainer()
{

}

void GUIContainerStyled::Init(Rect area
							  , const std::shared_ptr<GUIStyle>& style
							  , const std::shared_ptr<GUIBackgroundStyle>& backgroundStyle
							  , GUI_ANCHOR_POINT anchorPoint /*= GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT*/)
{
	Init(area.GetMinPosition(), area.GetSize(), style, backgroundStyle, anchorPoint);
}

void GUIContainerStyled::Init(glm::vec2 position
							  , glm::vec2 size
							  , const std::shared_ptr<GUIStyle>& style
							  , const std::shared_ptr<GUIBackgroundStyle>& backgroundStyle
							  , GUI_ANCHOR_POINT anchorPoint /*= GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT*/)
{
	GUIContainer::Init(position, size, anchorPoint);
	
	if(backgroundStyle != nullptr)
		this->background = backgroundStyle->CreateBackground();
	else
		this->background = std::make_unique<EmptyBackground>();

	this->background->Init(backgroundStyle, &this->area);
}

void GUIContainerStyled::SetPosition(float x, float y, GUI_ANCHOR_POINT anchorPoint)
{
	GUIContainer::SetPosition(x, y, anchorPoint);

	background->AreaChanged();
}

void GUIContainerStyled::SetSize(float x, float y, GUI_ANCHOR_POINT anchorPoint)
{
	GUIContainer::SetSize(x, y, anchorPoint);

	background->AreaChanged();
}

GUIBackground* GUIContainerStyled::GetBackground() const
{
	return background.get();
}
