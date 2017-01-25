#ifndef TextField_h__
#define TextField_h__

#include "guiContainerStyled.h"

#include "../timer.h"

#include "../rect.h"
#include "textFieldStyle.h"
#include "scrollbar.h"
#include "guiManager.h"

/**
* Creates a text field. A text field is a multi-line TextBox
*
* \see GUIContainer for more info
*/
class TextField :
	public GUIContainerStyled
{
	friend class GUIManager;
public:
	TextField();
	~TextField() = default;

	void Init(Rect area
			  , const std::shared_ptr<TextFieldStyle>& style
			  , const std::shared_ptr<GUIBackgroundStyle>& backgroundStyle
			  , GUI_ANCHOR_POINT anchorPoint = GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT);

	void Init(glm::vec2 position
			  , glm::vec2 size
			  , const std::shared_ptr<TextFieldStyle>& style
			  , const std::shared_ptr<GUIBackgroundStyle>& backgroundStyle
			  , GUI_ANCHOR_POINT anchorPoint = GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT);

	void Update(std::chrono::nanoseconds delta) override;

	void Draw(SpriteRenderer* spriteRenderer) override;

	/**
	* Adds text on a NEW line. Can contain newlines
	*
	* Example:\n
	* `AddText("One line\nTwo lines\nThreeLines")`
	* 
	* \param text
	*/
	void AddText(std::string text);
	/**
	* Adds text on the last line. Can contain newlines.
	* 
	* \param text
	*/
	void AppendText(const std::string& text);
	/**
	* Inserts text at the given line at the given index
	*
	* \param text
	* \param lineIndex
	* \param index
	*/
	void InsertText(const std::string& text, int lineIndex, int index);

	/**
	* Gets all text
	*
	* \returns a vector containing each paragraph
	*/
	std::vector<std::string> GetText() const;
	
	/**
	* Gets all lines from \p begin to \p begin + \p count
	*
	* \param begin
	* \param count
	*
	* \returns a vector containing each paragraph
	*/
	std::vector<std::string> GetLines(int begin, int count) const;
	/**
	* Gets the given line
	*
	* \param stringIndex
	*
	* \returns the line
	*/
	std::string	GetLine(int stringIndex) const;
	/**
	* Gets the currently selected text
	*
	* \returns currently selected text
	*/
	std::string	GetSelectedText() const;
	/**
	* Erases all lines from \p begin to \p begin + \p count
	*
	* \param begin
	* \param count
	*/
	void EraseLines(int begin, int count);
	/**
	* Gets the number of lines
	*
	* \returns number of lines
	*/
	int	GetLineCount() const;

	/**
	* Sets all text. Can contain newlines
	*
	* \param text
	*/
	void SetText(const std::string& text);

	/**
	* Clears all text
	*/
	void Clear();

	void Activate() override;
	void Deactivate() override;

	bool GetIsActive() const;

	/**
	* Whether or not to allow editing this textField
	*/
	bool allowEdit;

	void SetPosition(const glm::vec2& newPosition) override;
	void SetPosition(float x, float y) override;
	void SetSize(const glm::vec2& newSize) override;
	void SetSize(float x, float y) override;


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
	void SetJumpToBeforeSeparator(bool jump);

	bool OnKeyDown(const KeyState& keyState) override;
	bool OnMouseEnter() override;
	void OnMouseExit() override;
	bool OnMouseDown(const MouseButtonState& keyState, const glm::vec2& mousePosition) override;
	void OnMouseUp(const MouseButtonState& keyState, const glm::vec2& mousePosition) override;
	bool OnChar(unsigned keyCode) override;
	bool OnScroll(int distance) override;

	static std::shared_ptr<TextFieldStyle> GenerateDefaultStyle(ContentManager* contentManager);
	static std::unique_ptr<GUIBackground> GenerateDefaultBackground(ContentManager* contentManager);
	static std::shared_ptr<GUIBackgroundStyle> GenerateDefaultBackgroundStyle(ContentManager* contentManager);

protected:
	//<text, whether or not the line at this blockIndex and the line at the next blockIndex is "together">
	//I also realized now that I could've as well used std::pair. TODO
	std::vector<std::tuple<std::string, bool>> lines;

	glm::vec4 textColor;

	std::shared_ptr<TextFieldStyle> style;

	bool drawCursor;
	bool mouseDown;
	//If focus == true then process input
	bool focus;

	Scrollbar scrollbar;
		
	int yOffset;

	//////////////////////////////////////////////////////////////////////////
	//CURSOR
	//////////////////////////////////////////////////////////////////////////
	glm::vec4 cursorColor;

	int cursorIndex;
	int cursorLineIndex;

	int selectionIndex;
	int selectionStartIndex;
	int selectionEndIndex;

	int selectionLineIndex;
	int selectionStartLineIndex;
	int selectionEndLineIndex;

	int visibleStringsBegin;
	int visibleStringsEnd;
	int visibleStringsMax;

	int matchWidth;

	Timer cursorBlinkTimer;
	const std::chrono::nanoseconds cursorBlinkTime = std::chrono::milliseconds(750); //Nanoseconds

	glm::vec2 cursorPosition;

	//A jump seprator controls where the cursor should jump when you press CTRL + left or CTRL + right
	//If the separator is space it will jump to the beginning or the end of a word
	std::string jumpSeparators;
	bool jumpToBeforeSeparator;

	//Draws the background as well as text
	void DrawBackground(SpriteRenderer* spriteRenderer);
	//Draws any selection highlight
	void DrawMiddle(SpriteRenderer* spriteRenderer);
	//Draws cursor
	void DrawForeground(SpriteRenderer* spriteRenderer);

	int GetCursorIndex(int width, int line) const;

	void UpPressed(const KeyState& keyState);
	void DownPressed(const KeyState& keyState);
	void LeftPressed(const KeyState& keyState);
	void RightPressed(const KeyState& keyState);
	void BackspacePressed(const KeyState& keyState);
	void DeletePressed(const KeyState& keyState);
	void HomePressed(const KeyState& keyState);
	void EndPressed(const KeyState& keyState);

	void SetCursorIndex(int newIndex);
	void SetCursorLineIndex(int newIndex);
	void MoveCursorUp();
	void MoveCursorDown();
	void MoveCursorLeft();
	void MoveCursorRight();
	void JumpCursorLeft();
	void JumpCursorRight();

	void ExtendSelectionToCursor();
	void BeginSelection();
	void Deselect();
	void EraseSelection();
	bool SelectionMade() const;

	void UpdateText(int beginAtLine);
	void UpdateVisibleStrings();
		
	std::vector<std::tuple<std::string, bool>> CreateLineData(const std::string& text) const;

	void ScrollbarScrolled();
};

#endif // TextField_h__