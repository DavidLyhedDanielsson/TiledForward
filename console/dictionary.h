#ifndef OPENGLWINDOW_DICTIONARY_H
#define OPENGLWINDOW_DICTIONARY_H

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

class Dictionary
{
public:
	Dictionary();
	~Dictionary() = default;

	bool AddEntry(const std::string& entry);

	bool Contains(const std::string& text) const;
	std::vector<std::string> Match(const std::string& text) const;
private:
	struct TreeNode
	{
		TreeNode(TreeNode* parent, char value, bool completesWord)
				: parent(parent)
				  , value(value)
				  , completesWord(completesWord)
		{ }

		TreeNode* parent;
		char value;
		bool completesWord;

		std::unordered_map<char, std::unique_ptr<TreeNode>> children;
	};

	std::unique_ptr<TreeNode> root;

	/**
	* Goes through every child in \p node and its children, adding any matching
	* children, completely ignoring any character in-between text[index] and text[index + 1]
	*
	* Example:
	* If \p text is "abc" and the node is "aa b123bcc" it will match
	* 
	* \param node node to explore
	* \param addTo adds all matches to this vector
	* \param text
	* \param index index in \p text to start searching from. Set to 0 to check entire string
	*/
	void GetAllMatchesIgnoreCase(const TreeNode* node, std::vector<std::string>& addTo, const std::string& text, int index) const;
	/**
	* \see GetAllMatchesIgnoreCase
	*/
	void GetAllMatches(const TreeNode* node, std::vector<std::string>& addTo, const std::string& text, int index, int lastIndex) const;
	void GetAllWords(const TreeNode* node, std::vector<std::string>& addTo) const;
	void GetAllWords(const TreeNode* node, std::vector<std::string>& addTo, std::string& text) const;

	bool AddToNode(TreeNode* node, const std::string& text, int index);
	bool NodeHasChild(const TreeNode* node, const std::string& text, int index) const;
	bool NodeHasChildIgnoreCase(const TreeNode* node, const std::string& text, int index) const;
	bool NodeHasChildren(const TreeNode* node) const;

	std::string Backtrace(const TreeNode* node) const;

	TreeNode* GetChildIgnoreCase(const TreeNode* node, const std::string& text, int index) const;

	bool IsDelimiter(char character) const;
	bool IsDelimiter(const std::string& text, int index) const;
};

#endif //OPENGLWINDOW_DICTIONARY_H
