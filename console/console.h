#ifndef OPENGLWINDOW_CONSOLE_H
#define OPENGLWINDOW_CONSOLE_H

#include "guiWidgetStyled.h"

#include "guiBackground.h"
#include "textField.h"
#include "textBox.h"
#include "guiManager.h"
#include "list.h"
#include "label.h"

#include "consoleStyle.h"
#include "consoleAutoexecManager.h"
#include "consoleCommandManager.h"
#include "contextPointers.h"

#include <sstream>
#include <unordered_set>
#include <deque>
#include <queue>



#ifdef DETAILED_GRAPHS
class TimeManager;
struct GraphicsSettings;
#endif

#ifndef ConsoleSafeAdd
#define ConsoleSafeAdd(x)				\
{										\
	auto command = x;					\
	if(!console->AddCommand(command))	\
		delete command;					\
}
#endif

/**
* A highly customizable in-game console where the user can enter commands.
* The GUI can be customized through ConsoleStyle, and by giving Init 
* different backgrounds and background styles. To generate a default
* Doom-esque console use something like this:
*
* \code
* auto consoleStyle = std::shared_ptr<GUIStyle>(Console::GenerateDoomStyle(&contentManager));
* auto consoleBackground = std::unique_ptr<GUIBackground>(Console::GenerateDoomStyleBackground(&contentManager));
* auto consoleBackgroundStyle = std::shared_ptr<GUIStyle>(Console::GenerateDoomStyleBackgroundStyle(&contentManager));
*
* console.Init(Rect(0.0f, 0.0f, 1280.0f, 256.0f), consoleStyle, consoleBackground, consoleBackgroundStyle);
* guiManager.AddContainer(&console);
* \endcode
*
* Supports user-defined commands by inheriting ConsoleCommand and then adding them
* by calling AddCommand
*
* Also supports mathematical expressions so that the user can call some commands
* with different formulas as arguments.
*
* Optionally, an autoexec file can be used. When Autoexec is called, the file given
* in ConsoleStyle will be parsed and all commands inside it will be executed
*
* \see ConsoleCommandManager for information regarding commands
* \see ConsoleAutoexecManager for information regarding autoexec
*/
class Console
	: public GUIWidgetStyled
{
friend class GUIManager;
public:
	Console();
	virtual ~Console();

	Console(Console&& other) = delete;
	Console& operator=(Console& other) = delete;

	virtual bool Init(ContentManager* contentManager
					  , Rect area
					  , const std::shared_ptr<ConsoleStyle>& style
					  , const std::shared_ptr<GUIBackgroundStyle>& backgroundStyle
					  , bool allowMove
					  , bool updateWhenMoving
					  , bool allowResize
					  , bool updateWhenResizing
					  , MOUSE_BUTTON moveResizeButton = MOUSE_BUTTON::LEFT
					  , KEY_MODIFIERS moveResizeModifierKeys = KEY_MODIFIERS::CONTROL
					  , GUI_ANCHOR_POINT anchorPoint = GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT
					  , glm::vec2 minPosition = glm::vec2(std::numeric_limits<float>::min(), std::numeric_limits<float>::min())
					  , glm::vec2 maxPosition = glm::vec2(std::numeric_limits<float>::max(), std::numeric_limits<float>::max())
					  , glm::vec2 minSize = glm::vec2(std::numeric_limits<float>::min(), std::numeric_limits<float>::min())
					  , glm::vec2 maxSize = glm::vec2(std::numeric_limits<float>::max(), std::numeric_limits<float>::max()));

	virtual bool Init(ContentManager* contentManager
					, glm::vec2 position
					, glm::vec2 size
					, const std::shared_ptr<ConsoleStyle>& style
					, const std::shared_ptr<GUIBackgroundStyle>& backgroundStyle
					, bool allowMove
					, bool updateWhenMoving
					, bool allowResize
					, bool updateWhenResizing
					, MOUSE_BUTTON moveResizeButton = MOUSE_BUTTON::LEFT
					, KEY_MODIFIERS moveResizeModifierKeys = KEY_MODIFIERS::CONTROL
					, GUI_ANCHOR_POINT anchorPoint = GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT
					, glm::vec2 minPosition = glm::vec2(std::numeric_limits<float>::min(), std::numeric_limits<float>::min())
					, glm::vec2 maxPosition = glm::vec2(std::numeric_limits<float>::max(), std::numeric_limits<float>::max())
					, glm::vec2 minSize = glm::vec2(std::numeric_limits<float>::min(), std::numeric_limits<float>::min())
					, glm::vec2 maxSize = glm::vec2(std::numeric_limits<float>::max(), std::numeric_limits<float>::max()));

	void SubUpdate(std::chrono::nanoseconds delta) override;

	void SubDraw(SpriteRenderer* spriteRenderer) override;

	/**
	* Executes the given command
	*
	* \param command command to execute
	* \returns the called function's return value as a string, or an error message if
	* an error occurred
	*/
	std::string ExecuteCommand(const std::string& command);

	/**
	* Adds the given command so that it can be called from the console.
	* If the command is successfully added, its memory will be deallocated 
	* automatically when the console is destroyed
	*
	* \b Remember to dynamically allocate the command
	* and to deallocate it if this method returns false
	*
	* \see ConsoleCommandManager::VerifyCommandName for command namig rules
	*
	* Example:
	* \code
	* ConsoleCommand* someCommand = NEW ...
	* if(!AddCommand(someCommand))
	*     delete someCommand;
	* \endcode
	* 
	* \param command command to add
	* \returns true if no error occurred, if an error occurred its text
	* will be added to this console's output text area through Console::AddText
	*/
	bool AddCommand(ConsoleCommand* command);

	/**
	* Removes the given command
	*
	* \param commandName
	* \returns whether or not the command existed
	*/
	bool RemoveCommand(const std::string& commandName);

	/**
	* Adds \p text to this console's output text area
	* 
	* \param text text to add
	*/
	void AddText(const std::string& text);

	using GUIWidgetStyled::SetPosition;
	void SetPosition(float x, float y, GUI_ANCHOR_POINT anchorPoint) override;
	using GUIWidgetStyled::SetSize;
	void SetSize(float x, float y, GUI_ANCHOR_POINT anchorPoint) override;

	void SetDraw(bool draw) override;

	void Activate() override;
	void Deactivate() override;

	bool GetActive() const;

	/**
	* Parses the autoexec file at ConsoleStyle#autoexecFile
	*/
	void Autoexec();
	/**
	* Adds a command to autoexec
	*
	* \see ConsoleAutoexecManager::AddAutoexecWatch for usage
	* 
	* \param command command to add to autoexec watches
	*/
	bool AddAutoexecWatch(const std::string& command);
	/**
	* Removes a command from autoexec
	*
	* \see ConsoleAutoexecManager::RemoveAutoexecWatch for usage
	*
	* \param command command to remove from autoexec watches
	*/
	bool RemoveAutoexecWatch(const std::string& command);
	/**
	* Prints all autoexec watches to this console's output text area
	*/
	void PrintAutoexecWatches();

	//TODO: Pause autoexec controls

	bool OnMouseEnter() override;
	void OnMouseExit() override;
	bool SubOnMouseDown(const MouseButtonState& keyState, const glm::vec2& mousePosition) override;
	void SubOnMouseUp(const MouseButtonState& keyState, const glm::vec2& mousePosition) override;
	bool SubOnKeyDown(const KeyState& keyState) override;
	void SubOnKeyUp(const KeyState& keyState) override;
	bool OnChar(unsigned int keyCode) override;
	bool OnScroll(int distance) override;

	/**
	* Generates a style which makes this console look somewhat like the original Doom's console
	*
	* \see Console::GenerateDoomStyleBackground
	* \see Console::GenerateDoomStyleBackgroundStyle
	*
	* Example:
	* \code
	* auto style = std::shared_ptr<GUIStyle>(Console::GenerateDoomStyle(contentManager));
	* console.Init(..., style, ...);
	* \endcode
	* 
	* \param contentManager ContentManager to load content from
	* \returns a <b>dynamically allocated</b> GUIStyle* pointing to a ConsoleStyle*
	*/
	std::shared_ptr<ConsoleStyle>GenerateDoomStyle(ContentManager* contentManager, CharacterSet* characterSet);
	/**
	* Generates a background which makes this console look somewhat like the original Doom's console
	*
	* \see Console::GenerateDoomStyle
	* \see Console::GenerateDoomStyleBackgroundStyle
	*
	* Example:
	* \code
	* auto background = std::shared_ptr<GUIBackground>(Console::GenerateDoomStyleBackground(contentManager));
	* console.Init(..., std::move(background), ...);
	* \endcode
	*
	* \param contentManager ContentManager to load content from
	* \returns a <b>dynamically allocated</b> GUIBackground* pointing to a OutlineBackground*
	*/
	std::unique_ptr<GUIBackground>GenerateDoomStyleBackground(ContentManager* contentManager);
	/**
	* Generates a background style which makes this console look somewhat like the original Doom's console
	*
	* \see Console::GenerateDoomStyle
	* \see Console::GenerateDoomStyleBackground
	*
	* Example:
	* \code
	* auto backgroundStyle = std::shared_ptr<GUIBackgroundStyle>(Console::GenerateDoomStyleBackgroundStyle(contentManager));
	* console.Init(..., backgroundStyle, ...);
	* \endcode
	*
	* \param contentManager ContentManager to load content from
	* \returns a <b>dynamically allocated</b> GUIBackground* pointing to a OutlineBackgroundStyle*
	*/
	std::shared_ptr<GUIBackgroundStyle>GenerateDoomStyleBackgroundStyle(ContentManager* contentManager);

protected:
	enum COMPLETE_LIST_MODE { HISTORY, COMPLETION };
	COMPLETE_LIST_MODE completeListMode;

	ContextPointers* contextPointers;
	ConsoleCommandManager commandManager;
	ConsoleAutoexecManager autoexecManager;

	/**
	* Called when Console_AddAutoexecWatch is executed
	*
	* \see Argument
	* \see CommandCallMethod
	*/
	Argument AddAutoexecWatchInternal(const std::vector<Argument>& arguments);
	/**
	* Called when Console_RemoveAutoexecWatch is executed
	*
	* \see Argument
	* \see CommandCallMethod
	*/
	Argument RemoveAutoexecWatchInternal(const std::vector<Argument>& arguments);
	/**
	* Called when Print is executed
	*
	* \see Argument
	* \see CommandCallMethod
	*/
	Argument PrintAutoexecWatchesInternal(const std::vector<Argument>& arguments);

	/**
	* Called when PauseAutoexec is executed
	*
	* \see Argument
	* \see CommandCallMethod
	*/
	Argument PauseAutoexecInternal(const std::vector<Argument>& arguments);
	/**
	* Called when ResumeAutoexec is executed
	*
	* \see Argument
	* \see CommandCallMethod
	*/
	Argument ResumeAutoexecInternal(const std::vector<Argument>& arguments);
	/**
	* Called when ApplyPausedAutoexecWatches is executed
	*
	* \see Argument
	* \see CommandCallMethod
	*/
	Argument ApplyPausedAutoexecWatchesInteral(const std::vector<Argument>& arguments);

	//this->draw is always true since Draw should always be called
	//This is whether or not to draw the actual console
	bool actualDraw;

	//Used for mouse events for input, output, and completeList
	GUIManager manager;

	std::shared_ptr<ConsoleStyle> style;

	TextField output;
	TextBox input;
	List completeList;
	Label promptLabel;

	//Need this to be able to call its methods
	GUIBackground* completeListBackground;

	//Currently selected index when completeList is in history resp. suggestion mode
	int historyIndex;
	int suggestionIndex;

	//<message, duration shown>
	TextField lastMessages;
	std::queue<int> lastMessagesTexts;
	float lastMessagesDuration;

	//History of all entered strings
	std::deque<std::string> history;
	//List of created buttons for all buttons of the respective type, ready to move to completeList.
	//When moved, completeList will hold the raw pointers and the unique_ptr will stay inside these vectors
	std::vector<std::unique_ptr<GUIContainer>> historyButtons;
	std::vector<std::unique_ptr<GUIContainer>> suggestionButtons;
	//All suggestions from the commandDictionary
	std::vector<std::string> suggestions;

	/**
	* Called when OnKeyDown is called with GLFW_KEY_UP
	*/
	void UpPressed();
	/**
	* Called when OnKeyDown is called with GLFW_KEY_DOWN
	*/
	void DownPressed();
	/**
	* Called when OnKeyDown is called with GLFW_KEY_ENTER
	*
	* Enter is used to accept suggestions or to submit commands
	*/
	void EnterPressed();
	/**
	* Called when OnKeyDown is called with GLFE_KEY_BACKSPACE
	*/
	void BackspacePressed();
	/**
	* Called when OnKeyDown is called with GLFW_KEY_DELETE
	*/
	void DeletePressed();
	/**
	* Called when OnKeyDown is called with GLFW_KEY_TAB
	*
	* Tab is used to accept suggestions
	*/
	void TabPressed();

	/**
	* Adds \p text to #history if it's needed, if \p text is already in #history it will be moved to the front
	*
	* \param text text to add to history
	*/
	void AddToHistoryIfNeeded(const std::string& text);
	/**
	* Adds \p text to #history
	*
	* \param text text to add to history
	*/
	void AddToHistory(const std::string& text);
	/**
	* Moves \p index of #history to the front
	*
	* \param index index in #history to move to the front
	*/
	void MoveToFrontOfHistory(int index);
	/**
	* Moves \iter to the front of #history
	*
	* \param iter the iterator from #history to move to the front of #history
	* \returns
	*/
	void MoveToFrontOfHistory(std::deque<std::string>::iterator iter);

	/**
	* Clears #completeList and inserts all elements from #historyButtons 
	*/
	void MoveHistoryButtonsToCompleteList();
	/**
	* Clears #completeList and inserts all elements from #suggestionButtons
	*/
	void MoveSuggestionButtonsToCompleteList();

	/**
	* Stops #completeList from being drawing
	*/
	void HideCompleteList();
	/**
	* Starts drawing #completeList again
	*/
	void ShowCompleteList();
	/**
	* Updates the position of #completeList
	*
	* \b Note: will update the size of #completeList if it would go outside the window
	*/
	void UpdateCompleteListPosition();
	/**
	* Updates the size of #completeList
	*/
	void UpdateCompleteListSize();
	/**
	* Updates both the size and the position of #completeList
	*/
	void UpdateCompleteListArea();

	/**
	* Switches between suggestion mode and completion mode
	*
	* Example:
	* 
	* \param mode NEW mode, checks if #completeListMode is already \p mode
	* \returns
	*/
	void SwitchCompleteListMode(COMPLETE_LIST_MODE mode);
	/**
	* Generates commands by using ConsoleCommandManager::Match, also populates updates #suggestionButtons
	* 
	* \param text text to match against. Used as an argument to ConsoleCommandManager::Match
	*/
	void GenerateSuggestions(const std::string& text);

	/**
	* Highlights the given index in #completeList through List::HighlightElement. No bounds checking is performed
	* 
	* \param index index to highlight
	*/
	void HighlightCompleteListIndex(int index);
	/**
	* Replaces all/some text in #input with \p text.
	*
	* Called for instance when the user selects an index in #completeList
	* 
	* \param text text to use when replacing
	*/
	void AcceptText(std::string text);

	/**
	* Generates the text for which suggestions should be generated.
	*
	* If the cursor is at the end:\n
	* "Foo(bar" should generate suggestions for "bar"\n
	* "Foo(bar,bar1" should generate suggestions for "bar1"
	*
	* If the cursor is over Foo:\n
	* "Foo(bar" should generate suggestions for "Foo"\n
	* "Foo(bar,bar1" should generate suggestions for "Foo"
	* 
	* \returns the text for which suggestions should be generated
	*/
	std::string GenerateSuggestionText();

};

#endif //OPENGLWINDOW_CONSOLE_H
