#ifndef OPENGLWINDOW_CONSOLECOMMANDMANAGER_H
#define OPENGLWINDOW_CONSOLECOMMANDMANAGER_H

#include <memory>
#include <queue>
#include <string>
#include <map>
#include <stack>

#include "textBox.h"
#include "textField.h"
#include "list.h"

#include "dictionary.h"
#include "consoleCommand.h"
#include "consoleVariable.h"
#include "contextPointers.h"

class Console;

/**
* Manages adding and executing console commands.
*
* \see AddCommand for information regarding adding commands 
* \see ExecuteCommand for information regarding executing commands
*/
class ConsoleCommandManager
{
public:
	ConsoleCommandManager(TextBox* input
		, TextField* output
		, List* completeList
		, ConsoleAutoexecManager* autoexecManager
		, Console* console);
	~ConsoleCommandManager() = default;

	/**
	* Adds the given <b>dynamically allocated</b> command so that it can be called through this manager
	*
	* If the command is successfully added, its memory will be deallocated by this manager
	* when it is destroyed.
	*
	* \see VerifyCommandName for command naming rules
	* 
	* \param command command to add
	* \returns an empty string if no error occurred, otherwise returns an error string
	*/
	std::string AddCommand(ConsoleCommand* command);

	/**
	* Removes the given command
	*
	* \param commandName
	* \returns whether or not the command existed
	*/
	bool RemoveCommand(const std::string& commandName);

	/**
	* Parses the function call and arguments from \p text
	*
	* Example:
	* `ParseFunctionAndArgumentList("Functioncall(arg1, arg2)")` will return
	* 1. "Functioncall"
	* 2. "arg1, arg2"
	* \param text text to parse
	* \returns a pair containing the function to call and then the arguments
	*/
	std::pair<std::string, std::string> ParseFunctionAndArgumentList(const std::string& text);

	/**
	* Executes the given command
	*
	* Commands only need parentheses if one of the arguments need parentheses.
	* Thus, these are all legal:
	* 1. FunctionCall
	* 2. FunctionCall()
	* 3. FunctionCall arg1, arg2
	* 4. FunctionCall(arg1, arg2)
	* 5. FunctionCall OtherFunction, AnotherFunction
	* 6. FunctionCall(OtherFunction, AnotherFunction)
	* 7. FunctionCall(OtherFunction(1, 2), AnotherFunction("Argument"))
	*
	* And these are \b not legal:
	* 1. FunctionCall OtherFunction()
	* 2. FunctionCall OtherFunction(), AnotherFunction()
	*
	* \throws std::invalid_argument if \p text is invalid, exception contains details
	* \param text command to execute
	* \returns a tuple containing: the command's returned Argument cast to a string, the function called, the function's parameters
	*/
	std::tuple<std::string, std::string, std::string> ExecuteCommand(std::string text);

	/**
	* Matches the given text with all available commands
	*
	* Matched using DictionaryEntry::Matches
	* 
	* \param text text to match against all commands
	* \returns an std::vector containing all commands with a matching name
	*/
	std::vector<std::string> Match(const std::string& text) const;

	/**
	* Returns the variable with the given name if it is found
	* 
	* \param variable variable to search for
	* \returns nullptr if the variable didn't exist, otherwise the found variable
	*/
	ConsoleVariable* GetVariable(const std::string& variable) const;
	/**
	* Returns the command with the given name if it is found
	* 
	* \param command variable to search for
	* \returns nullptr if the command didn't exist, otherwise the found command
	*/
	ConsoleCommand* GetCommand(const std::string& command) const;

	/**
	* Verifies that \p name is a valid command name
	*
	* Command naming rules are:
	* 1. Can only begin with a letter
	* 2. Can only contain letters (a-z, A-Z), numbers (0-9), and underscores
	* 
	* \param name name to verify
	* \returns whether or not the command is valid
	*/
	bool VerifyCommandName(const std::string& name) const;

private:
	enum class OPERATORS { PLUS = 0, MINUS, MULT, DIV, NONE };

	ContextPointers contextPointers;

	Dictionary commandDictionary;
	//Maps a string to a ConsoleCommand
	std::map<std::string, std::unique_ptr<ConsoleCommand>> commandMap;

	/**
	* Same as ConsoleCommandManager::ExecuteCommand, except this returns an argument instead of a string
	* 
	* \throws std::invalid_argument if \p text is invalid, exception contains details
	* \param text command to execute
	* \returns the command's returned Argument
	*/
	Argument ExecuteArgumentFunction(std::string text);
	/**
	* Turns the given text into an Argument
	*
	* Example:
	* EvaluateExpression("1") returns an Argument of type int with value "1"
	* EvaluateExpression("1.0f") returns an Argument of type float with value "1.0f"
	* EvaluateExpression("1.0") returns an Argument of type double with value "1.0"
	* EvaluateExpression("\"string\"") returns an Argument of type string with value "string"
	* EvaluateExpression("true") returns an Argument of type bool with value "true"
	* 
	* \throws std::invalid_argument if \p text is invalid, exception contains details
	* \param expression
	* \returns
	*/
	Argument EvaluateExpression(std::string expression);
	/**
	* Turns numbers into a single Argument
	*
	* Example:
	* EvaluateExpression("1 + 1") returns an Argument of type int with value "2"
	* EvaluateExpression("1 + 1.0f") returns an Argument of type float with value "2.0f"
	* EvaluateExpression("1") returns an Argument of type int with value "1"
	* EvaluateExpression("1.0f") returns an Argument of type float with value "1.0f"
	* EvaluateExpression("1.0") returns an Argument of type double with value "1.0"
	* EvaluateExpression("true") returns an Argument of type bool with value "true"
	* 
	* \throws std::invalid_argument if \p text is invalid, exception contains details
	* \param expression
	* \returns
	*/
	Argument ParseExpression(std::string expression);
	/**
	* Calculates a mathematical expression
	*
	* Example:
	* EvaluateExpression("1 + 1") returns an Argument of type int with value "2"
	* EvaluateExpression("1 + 1.0f") returns an Argument of type float with value "2.0f"
	* 
	* \param expressionParts a queue with parts of the expression. View ConsoleCommandManager::SplitExpression
	* \returns
	*/
	Argument CalculateExpression(std::queue<std::string>& expressionParts);

	/**
	* Takes two operands from \p operands and one operator from \p operators and performs one calculation.
	* Result is pushed onto \p operands
	*
	* If \p operands contains less than two operands, "0" will be used instead
	*
	* \param[out] operands a stack containing operands to base the calculation off of
	* \param[out] operators a stack containing operators to base the calculation off of
	*/
	void ExecuteSingleCalculation(std::stack<Argument>& operands, std::stack<OPERATORS>& operators);

	/**
	* Splits \p args by \p delimiter
	*
	* Example: SplitArg("ab,cd,ef", ','); returns "ab","cd","ef"
	* Example: SplitArg("ab cd ef", ' '); returns "ab","cd","ef"
	* 
	* \param args the string to split
	* \param delimiter delimiter character
	* \returns a vector of strings containing the arguments as split by \p delimiter
	*/
	std::vector<std::string> SplitArg(std::string args);
	/**
	* Splits a mathematical expression into its components
	*
	* The components are:
	* 1. Arithmetic operators (+-/*)
	* 2. Parentheses
	* 3. Operands (function calls, numbers)
	*
	* Example:
	* `SplitExpression("(1 + 2) * 3.0f")` returns
	* "(", "1", "+", "2", ")", "*", "3.0f"
	* 
	* \param expression mathematical expression to split
	* \returns a queue containing the split expression
	*/
	std::queue<std::string> SplitExpression(std::string expression);

	/**
	* Removes unnecessary whitespace
	*
	* Example: `TrimText("   Function ( "parameter one", 1.0f)   ");`
	* returns "Function("parameter one",1.0f)"
	* 
	* \param text text to trim
	* \returns trimmed text
	*/
	std::string TrimText(std::string text);
	/**
	* Removes unnecessary whitespace from the front and back of strings
	* 
	* \param text text to trim
	* \returns trimmed text
	*/
	std::string TrimTextFrontBack(const std::string& text);

	/**
	* Tries using the >> operator to extract data from /p sstream to a variable of type T
	*
	* Example:
	* 
	* \param[out] sstream stream to try extracting from
	* \returns whether or not extraction succeeded
	*/
	template<typename T>
	bool TryExtract(std::stringstream& sstream) const
	{
		T testVal;
		sstream >> testVal;

		return !sstream.fail();
	}

	/**
	* Makes sure every left parenthesis has a buddy right parenthesis 
	*
	* Example:
	* VerifyParenthesesBalance("abc(d(), e())") returns true
	* VerifyParenthesesBalance("abc("ab(((")") return true
	* VerifyParenthesesBalance("abc(d()") returns false
	* 
	* \param text text to varify
	* \returns whether or not the amount of parentheses were balanced
	*/
	bool VerifyParenthesesBalance(const std::string& text) const;

};

#endif //OPENGLWINDOW_CONSOLECOMMANDMANAGER_H
