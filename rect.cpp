#include "rect.h"

#include <glm/gtx/norm.hpp>

Rect::Rect()
	: positionMin(0.0f, 0.0f)
	, positionMax(0.0f, 0.0f)
	, size(0.0f, 0.0f)
{
}

Rect::Rect(float x, float y, float width, float height)
	: positionMin(x, y)
	, positionMax(0.0f, 0.0f)
	, size(width, height)
{
	UpdatePositionMax();
}

Rect::Rect(float x, float y, glm::vec2 size)
	: positionMin(x, y)
	, positionMax(0.0f, 0.0f)
	, size(size)
{
	UpdatePositionMax();
}

Rect::Rect(glm::vec2 position, float width, float height)
	: positionMin(position)
	, positionMax(0.0f, 0.0f)
	, size(width, height)
{
	UpdatePositionMax();
}

Rect::Rect(glm::vec2 position, glm::vec2 size)
	: positionMin(position)
	, positionMax(0.0f, 0.0f)
	, size(size)
{
	UpdatePositionMax();
}

void Rect::Set(float x, float y, float width, float height)
{
	SetPos(x, y);
	SetSize(width, height);
}

void Rect::Set(float x, float y, glm::vec2 size)
{
	SetPos(x, y);
	SetSize(size);
}

void Rect::Set(glm::vec2 position, float width, float height)
{
	SetPos(position);
	SetSize(width, height);
}

void Rect::Set(glm::vec2 position, glm::vec2 size)
{
	SetPos(position);
	SetSize(size);
}

void Rect::SetPos(float x, float y)
{
	positionMin.x = x;
	positionMin.y = y;

	UpdatePositionMax();
}

void Rect::SetPos(glm::vec2 position)
{
	positionMin = position;

	UpdatePositionMax();
}

void Rect::SetSize(float width, float height)
{
	size.x = width;
	size.y = height;

	UpdatePositionMax();
}

void Rect::SetSize(glm::vec2 size)
{
	this->size = size;

	UpdatePositionMax();
}

bool Rect::Contains(float x, float y) const
{
	//Intentionally left as x < positionMax.x to make it zero-indexed
	return (x >= positionMin.x
		&& x < positionMax.x
		&& y >= positionMin.y
		&& y < positionMax.y);
}

bool Rect::Contains(glm::vec2 position) const
{
	//Intentionally left as x < positionMax.x to make it zero-indexed
	return (position.x >= positionMin.x
		&& position.x < positionMax.x
		&& position.y >= positionMin.y
		&& position.y < positionMax.y);
}

glm::vec2 Rect::GetMinPosition() const
{
	return positionMin;
}

glm::vec2 Rect::GetMidPosition() const
{
	glm::vec2 minPosition = GetMinPosition();

	minPosition.x += size.x * 0.5f;
	minPosition.y += size.y * 0.5f;

	return minPosition;
}

glm::vec2 Rect::GetMaxPosition() const
{
	return positionMax;
}

glm::vec2 Rect::GetSize() const
{
	return size;
}

float Rect::GetWidth() const
{
	return size.x;
}

float Rect::GetHeight() const
{
	return size.y;
}

Rect& Rect::operator+=(glm::vec2 rhs)
{
	positionMin += rhs;

	UpdatePositionMax();

	return *this;
}

Rect& Rect::operator-=(glm::vec2 rhs)
{
	positionMin -= rhs;

	UpdatePositionMax();

	return *this;
}

Rect& Rect::operator*=(glm::vec2 rhs)
{
	positionMin *= rhs;

	UpdatePositionMax();

	return *this;
}

Rect& Rect::operator/=(glm::vec2 rhs)
{
    positionMin /= rhs;

    UpdatePositionMax();

	return *this;
}

bool Rect::AlmostEqual(const Rect& rhs, float epsilon) const
{
    return glm::length2(positionMin - rhs.positionMin) <= epsilon * epsilon && glm::length2(positionMax - rhs.positionMax) <= epsilon * epsilon;
}

const Rect Rect::empty(0.0f, 0.0f, 0.0f, 0.0f);

void Rect::UpdatePositionMax()
{
    positionMax = positionMin + size;
}
