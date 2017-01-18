#ifndef OPENGLWINDOW_COMMANDPRINT_H
#define OPENGLWINDOW_COMMANDPRINT_H

#include "consoleCommand.h"

/**
* Prints the given variable, expression, or text.
*
* Accessed through "Print" in the console 
*/
class CommandPrint 
	: public ConsoleCommand
{
public:
	CommandPrint(const std::string& name);
	virtual ~CommandPrint();

	virtual Argument Execute(const ContextPointers* const contextPointers, const std::vector<Argument>& arguments) override;

	std::string GetHelp() const override;
	std::string GetUsage() const override;
	std::string GetExample() const override;
};

#endif //OPENGLWINDOW_COMMANDPRINT_H
