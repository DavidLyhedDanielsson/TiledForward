#include "constructedString.h"

#include <algorithm>

ConstructedString::ConstructedString()
	: width(0)
	, text("")
	, length(0)
{ }

ConstructedString::ConstructedString(const CharacterSet* characterSet, const std::string& text)
{
	Construct(characterSet, text);
}

ConstructedString::ConstructedString(const CharacterSet* characterSet, const std::string& text, const std::string& separators, const bool keepSeparators)
{
	Construct(characterSet, text, separators, keepSeparators);
}

ConstructedString::ConstructedString(std::vector<CharacterBlock> characterBlocks, unsigned int width, std::string text, unsigned int length) 
	: characterBlocks(characterBlocks)
	, width(width)
	, text(text)
	, length(length)
{ }

void ConstructedString::Insert(const CharacterSet* characterSet, unsigned int index, const std::string& string)
{
	text.insert(index, string.c_str());

	Construct(characterSet, text);
}

void ConstructedString::Insert(const CharacterSet* characterSet, int index, unsigned int character)
{
	if(character > static_cast<unsigned int>(std::numeric_limits<char>::max()))
		character = '?';

	text.insert(static_cast<unsigned long>(index), 1, static_cast<char>(character));

	Construct(characterSet, text);
}

void ConstructedString::Erase(const CharacterSet* characterSet, unsigned int startIndex, unsigned int count)
{
	text.erase(startIndex, count);

	Construct(characterSet, text);
}

void ConstructedString::Replace(const CharacterSet* characterSet, unsigned int begin, unsigned int end, const std::string& newText)
{
	text.replace(begin, end, newText);

	Construct(characterSet, text);
}

unsigned int ConstructedString::GetWidthAtMaxWidth(unsigned int maxWidth) const
{
	unsigned int currentWidth = 0;

	for(const CharacterBlock& block : characterBlocks)
	{
		if(currentWidth + block.width > maxWidth)
		{
			short characterXAdvance;

			for(const Character* character : block.characters)
			{
				characterXAdvance = character->xAdvance;

				currentWidth += characterXAdvance;

				if(currentWidth >= maxWidth)
					return currentWidth - characterXAdvance;
			}
		}
		else
			currentWidth += block.width;
	}

	return currentWidth;
}

unsigned int ConstructedString::GetWidthAtIndex(const CharacterSet* characterSet, unsigned int index) const
{
	if(index >= text.size())
		return width;

	unsigned int currentWidth = 0;
	unsigned int currentIndex = 0;

	for(const CharacterBlock& block : characterBlocks)
	{
		unsigned int blockSize = static_cast<unsigned int>(block.characters.size());

		//Is the index inside the current block?
		if(currentIndex + blockSize > index)
		{
			for(unsigned int i = 0; i < index - currentIndex; i++)
				currentWidth += block.characters[i]->xAdvance;

			return currentWidth;
		}
		else if(currentIndex + blockSize == index)
		{
			return currentWidth + block.width;
		}
		else
		{
			currentWidth += block.width;
			currentIndex += blockSize;
		}

		currentWidth += characterSet->GetSpaceXAdvance();
		currentIndex++;

		if(currentIndex == index)
			return currentWidth;
	}

	return currentWidth;
}

unsigned int ConstructedString::GetWidthAtIndex(const CharacterSet* characterSet, const std::string& text, unsigned int index) const
{
	if(index == 0)
		return 0;

	unsigned int currentWidth = 0;
	unsigned int currentIndex = 0;

	for(char stringCharacter : text)
	{
		const Character* character = characterSet->GetCharacter(stringCharacter);

		currentWidth += character->xAdvance;
		++currentIndex;

		if(currentIndex == index)
			return currentWidth;
	}

	return currentWidth;
}

unsigned int ConstructedString::GetIndexAtWidth(const CharacterSet* characterSet, unsigned int width, float roundingValue /*= 0.6f*/) const
{
	if(width >= static_cast<int>(this->width))
		return length;

	unsigned int currentWidth = 0;
	unsigned int currentIndex = 0;

	//Used for index rounding
	int lastCharXAdvance = 0;

	for(const CharacterBlock& characterBlock : characterBlocks)
	{
		if(currentWidth + static_cast<unsigned int>(characterBlock.width) < width)
		{
			currentWidth += characterBlock.width;
			currentIndex += static_cast<unsigned int>(characterBlock.characters.size());
		}
		else
		{
			for(const Character* character : characterBlock.characters)
			{
				if(currentWidth + character->xAdvance <= width)
				{
					currentWidth += character->xAdvance;
					currentIndex++;
				}
				else
				{
					lastCharXAdvance = character->xAdvance;
					break;
				}
			}

			break;
		}

		//Space
		if(currentWidth + static_cast<int>(characterSet->GetSpaceXAdvance()) <= width)
		{
			currentWidth += characterSet->GetSpaceXAdvance();
			currentIndex++;
		}
		else
		{
			lastCharXAdvance = characterSet->GetSpaceXAdvance();
			break;
		}
	}

	currentIndex += (currentWidth + lastCharXAdvance - width) < (lastCharXAdvance * roundingValue);

	return currentIndex;
}

unsigned int ConstructedString::GetRowsAtWidth(const CharacterSet* characterSet, unsigned int width) const
{
	unsigned int rows = 1;
	unsigned int currentWidth = 0;

	for(const CharacterBlock& block : characterBlocks)
	{
		if(currentWidth + block.width <= width)
		{
			currentWidth += block.width + characterSet->GetSpaceXAdvance();
		}
		else
		{
			if(block.width > width)
			{
				//Block needs to be split into several lines

				for(const Character* character : block.characters)
				{
					if(currentWidth + character->xAdvance < width)
						currentWidth += character->xAdvance;
					else
					{
						currentWidth = static_cast<unsigned int>(character->xAdvance);
						rows++;
					}
				}

				currentWidth += characterSet->GetSpaceXAdvance();
			}
			else
			{
				currentWidth = block.width + characterSet->GetSpaceXAdvance();
				rows++;
			}
		}
	}

	return rows;
}

void ConstructedString::Construct(const CharacterSet* characterSet, const std::string& text)
{
	this->characterBlocks.clear();;
	this->width = 0;
	this->length = 0;
	this->text = text;

	if(text == "")
	{
		std::vector<const Character*> newVector;
		this->characterBlocks.emplace_back(newVector, 0, 0);
		this->width = 0;
		this->length = 0;

		return;
	}

	//Split each word (and the trailing blankspace) into a character block
	//If the string is something like "abc            def" split it into "abc" and " ... def" (ignoring a single space after abc)
	//A space is always presumed to be after a CharacterBlock when drawing
	bool splitAtSpace = false;

	unsigned int totalWidth = 0;

	CharacterBlock characterBlock;
	const Character* character;

	unsigned int length = 0;

	auto iter = text.begin();
	while(iter != text.end())
	{
		character = characterSet->GetCharacter(static_cast<unsigned int>(*iter));
		++iter;

		if(character->id == characterSet->SPACE_CHARACTER)
		{
			if(splitAtSpace)
			{
				splitAtSpace = false;

				characterBlocks.emplace_back(std::move(characterBlock));

				characterBlock.characters.clear();
				characterBlock.width = 0;
				characterBlock.length = 0;
				totalWidth += characterSet->GetSpaceXAdvance(); //A space is always presumed to be after a character block so include it in the total width
				length++;
			}
			else
			{
				characterBlock.width += character->xAdvance;
				totalWidth += character->xAdvance;
				characterBlock.characters.emplace_back(character);
				characterBlock.length++;
				length++;
			}
		}
		else
		{
			splitAtSpace = true;

			characterBlock.width += character->xAdvance;
			totalWidth += character->xAdvance;
			characterBlock.characters.emplace_back(character);
			characterBlock.length++;
			length++;
		}
	}

	this->characterBlocks.emplace_back(std::move(characterBlock));

	this->width = totalWidth;
	this->length = length;
}

void ConstructedString::Construct(const CharacterSet* characterSet, const std::string& text, const std::string& separators, const bool keepSeparators)
{
	this->characterBlocks.clear();;
	this->width = 0;
	this->text.clear();
	this->length = 0;

	if(text == "")
	{
		std::vector<const Character*> newVector;
		this->characterBlocks.emplace_back(newVector, 0, 0);
		this->text = "";
		this->width = 0;
		this->length = 0;

		return;
	}

	unsigned int totalWidth = 0;

	CharacterBlock characterBlock;
	const Character* character;

	unsigned int length = 0;

	auto iter = text.begin();
	while(iter != text.end())
	{
		character = characterSet->GetCharacter(static_cast<unsigned int>(*iter));
		++iter;

		if(separators.find(static_cast<unsigned char>(character->id)) != separators.npos)
		{
			//Emplace current block
			characterBlocks.emplace_back(std::move(characterBlock));

			//Emplace block with separator if they should be kept
			if(keepSeparators)
			{
				characterBlock.width = static_cast<unsigned int>(character->xAdvance);
				totalWidth = static_cast<unsigned int>(character->xAdvance);
				characterBlock.characters.clear();
				characterBlock.characters.emplace_back(character);
				characterBlock.length = 1;
				length++;

				characterBlocks.emplace_back(std::move(characterBlock));
			}

			characterBlock.characters.clear();
			characterBlock.width = 0;
			characterBlock.length = 0;
			totalWidth += character->xAdvance;
			length++;
		}
		else
		{
			characterBlock.width += character->xAdvance;
			totalWidth += character->xAdvance;
			characterBlock.characters.emplace_back(character);
			characterBlock.length++;
			length++;
		}
	}

	this->characterBlocks.emplace_back(characterBlock);

	this->width = totalWidth;
	this->text = text;
	this->length = length;
}
