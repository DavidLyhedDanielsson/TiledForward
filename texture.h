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

//protected: // TODO!
	GLuint texture;

	std::unique_ptr<unsigned char> data;

	unsigned int width;
	unsigned int height;
	
	float predivWidth; // TODO: This memory fetch is probably slower than just doing the division when needed
	float predivHeight;

	virtual CONTENT_ERROR_CODES Load(const char* filePath, ContentManager* contentManager = nullptr, ContentParameters* contentParameters = nullptr) override;
	virtual void Unload(ContentManager* contentManager = nullptr) override;

    virtual CONTENT_ERROR_CODES BeginHotReload(const char* filePath, ContentManager* contentManager = nullptr) override;
    virtual bool ApplyHotReload() override;
    virtual bool Apply(Content* other);

	CONTENT_ERROR_CODES ReadData(const char* filePath);

	DiskContent* CreateInstance() const override;
};

#endif // texture_h__
