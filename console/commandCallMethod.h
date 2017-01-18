#ifndef OPENGLWINDOW_COMMANDCALLMETHOD_H
#define OPENGLWINDOW_COMMANDCALLMETHOD_H

#include "consoleCommand.h"

#include <functional>

/**
* Calls a C++ method
*
* Example:\n
* The following command can be accessed through "SomeMethod" in the console.
* \code
* auto someMethodCommand = NEW CommandCallMethod("SomeMethod", std::bind(&SomeClass::SomeMethod, this, std::placeholders::_1);
* if(!console->AddCommand(someMethodCommand))
*     delete someMethodCommand;
* \endcode
*/
class CommandCallMethod
		: public ConsoleCommand
{
public:
	CommandCallMethod(const std::string& name, std::function<Argument(const std::vector<Argument>&)> callMethod, FORCE_STRING_ARGUMENTS forceStringArguments = FORCE_STRING_ARGUMENTS::NONE);
	template<typename... Args>
	CommandCallMethod(const std::string& name, std::function<Argument(const std::vector<Argument>&)> callMethod, FORCE_STRING_ARGUMENTS forceStringArguments, AUTOCOMPLETE_TYPE autoCompleteType, Args... args)
		: ConsoleCommand(name, forceStringArguments, autoCompleteType, args...)
	{
		this->callMethod = callMethod;
	}
	virtual ~CommandCallMethod() = default;

	Argument Execute(const ContextPointers* const contextPointers, const std::vector<Argument>& arguments) override;

	std::string GetHelp() const override;
	std::string GetUsage() const override;
	std::string GetExample() const override;

protected:
	std::function<Argument(const std::vector<Argument>&)> callMethod;
};

#endif //OPENGLWINDOW_COMMANDCALLMETHOD_H
