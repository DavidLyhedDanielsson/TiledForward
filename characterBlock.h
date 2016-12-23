#ifndef CharacterBlock_h__
#define CharacterBlock_h__

#include <lib/character.h>

#include <vector>

struct CharacterBlock
{
	CharacterBlock()
		: width(0)
		, length(0)
	{};
	CharacterBlock(const std::vector<const Character*>& characters, unsigned int width, unsigned int length)
		: characters(characters)
		, width(width)
		, length(length)
	{};
	~CharacterBlock() = default;

	std::vector<const Character*> characters;
	unsigned int width;
	unsigned int length;
};

inline bool operator==(const CharacterBlock& lhs, const CharacterBlock& rhs)
{
	return lhs.length == rhs.length
		&& lhs.width == rhs.width
		&& lhs.characters == rhs.characters;
}

inline bool operator!=(const CharacterBlock& lhs, const CharacterBlock& rhs)
{
	return !(lhs == rhs);
}

#endif // CharacterBlock_h__
