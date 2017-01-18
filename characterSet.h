#ifndef CharacterSet_h__
#define CharacterSet_h__

#include "character.h"

#include "content.h"

#include <unordered_map>
#include <vector>

class ContentManager;
class Texture;

class CharacterSet
	: public DiskContent
{
public:
	CharacterSet();
	~CharacterSet() = default;

	const static int SPACE_CHARACTER = ' '; //Change this if needed. Should correspond to your desired value for a blankspace

	//const char* GetName() const;
	unsigned int GetFontSize() const;
	Texture* GetTexture() const;

	int GetSpaceXAdvance() const;

	const Character* GetCharacter(unsigned int id) const;

	//************************************
	// Method:		GetLineHeight
	// FullName:	CharacterSet::GetLineHeight
	// Access:		public
	// Returns:		unsigned int
	// Qualifier:	const
	// Description:	Returns the line height for the currently loaded font
	//************************************
	unsigned int GetLineHeight() const;
	unsigned int GetWidthAtIndex(const char* text, unsigned int index) const;

	virtual int GetStaticVRAMUsage() const override;
	virtual int GetDynamicVRAMUsage() const override;
	virtual int GetRAMUsage() const override;

	virtual bool CreateDefaultContent(const char* filePath, ContentManager* contentManager) override;

protected:
	DiskContent* CreateInstance() const;

	CONTENT_ERROR_CODES BeginHotReload(const char* filePath, ContentManager* contentManager) override;
	bool ApplyHotReload() override;

	bool Apply(Content* content) override;

private:
	const unsigned int errorCharacterID = 0x3F; //0x3F = "?"

	CONTENT_ERROR_CODES Load(const char* filePath, ContentManager* contentManager = nullptr, ContentParameters* contentParameters = nullptr) override;
	void Unload(ContentManager* contentManager = nullptr) override;

	/*
	std::string TrimString(std::string text) const;
	std::vector<std::string> SplitAt(std::string line, char delimiter);*/

	//std::string name;
	std::unordered_map<unsigned int, Character> characters;

	unsigned int fontSize;
	unsigned int lineHeight;
	unsigned int spaceXAdvance;

	Texture* texture;

	std::pair<std::string, int> GetFontNameAndSize(const std::string& path) const;
	std::vector<uint8_t> CreateBuffer(const char* filePath, unsigned int& width, unsigned int& height);
};

#endif // CharacterSet_h__