#ifndef TEXTURECREATIONPARAMETERS_H__
#define TEXTURECREATIONPARAMETERS_H__

#include "content.h"
#include "glEnums.h"

#include <GL/glew.h>

struct TextureCreationParameters
	: public ContentCreationParameters
{	
	TextureCreationParameters(const char* uniqueID)
		: ContentCreationParameters(uniqueID)
	{ }

    TextureCreationParameters(const char* uniqueID
                              , GLuint width
                              , GLuint height
                              , GLEnums::INTERNAL_FORMAT internalFormat
                              , GLEnums::FORMAT format
                              , GLEnums::TYPE type
                              , void* data)
            : ContentCreationParameters(uniqueID)
              , width(width)
              , height(height)
              , internalFormat(internalFormat)
              , format(format)
              , type(type)
              , data(data)
    {}

    ~TextureCreationParameters() = default;

private:
	GLuint width;
	GLuint height;

	GLEnums::INTERNAL_FORMAT internalFormat;
	GLEnums::FORMAT format;
	GLEnums::TYPE type;

	void* data;

	friend class MemoryTexture;
};

#endif // TEXTURECREATIONPARAMETERS_H__
