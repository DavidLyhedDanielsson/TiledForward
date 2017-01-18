#ifndef consoleVariable_h__
#define consoleVariable_h__

#include "consoleCommand.h"

/**
* A ConsoleVariable (unlike ConsoleCommand) has a GetValue method defined
*/
class ConsoleVariable 
	: public ConsoleCommand
{
public:
	ConsoleVariable(const std::string& name, FORCE_STRING_ARGUMENTS forceStringArguments)
		: ConsoleCommand(name, forceStringArguments)
	{}
	virtual ~ConsoleVariable() = default;

	/**
	* Gets the value of this variable as an std::string
	* 
	* \returns the variable's value
	*/
	virtual std::string GetValue() const = 0;
};

#endif // consoleVariable_h__
