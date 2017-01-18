#include "dictionary.h"

#include <cctype>
#include <algorithm>

Dictionary::Dictionary()
{
	root.reset(new TreeNode(nullptr, 0, false));
}

bool Dictionary::AddEntry(const std::string& entry)
{
	if(entry.size() == 0)
		return false;

	return AddToNode(root.get(), entry, 0);
}

bool Dictionary::Contains(const std::string& text) const
{
	int index = 0;

	TreeNode* node = root.get();

	while(NodeHasChild(node, text, index))
	{
		++index;

		if(index == text.length())
			return true;

		node = node->children[text[index]].get();
	}

	return false;
}

std::vector<std::string> Dictionary::Match(const std::string& text) const
{
	std::vector<std::string> matches;

	bool ignoreCase = true;
	for(const auto& character : text)
	{
		if(IsDelimiter(character))
		{
			ignoreCase = false;
			break;
		}
	}

	if(ignoreCase)
		GetAllMatchesIgnoreCase(root.get(), matches, text, 0);
	else
		GetAllMatches(root.get(), matches, text, 0, 0);

	return matches;
}

void Dictionary::GetAllMatchesIgnoreCase(const TreeNode* node, std::vector<std::string>& addTo, const std::string& text, int index) const
{
	if(index == text.size())
	{
		GetAllWords(node, addTo);
		return;
	}

	for(const auto& pair : node->children)
	{
		if(std::tolower(pair.second->value) == std::tolower(text[index]))
		{
			if(index + 1 == text.size())
			{
				auto iter = node->children.find(std::tolower(text[index]));
				if(iter != node->children.end())
					GetAllWords(iter->second.get(), addTo);

				iter = node->children.find(std::toupper(text[index]));
				if(iter != node->children.end())
					GetAllWords(iter->second.get(), addTo);

				continue;
			}

			GetAllMatchesIgnoreCase(pair.second.get(), addTo, text, index + 1);
		}
		else
			GetAllMatchesIgnoreCase(pair.second.get(), addTo, text, index);
	}
}

void Dictionary::GetAllMatches(const TreeNode* node, std::vector<std::string>& addTo, const std::string& text, int index, int lastIndex) const
{
	if(index == text.size())
	{
		GetAllWords(node, addTo);
		return;
	}

	for(const auto& pair : node->children)
	{
		int newIndex = index;

		if(IsDelimiter(text, newIndex))
			lastIndex = newIndex;

		if(pair.second.get()->value == text[newIndex])
			++newIndex;
		else
		{
			newIndex = lastIndex;

			if(pair.second.get()->value == text[newIndex])
			{
				if(IsDelimiter(text, newIndex))
					lastIndex = newIndex;

				++newIndex;
			}
		}

		GetAllMatches(pair.second.get(), addTo, text, newIndex, lastIndex);
	}
}

void Dictionary::GetAllWords(const TreeNode* node, std::vector<std::string>& addTo) const
{
	std::string text = Backtrace(node);

	if(node->completesWord)
		addTo.push_back(Backtrace(node));

	GetAllWords(node, addTo, text);
}

void Dictionary::GetAllWords(const TreeNode* node, std::vector<std::string>& addTo, std::string& text) const
{
	for(const auto& pair : node->children)
	{
		if(pair.second->completesWord)
			addTo.push_back(text + pair.first);

		if(node->children.size() <= 1)
		{
			//No need to re-allocate
			text += pair.first;
			GetAllWords(pair.second.get(), addTo, text);
		}
		else
		{
			std::string newText = text + pair.first;
			GetAllWords(pair.second.get(), addTo, newText);
		}
	}
}

bool Dictionary::AddToNode(TreeNode* node, const std::string& text, int index)
{
	if(index == text.size())
	{
		if(node->completesWord)
			return false;

		node->completesWord = true;

		return true;
	}

	if(!NodeHasChild(node, text, index))
	{
		TreeNode* newNode = new TreeNode(node, text[index], false); //Set to true in if-statement above
		node->children.insert(std::make_pair(text[index], std::unique_ptr<TreeNode>(newNode)));
	}

	return AddToNode(node->children[text[index]].get(), text, index + 1);
}

bool Dictionary::NodeHasChild(const TreeNode* node, const std::string& text, int index) const
{
	return node->children.count(text[index]) != 0;
}

bool Dictionary::NodeHasChildIgnoreCase(const TreeNode* node, const std::string& text, int index) const
{
	return node->children.count(std::tolower(text[index])) != 0 
		|| node->children.count(std::toupper(text[index])) != 0;
}

bool Dictionary::NodeHasChildren(const TreeNode* node) const
{
	return node->children.size() > 0;
}

std::string Dictionary::Backtrace(const TreeNode* node) const
{
	const TreeNode* currentNode = node;

	std::string returnString;

	while(currentNode != root.get())
	{
		returnString += currentNode->value;
		currentNode = currentNode->parent;
	}

	std::reverse(returnString.begin(), returnString.end());

	return returnString;
}

Dictionary::TreeNode* Dictionary::GetChildIgnoreCase(const TreeNode* node, const std::string& text, int index) const
{
	auto iter = node->children.find(std::tolower(text[index]));
	if(iter != node->children.end())
		return iter->second.get();

	iter = node->children.find(std::toupper(text[index]));
	if(iter != node->children.end())
		return iter->second.get();

	return nullptr;
}

bool Dictionary::IsDelimiter(const std::string& text, int index) const
{
	return IsDelimiter(text[index]);
}

bool Dictionary::IsDelimiter(char character) const
{
	return std::isupper(character) || character == '_';

}
