#ifndef memoryTexture_h__
#define memoryTexture_h__

#include "texture.h"

class MemoryTexture :
	public Texture
{
public:
	MemoryTexture();
	~MemoryTexture();

private:
	CONTENT_ERROR_CODES Load(const char* filePath, ContentManager* contentManager = nullptr, ContentParameters* contentParameters = nullptr) override;

	virtual bool CreateDefaultContent(const char* filePath, ContentManager* contentManager) override;
};

#endif // memoryTexture_h__
