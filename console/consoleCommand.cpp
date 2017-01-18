#include "consoleCommand.h"

ConsoleCommand::ConsoleCommand(const std::string& name, FORCE_STRING_ARGUMENTS forceStringArguments)
	: name(name)
	, autocompleteType(AUTOCOMPLETE_TYPE::ALL)
	, forceStringArguments(forceStringArguments)
{
}

std::string ConsoleCommand::GetName() const
{
	return name;
}

std::string ConsoleCommand::GetHelpUsageExample() const
{
	return GetHelp() + "\nUsage: " + GetUsage() + "\nExample: " + GetExample();
}

FORCE_STRING_ARGUMENTS ConsoleCommand::GetForceStringArguments() const
{
	return forceStringArguments;
}

AUTOCOMPLETE_TYPE ConsoleCommand::GetAutocompleteType() const
{
	return autocompleteType;
}

const Dictionary* ConsoleCommand::GetAutocompleteDictionary() const
{
	return &autocompleteDictionary;
}

void ConsoleCommand::AddAutocompleteEntry(const std::string& text)
{
	autocompleteDictionary.AddEntry(text);
}
