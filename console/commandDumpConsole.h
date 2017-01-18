#ifndef OPENGLWINDOW_COMMANDDUMPCONSOLE_H
#define OPENGLWINDOW_COMMANDDUMPCONSOLE_H

#include "consoleCommand.h"

class CommandDumpConsole
	: public ConsoleCommand
{
public:
	CommandDumpConsole(const std::string& name);
	virtual ~CommandDumpConsole() = default;

	Argument Execute(const ContextPointers* const contextPointers, const std::vector<Argument>& arguments) override;

	std::string GetHelp() const override;
	std::string GetUsage() const override;
	std::string GetExample() const override;
};

#endif //OPENGLWINDOW_COMMANDDUMPCONSOLE_H
