#include <string>
#include <fstream>
#include <cstring>

#include <IL/il.h>

#include "content.h"

#if defined(_MSC_VER)
#define EXPORT __declspec(dllexport)
#elif defined(__GNUC__)
#define EXPORT __attribute__((visibility("default")))
#else
#error No DLL export semantics?
#endif

CONTENT_ERROR_CODES LoadContent(const char* filePath, unsigned char* data)
{
    ilInit();
    ilEnable(IL_ORIGIN_SET);
    ilOriginFunc(IL_ORIGIN_UPPER_LEFT);

    ILuint image;
    ilGenImages(1, &image);
    ilBindImage(image);

    ILenum error = ilGetError();
    if(error != IL_NO_ERROR)
        return CONTENT_ERROR_CODES::CREATE_FROM_MEMORY;

    ilLoadImage(filePath);

    error = ilGetError();
    if(error != IL_NO_ERROR)
        return CONTENT_ERROR_CODES::COULDNT_OPEN_FILE;

    ILubyte* ilData = ilGetData();

    int bitsPerPixel = ilGetInteger(IL_IMAGE_BPP);

    const ILint width = ilGetInteger(IL_IMAGE_WIDTH);
    const ILint height = ilGetInteger(IL_IMAGE_HEIGHT);

    if(bitsPerPixel == 4)
        memcpy(data, ilData, (size_t)(width * height * 4));
    else
    {
        for(int i = 0; i < width * height; ++i)
        {
            memcpy(data + i * 4, ilData + i * bitsPerPixel, bitsPerPixel);
            memset(data + i * 4 + bitsPerPixel, 255, (size_t)(4 - bitsPerPixel));
        }
    }

    ilDeleteImage(image);

    error = ilGetError();
    if(error != IL_NO_ERROR)
        return CONTENT_ERROR_CODES::UNKNOWN;
    else
        return CONTENT_ERROR_CODES::NONE;
}

bool Dimensions(const char* filePath, uint32_t& width, uint32_t& height)
{
    std::ifstream in(filePath);
    if(!in.is_open())
        return false;

    const uint8_t magicNumbers[]{ 137, 80, 78, 71, 13, 10, 26, 10 };

    uint8_t readNumbers[8];
    in.read(reinterpret_cast<char*>(readNumbers), 8);

    if(memcmp(magicNumbers, readNumbers, 8))
        return false;

    // Skip header info
    in.seekg(8, std::ios_base::cur);

    // Check if big or small endian
    // http://stackoverflow.com/questions/1001307/detecting-endianness-programmatically-in-a-c-program
    union
    {
        uint32_t i;
        char c[4];
    } bint = { 0x01020304 };

    bool isBigEndian = bint.c[0] == 1;

    uint32_t tempWidth;
    uint32_t tempHeight;

    in.read(reinterpret_cast<char*>(&tempWidth), 4);
    in.read(reinterpret_cast<char*>(&tempHeight), 4);

    if(!isBigEndian)
    {
        width = __bswap_32(tempWidth);
        height = __bswap_32(tempHeight);
    }
    else
    {
        width = tempWidth;
        height = tempHeight;
    }

    return true;
}

extern "C"
{
    EXPORT CONTENT_ERROR_CODES Load(const char* filePath, unsigned char* data)
    {
        return LoadContent(filePath, data);
    }

    EXPORT bool GetDimensions(const char* filePath, uint32_t& width, uint32_t& height)
    {
        return Dimensions(filePath, width, height);
    }
}