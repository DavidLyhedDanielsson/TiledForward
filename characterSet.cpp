#include "characterSet.h"

#include "logger.h"
#include "contentManager.h"

#include <freetype2/ft2build.h>
#include FT_FREETYPE_H
#include <map>
#include <set>
#include <cmath>
#include <assert.h>

#include "content.h"
#include "memoryTexture.h"
#include "textureCreationParameters.h"

#if defined(_DEBUG) | defined(DEBUG_CHECKS)
#include <assert.h>
#endif

CharacterSet::CharacterSet()
	: fontSize(0)
	, lineHeight(-1)
	, spaceXAdvance(-1)
	, texture(nullptr)
{
}

const Character* CharacterSet::GetCharacter(unsigned int id) const
{
	auto iter = characters.find(id);

	if(iter != characters.end())
		return &iter->second;
	else
	{
		iter = characters.find(errorCharacterID);

		if(iter != characters.end())
			return &iter->second;
		else
		{
			Logger::LogLine(LOG_TYPE::WARNING, "CharacterSet::errorCharacterID set to a non-existing character (make sure CharacterSet is loaded)");
			return &characters.begin()->second;
		}
	}
}

/*
const char* CharacterSet::GetName() const
{
	return name.c_str();
}
*/

unsigned int CharacterSet::GetFontSize() const
{
	return fontSize;
}

Texture* CharacterSet::GetTexture() const
{
	return texture;
}

CONTENT_ERROR_CODES CharacterSet::Load(const char* filePath, ContentManager* contentManager /*= nullptr*/, ContentParameters* contentParameters /*= nullptr*/)
{
	unsigned int width;
	unsigned int height;

	std::vector<uint8_t> buffer = CreateBuffer(filePath, width, height);

	//Needed to fix some crash...?
	std::string uniqueID(filePath);
	uniqueID += "Texture";

	TextureCreationParameters textureParameters(uniqueID.c_str(), width, height, GLEnums::INTERNAL_FORMAT::RGBA8, GLEnums::FORMAT::RGBA, GLEnums::TYPE::UNSIGNED_BYTE, &buffer[0]);
	this->texture = contentManager->Load<MemoryTexture>("", &textureParameters);

	return CONTENT_ERROR_CODES::NONE;
}

void CharacterSet::Unload(ContentManager* contentManager)
{
	contentManager->Unload(texture);
}

std::pair<std::string, int> CharacterSet::GetFontNameAndSize(const std::string& path) const
{
	//Extract extension, default to ttf
	std::string extension;
	std::string pathWithoutExtension;

	auto dotIndex = path.find_last_of('.');
	if(path.size() >= 4 && path.compare(path.size() - 4, 4, ".ttf") == 0)
	{
		extension = ".ttf";
		pathWithoutExtension = path.substr(0, path.size() - 4);
	}
	else
	{
		if(dotIndex == path.npos)
		{
			extension = ".ttf";
			pathWithoutExtension = path;
		}
		else
		{
			extension = path.substr(dotIndex);
			pathWithoutExtension = path.substr(0, dotIndex);
		}
	}


	//Extract size, default to 12
	//If font file contains a number, _ is used to separate name from size
	//e.g SomeFont2_24 => SomeFont2 with size 24
	auto nonNumber = pathWithoutExtension.find_last_not_of("0123456789");

	int size = -1;
	if(pathWithoutExtension.find_first_of("0123456789") == pathWithoutExtension.npos
	   || nonNumber == pathWithoutExtension.npos)
		size = 12;
	else
	{
		if(dotIndex == pathWithoutExtension.npos)
			size = std::stoi(pathWithoutExtension.substr(nonNumber + 1));
		else
			size = std::stoi(pathWithoutExtension.substr(nonNumber + 1, dotIndex - nonNumber + 1));
	}

	//Extract name
	std::string fontName;

	if(nonNumber == pathWithoutExtension.npos)
		fontName = pathWithoutExtension;
	else
	{
		if(pathWithoutExtension[nonNumber] == '_')
			fontName = pathWithoutExtension.substr(0, nonNumber);
		else
			fontName = pathWithoutExtension.substr(0, nonNumber + 1);
	}

	return std::make_pair(fontName + extension, size);
}

std::vector<uint8_t> CharacterSet::CreateBuffer(const char* filePath, unsigned int& width, unsigned int& height)
{
	std::vector<uint8_t> returnVector;

	//////////////////////////////////////////////////
	//Init FreeType
	//////////////////////////////////////////////////
	static FT_Library ftLibrary;

	auto error = FT_Init_FreeType(&ftLibrary);
	if(error)
	{
		Logger::LogLine(LOG_TYPE::WARNING, "Couldn't initialize FreeType. No fonts will be available");
		return returnVector;
	}

	FT_Face face;

	std::string filePathString(filePath);

	auto nameAndSize = GetFontNameAndSize(filePathString);

	error = FT_New_Face(ftLibrary, nameAndSize.first.c_str(), 0, &face);

	if(error == FT_Err_Unknown_File_Format)
	{
		Logger::LogLine(LOG_TYPE::WARNING, "Font file at " + filePathString + ", (" + nameAndSize.first + ")" + " is unsupported");
		FT_Done_FreeType(ftLibrary);
		return returnVector;
	}
	else if(error)
	{
		Logger::LogLine(LOG_TYPE::WARNING, "Font file at " + filePathString + ", (" + nameAndSize.first + ")" + " couldn't be created");
		FT_Done_FreeType(ftLibrary);
		return returnVector;
	}

	fontSize = nameAndSize.second;
	error = FT_Set_Pixel_Sizes(face, 0, fontSize);
	if(error)
	{
		Logger::LogLine(LOG_TYPE::WARNING, "Couldn't set pixel sizes");
		FT_Done_Face(face);
		FT_Done_FreeType(ftLibrary);
		return returnVector;
	}

	FT_GlyphSlot slot = face->glyph;

	//////////////////////////////////////////////////
	//Get all characters
	//////////////////////////////////////////////////
	//Sort characters by width when inserting into set
	struct CharacterComparison
	{
		bool operator()(const Character& lhs, const Character& rhs) const
		{
			return lhs.width > rhs.width;
		}
	};

	std::multiset<Character, CharacterComparison> tempCharacters;
    for(int i = 32; i <= 126; ++i)
	{
		error = FT_Load_Char(face, i, FT_LOAD_DEFAULT);
		if(error)
		{
			Logger::LogLine(LOG_TYPE::WARNING, "Couldn't load character with ID ", i, " when trying to load font ", nameAndSize.first, " with size ", fontSize, ". Character will be ignored");
			continue;
		}

#ifndef NDEBUG
		if(slot->metrics.width >> 6 > std::numeric_limits<unsigned char>().max())
			Logger::LogLine(LOG_TYPE::WARNING, "Character width is greater than max value of an unsigned char");
		if(slot->metrics.height >> 6 > std::numeric_limits<unsigned char>().max())
			Logger::LogLine(LOG_TYPE::WARNING, "Character height is greater than max value of an unsigned char");
		if(slot->metrics.horiBearingX >> 6 > std::numeric_limits<char>().max())
			Logger::LogLine(LOG_TYPE::WARNING, "Character x bearing is greater than max value of a char");
		if(slot->metrics.horiBearingY >> 6 < std::numeric_limits<char>().min())
			Logger::LogLine(LOG_TYPE::WARNING, "Character y bearing is less than min value of a char");
		if(slot->advance.x >> 6 > std::numeric_limits<unsigned char>().max())
			Logger::LogLine(LOG_TYPE::WARNING, "Character x-advance is greater than min value of a char");
#endif // NDEBUG

		//if(i != 32) //Space
			tempCharacters.insert(Character(i
											, -1
											, -1
											, static_cast<unsigned char>(slot->metrics.width >> 6)
											, static_cast<unsigned char>(slot->metrics.height >> 6)
											, static_cast<char>(slot->metrics.horiBearingX >> 6)
											, static_cast<char>(slot->metrics.horiBearingY >> 6)
											, static_cast<unsigned short>(slot->advance.x >> 6)));
	}

	lineHeight = face->size->metrics.height >> 6;

	//////////////////////////////////////////////////
	//Pack characters
	//////////////////////////////////////////////////
	//Simple biggest to smallest (width-wise) packing
	//From top left to bottom right
	int posX = 0;
	int posY = 0;

	//Very simple and probably not well-fitting since not all characters are the same size
	int dimension = static_cast<int>(std::ceil(std::sqrt(tempCharacters.size())));

	width = dimension * tempCharacters.begin()->width;

	//Round to next power of two to create a texture of that size
	--width;
	width |= width >> 1;
	width |= width >> 2;
	width |= width >> 4;
	width |= width >> 8;
	width |= width >> 16;
	++width;

#ifndef NDEBUG
        std::set<int> removedCharacters;
#endif // NDEBUG

	for(auto iter = tempCharacters.begin(), end = tempCharacters.end(); iter != end; ++iter)
	{
		if(static_cast<uint32_t>(posX + iter->width) > width)
		{
			//Fit smaller characters until it's not possible anymore.
			//Characters are sorted after width, so lower_bound will
			//always return an element after iter
			auto tempIter = tempCharacters.lower_bound(Character(0, 0, 0, width - posX, 0, 0, 0, 0));
			while(tempIter != tempCharacters.end())
			{
				Character character = *tempIter;
				character.x = posX;
				character.y = posY;

#ifndef NDEBUG
				assert(removedCharacters.find(character.id) == removedCharacters.end());
				assert(static_cast<uint32_t>(character.x + character.width) <= width);
				removedCharacters.insert(character.id);
#endif // NDEBUG

				characters.insert(std::make_pair(character.id, character));

				tempCharacters.erase(tempIter);

				posX += character.width;
				tempIter = tempCharacters.lower_bound(Character(0, 0, 0, width - posX, 0, 0, 0, 0));
			}

			posX = 0;
			posY += lineHeight;
		}

		Character character = *iter;
		character.x = posX;
		character.y = posY;

#ifndef NDEBUG
		assert(removedCharacters.find(character.id) == removedCharacters.end());
		assert(static_cast<uint32_t>(character.x + character.width) <= width);
#endif // NDEBUG

		posX += character.width;

		characters.insert(std::make_pair(character.id, character));
	}

	height = posY + lineHeight;

	--height;
	height |= height >> 1;
	height |= height >> 2;
	height |= height >> 4;
	height |= height >> 8;
	height |= height >> 16;
	++height;

	returnVector.resize(width * height * 4, 0);
	for(const auto& character : characters)
	{
		error = FT_Load_Char(face, character.first, FT_LOAD_RENDER);
		if(error)
		{
			Logger::LogLine(LOG_TYPE::WARNING, "Couldn't render character with ID ", character.first, " when trying to load font ", nameAndSize.first, " with size ", fontSize, ". Character will be ignored");
			characters.erase(character.first);
			continue;
		}

        //TODO: Implement swizzling
		for(int y = static_cast<int>(character.second.height) - 1; y >= 0; --y)
        {
            for(int x = 0, endx = static_cast<int>(character.second.width); x < endx; ++x)
            {
                returnVector[(character.second.y + y) * width * 4 + (character.second.x + x) * 4] = 255;
                returnVector[(character.second.y + y) * width * 4 + (character.second.x + x) * 4 + 1] = 255;
                returnVector[(character.second.y + y) * width * 4 + (character.second.x + x) * 4 + 2] = 255;
                returnVector[(character.second.y + y) * width * 4 + (character.second.x + x) * 4 + 3] = slot->bitmap.buffer[y * slot->bitmap.width + x];
            }
        }
	}

	FT_Done_Face(face);
	FT_Done_FreeType(ftLibrary);

    spaceXAdvance = GetCharacter(32)->xAdvance;

	return returnVector;
}

int CharacterSet::GetStaticVRAMUsage() const
{
	return 0;
}

int CharacterSet::GetDynamicVRAMUsage() const
{
	return 0;
}

int CharacterSet::GetRAMUsage() const
{
	return 0;
}

bool CharacterSet::CreateDefaultContent(const char* filePath, ContentManager* contentManager)
{
	std::string filePathString(filePath);

	auto nameAndSize = GetFontNameAndSize(filePathString);

#ifdef _WIN32
	std::string newFont = "C:\\Windows\\Fonts\\Consola" + std::to_string(nameAndSize.second) + ".ttf";
	if(Load(device, newFont.c_str(), contentManager, nullptr) == CONTENT_ERROR_CODES::NONE)
		return true;

	newFont = "C:\\Windows\\Fonts\\Calibri" + std::to_string(nameAndSize.second) + ".ttf";
	if(Load(device, newFont.c_str(), contentManager, nullptr) == CONTENT_ERROR_CODES::NONE)
		return true;
#else
    std::string newFont = "/usr/share/fonts/truetype/ubuntu-font-family/Ubuntu-R" + std::to_string(nameAndSize.second) + ".ttf";
    if(Load(newFont.c_str(), contentManager, nullptr) == CONTENT_ERROR_CODES::NONE)
        return true;
#endif

	return false;
}

DiskContent* CharacterSet::CreateInstance() const
{
	return new CharacterSet;
}

bool CharacterSet::Apply(Content* content)
{
	CharacterSet* other = dynamic_cast<CharacterSet*>(content);
	if(other == nullptr)
		return false;

	this->characters = std::move(other->characters);
	this->fontSize = other->fontSize;
	this->lineHeight = other->lineHeight;
	this->spaceXAdvance = other->spaceXAdvance;
    this->texture = other->texture;

	return true;
}

unsigned int CharacterSet::GetLineHeight() const
{
	return lineHeight;
}

unsigned int CharacterSet::GetWidthAtIndex(const char* text, unsigned int index) const
{
	if(index == 0)
		return 0;

	unsigned int currentWidth = 0;
	unsigned int currentIndex = 0;

	std::string textString(text);
	for(char stringCharacter : textString)
	{
		const Character* character = GetCharacter(stringCharacter);

		currentWidth += character->xAdvance;
		++currentIndex;

		if(currentIndex == index)
			return currentWidth;
	}

	return currentWidth;
}

int CharacterSet::GetSpaceXAdvance() const
{
	return spaceXAdvance;
}

CONTENT_ERROR_CODES CharacterSet::BeginHotReload(const char* filePath, ContentManager* contentManager)
{
    return CONTENT_ERROR_CODES::NONE; //Not supported
}

bool CharacterSet::ApplyHotReload()
{
    return false; //Not supported
}
