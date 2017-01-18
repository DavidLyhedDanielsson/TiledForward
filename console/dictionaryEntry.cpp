#include "dictionaryEntry.h"

#include <cctype>

DictionaryEntry::DictionaryEntry()
		: name("?")
{
}

DictionaryEntry::DictionaryEntry(const std::string& name)
	: name(name)
{
	indexCharacters.emplace_back(0);

	bool lastWasUpper = true;
	bool lastWasPunct = true;
	for(int i = 0, end = static_cast<int>(this->name.size()); i < end; i++)
	{
		if((std::isupper(this->name[i]) || std::isdigit(this->name[i])) && !lastWasUpper)
		{
			indexCharacters.emplace_back(i);
			lastWasUpper = true;
		}
		else if(std::ispunct(this->name[i]) && !lastWasPunct)
		{
			indexCharacters.emplace_back(i);
			lastWasPunct = true;
		}
		else
		{
			lastWasUpper = false;
			lastWasPunct = false;
		}
	}
}

std::string DictionaryEntry::GetName() const
{
	return name;
}

char DictionaryEntry::GetIndexCharacter() const
{
	return name[0];
}

std::vector<char> DictionaryEntry::GetIndexCharacters() const
{
	std::vector<char> characters;
	characters.reserve(indexCharacters.size());

	for(unsigned int index : indexCharacters)
		characters.push_back(name[index]);

	return characters;
}

int DictionaryEntry::GetIndexCharacterSize() const
{
	return static_cast<int>(indexCharacters.size());
}

bool DictionaryEntry::Matches(const std::string& text) const
{
	int matchIndex = 0; //Number of matching indicies
	int prevMatchIndex = 0;

	for(int i = 0, end = static_cast<int>(indexCharacters.size()); i < end; i++)
	{
		std::string substr = name.substr(indexCharacters[i], (i + 1 < end) ? indexCharacters[i + 1] - indexCharacters[i] : name.npos);

		int substrIndex = 0;

		char textChar = std::isupper(text[matchIndex]) ? substr[0] : static_cast<char>(std::tolower(substr[0]));
		if(textChar != text[matchIndex])
		{
			if(matchIndex > 0)
			{
				int currMatchIndex = matchIndex;

				bool found = false;

				for(; matchIndex < currMatchIndex; matchIndex++)
				{
					textChar = std::isupper(text[matchIndex]) ? substr[0] : static_cast<char>(std::tolower(substr[0]));

					if(textChar == text[matchIndex])
					{
						found = true;
						break;
					}
				}

				if(!found)
				{
					matchIndex = currMatchIndex; //if(matchIndex == text.size()) ???
					continue;
				}
			}
			else
				continue;
		}

		for(int j = 0, substrEnd = static_cast<int>(substr.size()); substrIndex < substrEnd; j++, substrIndex++)
		{
			//If any of the "substrChar = ..." or "textChar = ..." throw (possibly OutOfRange?),
			//then make sure this code is compiled for C++11.
			//If it is compiled as C++11 code and still crashes then uncomment the following if-statement:
			//if(matchIndex + substrIndex >= text.size())
			//	break;

			char substrChar = std::isupper(text[matchIndex + substrIndex]) ? substr[j] : static_cast<char>(std::tolower(substr[j]));
			textChar = std::isupper(text[matchIndex + substrIndex]) ? text[matchIndex + substrIndex] : static_cast<char>(std::tolower(text[matchIndex + substrIndex]));

			if(substrChar != textChar)
				break;
		}

		//If the previous match was better, use it
		if(prevMatchIndex > matchIndex)
			matchIndex = prevMatchIndex;
		else
		{
			prevMatchIndex = matchIndex;
			matchIndex += substrIndex;
		}

		if(matchIndex == text.size())
			return true;
	}

	return matchIndex == text.size();
}
