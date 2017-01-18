#ifndef horizontalSlider_h__
#define horizontalSlider_h__

#include "guiContainerStyled.h"

#include "sliderStyle.h"
#include "guiManager.h"
#include "guiBackground.h"

#include <memory>

class ContentManager;

class Slider :
	public GUIContainerStyled
{
friend class GUIManager;
public:
	Slider();
	~Slider() = default;

	void Init(Rect area
			  , const std::shared_ptr<SliderStyle>& style
			  , const std::shared_ptr<GUIBackgroundStyle>& backgroundStyle
			  , float minValue
			  , float maxValue
			  , float startValue
			  , float snapValue
			  , bool snapPosition
			  , std::function<void(float)> valueChangedCallback = nullptr
			  , GUI_ANCHOR_POINT anchorPoint = GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT);

	void Init(glm::vec2 position
			  , glm::vec2 size
			  , const std::shared_ptr<SliderStyle>& style
			  , const std::shared_ptr<GUIBackgroundStyle>& backgroundStyle
			  , float minValue
			  , float maxValue
			  , float startValue
			  , float snapValue
			  , bool snapPosition
			  , std::function<void(float)> valueChangedCallback = nullptr
			  , GUI_ANCHOR_POINT anchorPoint = GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT);

	void Update(std::chrono::nanoseconds delta) override;
	void Draw(SpriteRenderer* spriteRenderer) override;

	void SetPosition(float x, float y, GUI_ANCHOR_POINT anchorPoint) override;
	void SetSize(float x, float y, GUI_ANCHOR_POINT anchorPoint) override;

	bool OnMouseDown(const MouseButtonState& keyState, const glm::vec2& mousePosition) override;
	void OnMouseUp(const MouseButtonState& keyState, const glm::vec2& mousePosition) override;

	float GetValue();
protected:
	std::function<void(float)> valueChangedCallback;

	bool mouseDown;

	int numberOfValues;
	/**
	* Not actually this slider's current value, see GetValue.
	*
	* It represents the index of the current value, so if
	* the slider is at its minimum position it will be 0,
	* and if it's at its maximum position it will be numberOfValues
	*/
	int currentValue;

	/**
	* If true, then the slider's thumb's position will be snapped
	* to the values, which means its value changes every time
	* it moves.
	*
	* Otherwise it will smootly scroll from side to side
	*/
	bool snapPosition;

	/**
	* Rounds the slider's value to a multiple of the given value
	*/
	float snapValue;

	float minValue;
	float maxValue;
	float startValue;

	Rect thumb;

	int grabOffset;

	std::shared_ptr<SliderStyle> style;

	void ClampThumbPosition();

	void UpdateThumbPosition();
	void UpdateThumbSize();
	void UpdateCurrentValue();

	bool OnMouseEnter() override;
	void OnMouseExit() override;
};

#endif // Scrollbar_h__
