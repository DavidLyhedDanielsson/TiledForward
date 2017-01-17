#ifndef _WIN32
#include <dlfcn.h>
#endif

#include "texture.h"

#include "textureCreationParameters.h"

#include "contentManager.h"
#include "memoryTexture.h"


Texture::Texture()
	: width(-1)
	, height(-1)
	, predivWidth(-1.0f)
	, predivHeight(-1.0f)
{

}

unsigned int Texture::GetWidth() const
{
	return width;
}

unsigned int Texture::GetHeight() const
{
	return height;
}

float Texture::GetPredivWidth() const
{
	return predivWidth;
}

float Texture::GetPredivHeight() const
{
	return predivHeight;
}

void Texture::Unload(ContentManager* contentManager /*= nullptr*/)
{
    glDeleteTextures(1, &texture);

    texture = 0;
}

DiskContent* Texture::CreateInstance() const
{
	return new Texture;
}

bool Texture::Apply(Content* content)
{
	Texture* other = dynamic_cast<Texture*>(content);
	if(other == nullptr)
		return false;

    texture = other->texture;

    width = other->width;
    height = other->height;

    predivWidth = other->predivWidth;
    predivHeight = other->predivHeight;

	return true;
}

CONTENT_ERROR_CODES Texture::Load(const char* filePath, ContentManager* contentManager /*= nullptr*/, ContentParameters* contentParameters /*= nullptr*/)
{
    // TODO: Error handling
    ReadData(filePath);
    ApplyHotReload();

	return CONTENT_ERROR_CODES::NONE;
}

CONTENT_ERROR_CODES Texture::BeginHotReload(const char* filePath, ContentManager* contentManager /*= nullptr*/)
{
    return ReadData(filePath);
}

CONTENT_ERROR_CODES Texture::ReadData(const char* filePath)
{
    const std::string filePathString(filePath);
    const std::string fileExtension = filePathString.substr(filePathString.find_last_of('.') + 1);

    // TODO: Windows config
#ifndef NDEBUG
    const std::string libraryName = "lib" + fileExtension + "_d.so";
#else
    const std::string libraryName = "lib" + fileExtension + ".so";
#endif // NDEBUG

#ifdef _WIN32
    typedef CONTENT_ERROR_CODES (WINAPI* LoadFunction)(ID3D11Device*, const char*, ID3D11Texture2D**, ID3D11ShaderResourceView**);

#define CONTENT_TEXTURE_DEBUG_DLL

#ifdef CONTENT_TEXTURE_DEBUG_DLL
	HINSTANCE dllHandle = LoadLibrary((fileExtension + "_d.dll").c_str());
#else
	HINSTANCE dllHandle = LoadLibrary((fileExtension + ".dll").c_str());
#endif

	if(dllHandle == nullptr)
		return CONTENT_ERROR_CODES::COULDNT_OPEN_FILE;

	LoadFunction load = (LoadFunction)GetProcAddress(dllHandle, "Load");
	if(load == nullptr)
		return CONTENT_ERROR_CODES::CONTENT_PARAMETER_CAST;

	ID3D11Texture2D* textureDumb = nullptr;
	ID3D11ShaderResourceView* srvDumb = nullptr;

	CONTENT_ERROR_CODES errorCode = load(device, filePath, &textureDumb, &srvDumb);
	if(errorCode != CONTENT_ERROR_CODES::NONE)
	{
		if(textureDumb != nullptr)
			textureDumb->Release();
		if(srvDumb != nullptr)
			srvDumb->Release();

		FreeLibrary(dllHandle);

		return errorCode;
	}

	texture.reset(textureDumb);
	shaderResourceView.reset(srvDumb);

	D3D11_TEXTURE2D_DESC texDesc;
	this->texture->GetDesc(&texDesc);

	FreeLibrary(dllHandle);

    // TODO: Update width and height!
#else
#ifndef NDEBUG
    void* library = dlopen(libraryName.c_str(), RTLD_NOW);
#else
    void* library = dlopen(libraryName.c_str(), RTLD_LAZY);
#endif // NDEBUG

    typedef bool (*Dimensions)(const char*, uint32_t&, uint32_t&);
    typedef CONTENT_ERROR_CODES (*Load)(const char*, unsigned char*);

    Dimensions dimensions = (Dimensions)dlsym(library, "GetDimensions");

    if(dimensions != nullptr)
    {
        if(!dimensions(filePath, width, height))
            return CONTENT_ERROR_CODES::COULDNT_OPEN_FILE;
    }

    data.reset(new unsigned char[width * height * 4]);

    Load load = (Load)dlsym(library, "Load");
    if(load != nullptr)
        load(filePath, data.get()); // TODO: Return value
	else
		return CONTENT_ERROR_CODES::COULDNT_OPEN_FILE;

    dlclose(library);
#endif

    return CONTENT_ERROR_CODES::NONE;
}

bool Texture::ApplyHotReload()
{
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.get());

    glBindTexture(GL_TEXTURE_2D, 0);

    predivWidth = 1.0f / width;
    predivHeight = 1.0f / height;

    data.reset(nullptr); // TODO: I don't like this

	return true;
}

int Texture::GetStaticVRAMUsage() const
{
//	D3D11_TEXTURE2D_DESC desc;
//	texture->GetDesc(&desc);
//
//	if(desc.Usage != D3D11_USAGE_DEFAULT
//	   && desc.Usage != D3D11_USAGE_IMMUTABLE)
//		return 0;
//
//	float mipMapMult = desc.MipLevels == 0 ? 4.0f / 3.0f : 1.0f;
//	int sizeMult = static_cast<int>(std::roundf(GetSizeMult(desc.Format) * (1.0f / 8.0f) * mipMapMult));
//
//	return desc.Width * desc.Height * sizeMult;

	// TODO
	return -1;
}

int Texture::GetDynamicVRAMUsage() const
{
//	D3D11_TEXTURE2D_DESC desc;
//	texture->GetDesc(&desc);
//
//	if(desc.Usage != D3D11_USAGE_DYNAMIC)
//		return 0;
//
//	float mipMapMult = desc.MipLevels == 0 ? 4.0f / 3.0f : 1.0f;
//	int sizeMult = static_cast<int>(std::roundf(GetSizeMult(desc.Format) * (1.0f / 8.0f) * mipMapMult));
//
//	return desc.Width * desc.Height * sizeMult;

	// TODO
	return -1;
}

int Texture::GetRAMUsage() const
{
	return sizeof(Texture);
}

bool Texture::CreateDefaultContent(const char* filePath, ContentManager* contentManager)
{
    // TODO: Test
	struct Color
	{
		uint8_t r;
		uint8_t g;
		uint8_t b;
		uint8_t a;
	};

	Color hotCheckers[4][4];

	for(int x = 0, endx = 4; x < endx; ++x)
	{
		for(int y = 0, endy = 4; y < endy; ++y)
		{
			hotCheckers[y][x] = (x / 2 + y / 2) % 2 == 0 ? Color{ 255, 105, 180, 255 } : Color{ 0, 0, 0, 255 };
		}
	}

	TextureCreationParameters creationParameters("Texture::CreateDefaultContent", 4, 4, GLEnums::INTERNAL_FORMAT::RGBA, GLEnums::FORMAT::RGBA, GLEnums::TYPE::UNSIGNED_BYTE, &hotCheckers);
	MemoryTexture* memoryTexture = contentManager->Load<MemoryTexture>("", &creationParameters);

	if(memoryTexture == nullptr)
		return false;

    texture = memoryTexture->texture;

	width = memoryTexture->width;
	height = memoryTexture->height;

	predivWidth = memoryTexture->predivWidth;
	predivHeight = memoryTexture->predivHeight;

    // TODO: Is this needed?
	contentManager->IncreaseRefCount(memoryTexture);
	contentManager->Unload(memoryTexture);

	return true;
}

inline bool operator==(const Texture& lhs, const Texture& rhs)
{
	return lhs.texture == rhs.texture;
}

inline bool operator!=(const Texture& lhs, const Texture& rhs)
{
	return !(lhs == rhs);
}

GLuint Texture::GetTexture() const
{
    return texture;
}
