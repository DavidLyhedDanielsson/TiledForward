#ifndef GUIContainerTooltip_h__
#define GUIContainerTooltip_h__

#include "guiContainer.h"

class GUIContainerTooltip :
	public GUIContainer
{
public:
	GUIContainerTooltip();
	virtual ~GUIContainerTooltip();

protected:
	int tooltipDelay;
};

#endif // GUIContainerTooltip_h__
