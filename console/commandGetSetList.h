#ifndef OPENGLWINDOW_COMMANDGETSETLIST_H
#define OPENGLWINDOW_COMMANDGETSETLIST_H

#include "consoleVariable.h"

#include <sstream>

/**
* Gets and/or sets multiple C++ variables to the same value
*
* Use AddValue() to add value to the get/set list
*
* \see CommandGetSet for example
*
* \endcode
*/
template<typename T>
class CommandGetSetList
	: public ConsoleVariable<T>
{
public:
	CommandGetSetList(std::string name)
		: ConsoleVariable(name, Argument::TYPE::VARIABLE, false)
	{}
	virtual ~CommandGetSetList() = default;

	void AddValue(T* value)
	{
		values.emplace_back(value);
	}

	virtual std::vector<Argument> Execute(const ContextPointers* const contextPointers, const std::vector<Argument>& arguments) override
	{
		std::vector<Argument> returnArguments;

		//Use sstream since it allows for overloading of << and >> for custom classes
		if(arguments.empty())
		{
			//Get
			if(values.size() == 0)
				returnArguments.emplace_back("null");
			else
				*values[0] >> returnArguments;

			if(returnArguments.size() == 1)
				returnArguments[0].origin = name;
			else
			{
				for(int i = 0, end = static_cast<int>(returnArguments.size()); i < end; ++i)
					returnArguments[i].origin = name + "[" + std::to_string(i) + "]";
			}
		}
		else
		{
			if(values.size() == 0)
				returnArguments.emplace_back("No values in list");
			else
			{
				//Set
				bool success = true;
				for(T* value : values)
				{
					success = arguments >> *value;

					if(!success)
						break;
				}

				if(!success)
				{
					returnArguments.emplace_back("Couldn't insert \"" + arguments + "\" into one or more values. Check your input and/or data types.");
					returnArguments.back().origin = name;
				}
				else
				{
					std::stringstream stream;
					stream << name + " = ";
					stream << *values[0];

					returnArguments.emplace_back(stream.str());
					returnArguments.back().type = arguments.front().type;
					returnArguments.back().origin = name;
				}
			}
		}

		return returnArguments;
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
		if(values.size == 0)
			return T();
		else
		{
			Argument argument;

			*values.front() >> argument;

			return argument.value;
		}
	}

protected:
	std::vector<T*> values;
};

#endif //OPENGLWINDOW_COMMANDGETSETLIST_H
