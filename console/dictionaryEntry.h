#ifndef OPENGLWINDOW_DICTIONARYENTRY_H
#define OPENGLWINDOW_DICTIONARYENTRY_H

#include <string>
#include <vector>

class DictionaryEntry
{
public:
	DictionaryEntry();
	DictionaryEntry(const std::string& name);
	//~DictionaryEntry() = default;

	std::string GetName() const;

	int GetIndexCharacterSize() const;
	char GetIndexCharacter() const;
	std::vector<char> GetIndexCharacters() const;

	//When matching, capital letters have to match exactly. E.g.:
	//name = SomeVariableName
	//svn => match
	//SVN => match
	//sovana => match
	//SoVaNa => match
	//SOVaNa => doesn't match
	bool Matches(const std::string& text) const;

	friend bool operator==(const DictionaryEntry&, const DictionaryEntry&);

private:
	//Name is the whole name and indexCharacters is used for searching.
	//The index characters of SomeVariableName0 is SVN0.
	//The index characters of SOMeVARiableName0 is still SVN0
	//The index characters of Some0Variable1Name2 is S0V1N2
	std::string name;
	std::vector<unsigned int> indexCharacters;
};

inline bool operator==(const DictionaryEntry& lhs, const DictionaryEntry& rhs)
{
	return lhs.name == rhs.name;
}

inline bool operator!=(const DictionaryEntry& lhs, const DictionaryEntry& rhs)
{
	return !(lhs == rhs);
}

#endif //OPENGLWINDOW_DICTIONARYENTRY_H
