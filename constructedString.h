#ifndef ConstructedString_h__
#define ConstructedString_h__

#include <vector>
#include <string>

#include "characterBlock.h"

#include "characterSet.h"

struct ConstructedString
{
	ConstructedString();
	ConstructedString(const CharacterSet* characterSet, const std::string& text);
	ConstructedString(const CharacterSet* characterSet, const std::string& text, const std::string& separators, const bool keepSeparators);
	ConstructedString(std::vector<CharacterBlock> characterBlocks, unsigned int width, std::string text, unsigned int length);
	~ConstructedString() = default;

	void Insert(const CharacterSet* characterSet, unsigned int index, const std::string& string);
	void Insert(const CharacterSet* characterSet, int index, unsigned int character);

	void Erase(const CharacterSet* characterSet, unsigned int startIndex, unsigned int count);

	//Replaces a character block
	void Replace(const CharacterSet* characterSet, unsigned int begin, unsigned int end, const std::string& newText);

	unsigned int GetWidthAtMaxWidth(unsigned int maxWidth) const;

	//TODO: rewrite to use local members instead of characterSet
	unsigned int GetWidthAtIndex(const CharacterSet* characterSet, unsigned int index) const;
	unsigned int GetWidthAtIndex(const CharacterSet* characterSet, const std::string& text, unsigned int index) const;
	unsigned int GetIndexAtWidth(const CharacterSet* characterSet, unsigned int width, float roundingValue = 0.6f) const;
	unsigned int GetRowsAtWidth(const CharacterSet* characterSet, unsigned int width) const;

	std::vector<CharacterBlock> characterBlocks;
	unsigned int width;
	std::string text;
	unsigned int length;

private:
	void Construct(const CharacterSet* characterSet, const std::string& text);
	void Construct(const CharacterSet* characterSet, const std::string& text, const std::string& separators, const bool keepSeparators);
};

inline bool operator==(const ConstructedString& lhs, const std::string& rhs)
{
	return lhs.text == rhs;
}

inline bool operator!=(const ConstructedString& lhs, const std::string& rhs)
{
	return !(lhs == rhs);
}

inline bool operator==(const std::string& lhs, const ConstructedString& rhs)
{
	return rhs == lhs;
}

inline bool operator!=(const std::string& lhs, const ConstructedString& rhs)
{
	return !(rhs == lhs);
}

#endif // ConstructedString_h__
