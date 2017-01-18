#include "commandCallMethod.h"

CommandCallMethod::CommandCallMethod(const std::string& name, std::function<Argument(const std::vector<Argument>&)> callMethod, FORCE_STRING_ARGUMENTS forceStringArguments /*= FORCE_STRING_ARGUMENTS::NONE*/) 
	: ConsoleCommand(name, forceStringArguments)
	, callMethod(callMethod)
{}

Argument CommandCallMethod::Execute(const ContextPointers* const contextPointers, const std::vector<Argument>& arguments)
{
	//No nullptr check is intentional
	return callMethod(arguments);
}

std::string CommandCallMethod::GetHelp() const
{
	return "Calls a C++ method";
}

std::string CommandCallMethod::GetUsage() const
{
	return GetName() + "(<parameters>)\nWhere parameters is the list of parameters for the function";
}

std::string CommandCallMethod::GetExample() const
{
	return "Assuming method with these parameters: " + GetName() + "(1, 2.0, 3.0f, \"four\", Five())";
}