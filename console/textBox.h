#ifndef TextBox_h__
#define TextBox_h__

#include "guiContainerStyled.h"

#include "../content/characterSet.h"
#include "../content/constructedString.h"
#include "../timer.h"

#include "textBoxStyle.h"
#include "guiManager.h"



/**
* Creates a text box where the user can enter text
*/
class TextBox :
	public GUIContainerStyled
{
friend class GUIManager;
public:
	TextBox();
	virtual ~TextBox();

	/**
	* Whether or not to allow the user to edit the currently entered text
	*/
	bool allowEdit;

	void Init(Rect area
			  , const std::shared_ptr<TextBoxStyle>& style
			  , const std::shared_ptr<GUIBackgroundStyle>& backgroundStyle
			  , GUI_ANCHOR_POINT anchorPoint = GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT);

	void Init(glm::vec2 position
					  , glm::vec2 size
					  , const std::shared_ptr<TextBoxStyle>& style
					  , const std::shared_ptr<GUIBackgroundStyle>& backgroundStyle
					  , GUI_ANCHOR_POINT anchorPoint = GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT);

	void Update(std::chrono::nanoseconds delta) override;
	void Draw(SpriteRenderer* spriteRenderer) override;

	/**@{*/
	/**
	* Inserts text at the given index
	*/
	void Insert(int index, unsigned int character);
	void Insert(unsigned int index, const std::string& newText);
	void Insert(unsigned int index, const char* newText);
	/**@}*/
	
	/**
	* Erases text from \p startIndex to \p startIndex + \p count
	*
	* \param startIndex
	* \param count
	*/
	void Erase(unsigned int startIndex, unsigned int count);

	void Activate() override;
	void Deactivate() override;

	bool OnMouseEnter() override;
	bool OnMouseDown(const MouseButtonState& keyState, const glm::vec2& mousePosition) override;
	void OnMouseUp(const MouseButtonState& keyState, const glm::vec2& mousePosition) override;
	bool OnKeyDown(const KeyState& keyState) override;
	void OnKeyUp(const KeyState& keyState) override;
	bool OnChar(unsigned int keyCode) override;

	/**
	* Sets all text in this text box
	*
	* \param text
	*/
	void SetText(const std::string& newText);

	/**@{*/
	/**
	* Basic getters and setters
	*/
	std::string GetText() const;
	std::string GetSelectedText() const;
	int GetCharacterAt(unsigned int index) const;
	int GetTextLength() const;
	int GetCursorIndex() const;

	bool GetIsEmpty() const;
	bool GetSelectionMade() const;
	bool GetIsActive() const;

	void SetCursorIndex(unsigned int newIndex);
	/**@}*/

	/**
	* Sets the separators to use when jumping with ctrl + left and ctrl + right.
	*
	* The cursor will stop at the separators
	*
	* Example:\n
	* `SetJumpSeparators(" /<>") //Will stop at spaces, /, <, and >`
	*
	* \param separators
	*/
	void SetJumpSeparators(const std::string& separators);
	/**
	* Whether or not to jump to before or after the separators when using ctrl + left and ctrl + right
	*
	* \see SetJumpSeparators
	*
	* \param jump
	*/
	void SetJumpToBeforeSeparator(bool jump);

	/**@{*/
	/**
	* Sets the character set. Needs to rebuild all text in the entire text box
	*/
	void SetCharacterSet(const std::string& fontPath, ContentManager* contentManager);
	void SetCharacterSet(const CharacterSet* characterSet);
	/**@}*/

	/**
	* Replaces the text of the character block where the cursor currently is located
	*/
	void ReplaceActiveCharacterBlockText(std::string newText);

	/**@{*/
	/**
	* Gets text from the current character block
	*/
	std::string GetActiveCharacterBlockText();
	std::string GetActiveCharacterBlockText(unsigned int& index);
	/**@}*/

	static std::shared_ptr<TextBoxStyle> GenerateDefaultStyle(ContentManager* contentManager);
	static std::unique_ptr<GUIBackground> GenerateDefaultBackground(ContentManager* contentManager);
	static std::shared_ptr<GUIBackgroundStyle> GenerateDefaultBackgroundStyle(ContentManager* contentManager);
protected:
	//ConstructedString constructedString;
	std::string text;

	std::shared_ptr<TextBoxStyle> style;

	//Used as a general flag. If cursor is drawn then input should be accepted and processed
	bool drawCursor;
	bool mouseDown;

	//////////////////////////////////////////////////////////////////////////
	//CURSOR
	//////////////////////////////////////////////////////////////////////////
	glm::vec4 cursorColor;

	int cursorIndex;

	int selectionIndex;
	int selectionStartIndex;
	int selectionEndIndex;
		
	Timer cursorBlinkTimer;
	const std::chrono::nanoseconds cursorBlinkTime = std::chrono::milliseconds(750);

	float xOffset;

	//A jump seprator controls where the cursor should jump when you press CTRL + left or CTRL + right
	//If the separator is space it will jump to the beginning or the end of a word etc.
	std::string jumpSeparators;
	bool jumpToBeforeSeparator;

	void LeftPressed(const KeyState& keyState);
	void RightPressed(const KeyState& keyState);
	void BackspacePressed(const KeyState& keyState);
	void DeletePressed(const KeyState& keyState);

	void MoveCursorLeft();
	void MoveCursorRight();
	void JumpCursorLeft();
	void JumpCursorRight();

	void ExtendSelectionToCursor();
	void BeginSelection();
	void Deselect();
	void EraseSelection();
	bool SelectionMade() const;

	void SetXOffset();
};

#endif // TextBox_h__
