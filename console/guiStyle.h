#ifndef GUIStyle_h__
#define GUIStyle_h__

#include <memory>

class GUIBackground;

/**
* Base class for class describing what GUI elements look like
*/
struct GUIStyle
{
public:
	GUIStyle() {};
	virtual ~GUIStyle() = default;
};

#endif // GUIStyle_h__
