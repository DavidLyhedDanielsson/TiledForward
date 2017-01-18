#ifndef guiContainerStyled_h__
#define guiContainerStyled_h__

#include "guiContainer.h"

#include "guiBackground.h"
#include "guiBackgroundStyle.h"

class GUIContainerStyled :
	public GUIContainer
{
public:
	GUIContainerStyled();
	virtual ~GUIContainerStyled() = default;

	GUIContainerStyled(const GUIContainerStyled&) = delete;
	GUIContainerStyled& operator=(const GUIContainerStyled&) = delete;

	GUIContainerStyled(GUIContainerStyled&&) = default;
	GUIContainerStyled& operator=(const GUIContainerStyled&&) = delete;

	virtual void Init(Rect area
					  , const std::shared_ptr<GUIStyle>& style
					  , const std::shared_ptr<GUIBackgroundStyle>& backgroundStyle
					  , GUI_ANCHOR_POINT anchorPoint = GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT);

	virtual void Init(glm::vec2 position
					  , glm::vec2 size
					  , const std::shared_ptr<GUIStyle>& style
					  , const std::shared_ptr<GUIBackgroundStyle>& backgroundStyle
					  , GUI_ANCHOR_POINT anchorPoint = GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT);

	using GUIContainer::SetPosition;
	virtual void SetPosition(float x, float y, GUI_ANCHOR_POINT anchorPoint) override;

	using GUIContainer::SetSize;
	virtual void SetSize(float x, float y, GUI_ANCHOR_POINT anchorPoint) override;

	GUIBackground* GetBackground() const;

protected:
	//std::shared_ptr<GUIStyle> style;
	std::unique_ptr<GUIBackground> background;
	//std::shared_ptr<GUIStyle> backgroundStyle;

private:
	//Don't allow these since they don't set a style
	bool Init(Rect area, GUI_ANCHOR_POINT anchorPoint = GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT) override
	{
		throw std::invalid_argument("This method should't be used. Use the Init methods that acceppt styles and a background");
	}
	bool Init(glm::vec2 position, glm::vec2 size, GUI_ANCHOR_POINT anchorPoint = GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT) override
	{
		throw std::invalid_argument("This method should't be used. Use the Init methods that acceppt styles and a background");
	}
};

#endif // guiContainerStyled_h__
