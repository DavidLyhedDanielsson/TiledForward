#include <assert.h>
#include "memoryTexture.h"
#include "textureCreationParameters.h"

MemoryTexture::MemoryTexture()
{ }

MemoryTexture::~MemoryTexture()
{ }

CONTENT_ERROR_CODES MemoryTexture::Load(const char* filePath, ContentManager* contentManager /*= nullptr*/, ContentParameters* contentParameters /*= nullptr*/)
{
	auto parameters = dynamic_cast<TextureCreationParameters*>(contentParameters);
	if(parameters == nullptr)
		return CONTENT_ERROR_CODES::CONTENT_PARAMETER_CAST;

	glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // TODO: Create mipmaps?
	glTexImage2D(GL_TEXTURE_2D
				 , 0
				 , (GLint)parameters->internalFormat
				 , parameters->width
				 , parameters->height
				 , 0
				 , (GLenum)parameters->format
				 , (GLenum)parameters->type
				 , parameters->data);

    glBindTexture(GL_TEXTURE_2D, 0);

    width = parameters->width;
	height = parameters->height;

	predivWidth = 1.0f / width;
	predivHeight = 1.0f / height;

	return CONTENT_ERROR_CODES::NONE;
}

bool MemoryTexture::CreateDefaultContent(const char* filePath, ContentManager* contentManager)
{
	throw std::logic_error("The method or operation is not implemented.");
}
