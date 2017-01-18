#ifndef OPENGLWINDOW_COMMANDGETSET_H
#define OPENGLWINDOW_COMMANDGETSET_H

#include "consoleVariable.h"

#include <sstream>

/**
* Gets and/or sets a C++ variable through a get method and a set method
*
* Example:\n
* The following variable can be accessed through "someVariable" in the console.
* Only typing "someVariable" gets the variable and typing "someVariable 5.0f" sets it to 5.0f
* \code
* auto getSetSomeVariable = NEW CommandGetSet<float>("someVariable", &someVariable);
* if(!console->AddCommand(getSetSomeVariable))
*     delete getSetSomeVariable;
* \endcode
*/
template<typename T>
class CommandGetSet
	: public ConsoleVariable
{
public:
	CommandGetSet(std::string name, T* value)
		: ConsoleVariable(name, FORCE_STRING_ARGUMENTS::NONE)
		, value(value)
	{ }

	virtual ~CommandGetSet() {}

	virtual Argument Execute(const ContextPointers* const contextPointers, const std::vector<Argument>& arguments) override
	{
		Argument returnArgument;
		returnArgument.origin = name;

		if(arguments.empty())
		{
			//Get
			*value >> returnArgument;
		}
		else
		{
			Argument tempArgument;
			for(const auto& argument : arguments)
				tempArgument.value += argument.value + ",";

			if(!tempArgument.value.empty())
				tempArgument.value.pop_back(); //Remove comma

			//Set
			if(!(tempArgument >> *value))
				returnArgument.value = "Couldn't insert \"" + tempArgument + "\" into value. Check your input and/or data types.";
			else
			{
				//Use sstream since it already has overloads for >> for primitive data types
				*value >> returnArgument;
				returnArgument.value.insert(0, name + " = ");

				returnArgument.type = arguments.size() == 1 ? arguments.front().type : Argument::TYPE::UNKNOWN;
			}
		}

		return returnArgument;
	}

	std::string GetHelp() const override
	{
		return "Gets or sets a C++ variable";
	}
	std::string GetUsage() const override
	{
		return "Get: " + GetName() + "\nSet: " + GetName() + "(<data>)\nWhere data is the NEW data for the variable";
	}
	std::string GetExample() const override
	{
		return "Get: " + GetName() + "\nSet (assuming data type is float): " + GetName() + "(2.0f)";
	}

	std::string GetValue() const override
	{
		Argument tempArgument;
		*value >> tempArgument;

		std::string tempString;
		tempArgument >> tempString;

		return tempString;
	}

protected:
	T* value;
};

#endif //OPENGLWINDOW_COMMANDGETSET_H
