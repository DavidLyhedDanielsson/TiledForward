#ifndef texture_h__
#define texture_h__

#include "content.h"

#include <memory>
#include <string>

#include <GL/glew.h>

class Texture
	: public DiskContent
{
public:
	Texture();
	virtual ~Texture() = default;

	unsigned int GetWidth() const;
	unsigned int GetHeight() const;

	float GetPredivWidth() const;
	float GetPredivHeight() const;

	virtual int GetStaticVRAMUsage() const override;
	virtual int GetDynamicVRAMUsage() const override;
	virtual int GetRAMUsage() const override;

	friend bool operator==(const Texture& lhs, const Texture& rhs);
	friend bool operator!=(const Texture& lhs, const Texture& rhs);

	virtual bool CreateDefaultContent(const char* filePath, ContentManager* contentManager) override;

	bool Apply(Content* content) override;

protected:
	GLuint texture;

	unsigned int width;
	unsigned int height;
	
	float predivWidth;
	float predivHeight;

	virtual CONTENT_ERROR_CODES Load(const char* filePath, ContentManager* contentManager = nullptr, ContentParameters* contentParameters = nullptr) override;
	virtual CONTENT_ERROR_CODES LoadTemporary(const char* filePath, ContentManager* contentManager = nullptr) override;
	virtual void Unload(ContentManager* contentManager = nullptr) override;

	DiskContent* CreateInstance() const override;
};

#endif // texture_h__
