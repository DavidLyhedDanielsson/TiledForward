#ifndef Rect_h__
#define Rect_h__

#include <glm/vec2.hpp>

class Rect
{
public:
	Rect();
	Rect(float x, float y, float width, float height);
	Rect(float x, float y, glm::vec2 size);
	Rect(glm::vec2 position, float width, float height);
	Rect(glm::vec2 position, glm::vec2 size);
	~Rect() = default;

	void Set(float x, float y, float width, float height);
	void Set(float x, float y, glm::vec2 size);
	void Set(glm::vec2 position, float width, float height);
	void Set(glm::vec2 position, glm::vec2 size);

	void SetPos(float x, float y);
	void SetPos(glm::vec2 position);

	void SetSize(float width, float height);
	void SetSize(glm::vec2 size);

	bool Contains(float x, float y) const;
	bool Contains(glm::vec2 position) const;

	glm::vec2 GetMinPosition() const;
	glm::vec2 GetMidPosition() const;
	glm::vec2 GetMaxPosition() const;
	glm::vec2 GetSize() const;

	float GetWidth() const;
	float GetHeight() const;

	Rect& operator+=(glm::vec2 rhs);
	Rect& operator-=(glm::vec2 rhs);
	Rect& operator*=(glm::vec2 rhs);
	Rect& operator/=(glm::vec2 rhs);

	bool AlmostEqual(const Rect& rhs, float epsilon) const;

	friend bool operator==(const Rect& lhs, const Rect& rhs);
	friend bool operator!=(const Rect& lhs, const Rect& rhs);

	const static Rect empty;

private:
	glm::vec2 positionMin;
	glm::vec2 positionMax;
	glm::vec2 size;

	void UpdatePositionMax();
};

inline Rect operator+(Rect lhs, glm::vec2 rhs)
{
	lhs += rhs;
	return lhs;
}

inline Rect operator-(Rect lhs, glm::vec2 rhs)
{
	lhs -= rhs;
	return lhs;
}

inline Rect operator*(Rect lhs, glm::vec2 rhs)
{
	lhs *= rhs;
	return lhs;
}

inline Rect operator/(Rect lhs, glm::vec2 rhs)
{
	lhs /= rhs;
	return lhs;
}

inline bool operator==(const Rect& lhs, const Rect& rhs)
{
	return lhs.positionMin == rhs.positionMin && lhs.positionMax == rhs.positionMax;
}

inline bool operator!=(const Rect& lhs, const Rect& rhs)
{
	return !(lhs == rhs);
}

#endif // Rect_h__
