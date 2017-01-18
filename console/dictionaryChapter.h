#ifndef OPENGLWINDOW_DICTIONARYCHAPTER_H
#define OPENGLWINDOW_DICTIONARYCHAPTER_H

#include "dictionaryEntry.h"

#include <map>
#include <vector>

class DictionaryChapter
{
public:
	DictionaryChapter();
	DictionaryChapter(char index);
	~DictionaryChapter();

	void AddEntry(const std::string& entry);
	void AddEntry(const DictionaryEntry& entry);

	friend bool operator==(const DictionaryChapter&, const DictionaryChapter&);
	friend bool operator<(const DictionaryChapter&, const DictionaryChapter&);

	const DictionaryEntry* Find(const std::string& text) const;
	std::vector<const DictionaryEntry*> Match(const std::string& text) const;
	//const std::vector<DictionaryEntry>& GetEntries() const; //TODO: Implement this?
private:
	char index;

	std::map<int, std::vector<DictionaryEntry>> entries;
};

inline bool operator==(const DictionaryChapter& lhs, const DictionaryChapter& rhs)
{
	return lhs.index == rhs.index;
}

inline bool operator<(const DictionaryChapter& lhs, const DictionaryChapter& rhs)
{
	return lhs.index < rhs.index;
}

#endif //OPENGLWINDOW_DICTIONARYCHAPTER_H
