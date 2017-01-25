#include "dictionaryChapter.h"

DictionaryChapter::DictionaryChapter()
	: index('?')
{

}

DictionaryChapter::DictionaryChapter(char index)
	: index(index)
{

}

DictionaryChapter::~DictionaryChapter()
{

}

void DictionaryChapter::AddEntry(const std::string& entry)
{
	DictionaryEntry newEntry(entry);

	AddEntry(newEntry);
}

void DictionaryChapter::AddEntry(const DictionaryEntry& entry)
{
	entries[entry.GetIndexCharacterSize()].emplace_back(entry);
}

const DictionaryEntry* DictionaryChapter::Find(const std::string& text) const
{
	DictionaryEntry entry = DictionaryEntry(text);

	if(entries.count(entry.GetIndexCharacterSize()) == 0)
		return nullptr;

	for(std::vector<DictionaryEntry>::const_iterator iter = entries.at(entry.GetIndexCharacterSize()).begin()
		, end = entries.at(entry.GetIndexCharacterSize()).end(); iter != end; ++iter)
	{
		if(entry == *iter)
			return &(*iter);
	}

	return nullptr;
}

std::vector<const DictionaryEntry*> DictionaryChapter::Match(const std::string& text) const
{
	std::vector<const DictionaryEntry*> matches;

	DictionaryEntry entry = DictionaryEntry(text);

	//Loop over all entries with the same amount of blockIndex characters as entry, and upwards
	for(auto lowerIter = entries.lower_bound(entry.GetIndexCharacterSize()),
			 entriesEnd = entries.end(); lowerIter != entriesEnd; ++lowerIter)
	{
		//Have to check every single entry
		for(auto iter = lowerIter->second.begin()
			, end = lowerIter->second.end(); iter != end; ++iter)
		{
			if(iter->Matches(text))
				matches.push_back(&(*iter));
		}
	}

	return matches;
}
