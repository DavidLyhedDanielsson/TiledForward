#ifndef OPENGLWINDOW_COMMON_H
#define OPENGLWINDOW_COMMON_H

/*Use logical or "|" to combine X and Y properties
// X properties are:
//  LEFT
//  RIGHT
//  CENTER_X
//
// Y properties are:
//  TOP
//  BOTTOM
//  CENTER_Y
//
// Get X properties by typing <enum> & X_BITS
// Get Y properties by typing <enum> & Y_BITS
*/
enum class DIRECTIONS
{
	NONE = 0x0,
	TOP = 0x1,
	BOTTOM = 0x2,
	CENTER_Y = 0x4,
	LEFT = 0x8,
	RIGHT = 0x10,
	CENTER_X = 0x20,
	Y_BITS = LEFT - 1,
	X_BITS = ~Y_BITS,
	TOP_BOTTOM_LEFT_RIGHT = TOP | BOTTOM | LEFT | RIGHT
};

inline DIRECTIONS operator|(DIRECTIONS lhs, DIRECTIONS rhs)
{
	return static_cast<DIRECTIONS>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

inline DIRECTIONS operator&(DIRECTIONS lhs, DIRECTIONS rhs)
{
	return static_cast<DIRECTIONS>(static_cast<int>(lhs) & static_cast<int>(rhs));
}

#endif //OPENGLWINDOW_COMMON_H
