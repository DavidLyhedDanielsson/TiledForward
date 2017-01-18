#ifndef OPENGLWINDOW_BACKGROUND_H
#define OPENGLWINDOW_BACKGROUND_H

#include "guiStyle.h"

#include "../rect.h"

#include <memory>
#include "../logger.h"



class SpriteRenderer;

class GUIBackground
{
public:
	GUIBackground();
	virtual ~GUIBackground() = default;

	virtual void Init(const std::shared_ptr<GUIStyle>& style, const Rect* area) = 0;
	virtual void Draw(SpriteRenderer* spriteRenderer) = 0;

	virtual void AreaChanged();
	virtual void ChangePreset(int preset);

	//virtual Rect SetWorkArea(const Rect& newArea);
	virtual Rect GetWorkArea() const;
	virtual Rect GetFullArea() const;

	virtual std::unique_ptr<GUIBackground> Clone() = 0;
protected:
	const Rect* area;

	int preset;

	template<typename T>
	std::shared_ptr<T> CastStyleTo(const std::shared_ptr<GUIStyle>& style)
	{
		auto ptr = std::static_pointer_cast<T>(style);

		if(ptr == nullptr)
			Logger::LogLine(LOG_TYPE::FATAL, "Couldn't cast style to " + std::string(typeid(T).name()));

		return ptr;
	}
};

#endif //OPENGLWINDOW_BACKGROUND_H
