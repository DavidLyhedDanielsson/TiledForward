#include "commandPrint.h"

CommandPrint::CommandPrint(const std::string& name)
		: ConsoleCommand(name, FORCE_STRING_ARGUMENTS::NONE)
{

}

CommandPrint::~CommandPrint()
{

}

Argument CommandPrint::Execute(const ContextPointers* const contextPointers, const std::vector<Argument>& arguments)
{
	Argument returnArgument;

	returnArgument.type = Argument::TYPE::STRING;
	returnArgument.origin = "CommandPrint";

	for(const Argument& argument : arguments)
		returnArgument.value += argument.origin + " = " + argument + "\n";

	returnArgument.value.pop_back(); //Pop \n

	return returnArgument;
}

std::string CommandPrint::GetHelp() const
{
	return "Prints the given variables";
}

std::string CommandPrint::GetUsage() const
{
	return GetName() + "(<parameters>)\nWhere parameters is the list of parmeters to print";
}

std::string CommandPrint::GetExample() const
{
	return GetName() + "(1, 2.0, 2.0f, \"three\", Four())";
}