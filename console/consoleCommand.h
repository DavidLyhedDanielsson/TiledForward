#ifndef OPENGLWINDOW_CONSOLECOMMAND_H
#define OPENGLWINDOW_CONSOLECOMMAND_H

#include <string>
#include <vector>



#include "argument.h"
#include "contextPointers.h"
#include "dictionary.h"

/**
* Forcing string arguments means that whatever the user types in, it will be parsed as a string
* So SomeMethod (alsvkdvalh"(()(0102+1+3) will call SomeMethod with (alsv...+3) as an argument
* If ALL is set, then SomeMethod a,b,c,3 will call SomeMethod with a,b,c,3 as an argument
* If PER_ARGUMENT is set, then SomeMethod a,b\,\,,c will call SomeMethod with three arguments:
* a
* b,,
* c
*/
enum class FORCE_STRING_ARGUMENTS
{
	NONE, ALL, PER_ARGUMENT
};

/**
* Which autocomplete type this command should use.
* NONE means no autocompletion will be offered at all
* ALL means both the "global" dictionary and this object's dictionary will be used
* ONLY_CUSTOM means only this object's dictionary will be used
*/
enum class AUTOCOMPLETE_TYPE
{
	NONE, ALL, ONLY_CUSTOM
};

/**
* Base class for all console commands. 
*
* When someone writes the command's name in the console Execute() will be called and all arguments will be passed.
*
* \see Argument
*/
class ConsoleCommand
{
public:
	//ConsoleCommand();
	ConsoleCommand(const std::string& name, FORCE_STRING_ARGUMENTS forceStringArguments);
	template<typename... Args>
	ConsoleCommand(const std::string& name, FORCE_STRING_ARGUMENTS forceStringArguments, AUTOCOMPLETE_TYPE autoCompleteType, Args... args)
		: ConsoleCommand(name, forceStringArguments)
	{
		this->autocompleteType = autoCompleteType;

		std::vector<std::string> arguments = { args... };
		for(const auto& argument : arguments)
			autocompleteDictionary.AddEntry(argument);
	}
	virtual ~ConsoleCommand() = default;

	virtual Argument Execute(const ContextPointers* const contextPointers, const std::vector<Argument>& arguments) = 0;

	std::string GetName() const;
	std::string GetHelpUsageExample() const;
	FORCE_STRING_ARGUMENTS GetForceStringArguments() const;

	AUTOCOMPLETE_TYPE GetAutocompleteType() const;
	const Dictionary* GetAutocompleteDictionary() const;

	void AddAutocompleteEntry(const std::string& text);
protected:
	std::string name;

	AUTOCOMPLETE_TYPE autocompleteType;
	Dictionary autocompleteDictionary;

	/**
	* Controls whether or not arguments should be converted to strings.
	* If set to NONE, arguments will be parsed as usual.
	* If set to ALL, the entire argument will be converted to a string
	* e.g. SomeMethod a(), b"123.0f, 12 will call SomeMethod with "a(), b"123.0f, 12" as a single parameters
	* If set to PER_ARGUMENT, each argument will be converted to a string
	*/
	FORCE_STRING_ARGUMENTS forceStringArguments;

	virtual std::string GetHelp() const = 0;
	virtual std::string GetUsage() const = 0;
	virtual std::string GetExample() const = 0;
};

#endif //OPENGLWINDOW_CONSOLECOMMAND_H
