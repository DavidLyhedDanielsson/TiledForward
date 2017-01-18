#include "consoleCommandManager.h"

#include <stack>
#include <cctype>

/*
ConsoleCommandManager::ConsoleCommandManager()
{}*/

ConsoleCommandManager::ConsoleCommandManager(TextBox* input
	, TextField* output
	, List* completeList
	, ConsoleAutoexecManager* autoexecManager
	, Console* console)
	: contextPointers(input, output, completeList, this, autoexecManager, console)
{

}

std::string ConsoleCommandManager::AddCommand(ConsoleCommand* command)
{
	if(VerifyCommandName(command->GetName()))
	{
		if(commandMap.find(command->GetName()) == commandMap.end())
		{
			commandMap.insert(std::make_pair(command->GetName(), std::unique_ptr<ConsoleCommand>(command)));
			commandDictionary.AddEntry(command->GetName());

			return "";
		}
		else
			return "Tried to add already existing command \"" + command->GetName() + "\"";
	}

	return "Couldn't add command \"" + command->GetName() + "\" since it has an invalid name (only a-z, A-Z, 0-9, and _ allowed. Must start with a letter)";
}

bool ConsoleCommandManager::RemoveCommand(const std::string& commandName)
{
	if(commandMap.find(commandName) == commandMap.end())
		return false;

	commandMap.erase(commandName);

	return true;
}

std::pair<std::string, std::string> ConsoleCommandManager::ParseFunctionAndArgumentList(const std::string& text)
{
	//Anything with quotes around it (") will be regarded as one single argument
	//Any variable/function wrapped in quotes will be sent as-is as a string
	//Any variable/function not wrapped in quotes will be resolved
	//Any argument that is a function call needs to use parentheses to mark its parameter list

	//Can't call TrimText here since it's legal to write "Function arg1, arg2, arg3"
	std::string trimmed = TrimTextFrontBack(text);

	//Space or parenthesis, whichever comes first marks end of function and
	//beginning of parameters
	auto splitIndex = trimmed.find_first_of("( ");

	std::string function = trimmed.substr(0, splitIndex);

	bool parentheses = false;

	if(splitIndex != function.npos)
		parentheses = text[splitIndex] == '(';

	//Remove parentheses if needed
	//SomeFunction(abc) will strip parentheses
	//SomeFunction (abc) will not strip parentheses
	if(parentheses)
		return std::make_pair(function, trimmed.substr(splitIndex + 1, trimmed.size() - splitIndex - 2));
	else
	{
		if(splitIndex != function.npos)
			return std::make_pair(function, trimmed.substr(splitIndex + 1));
		else
			return std::make_pair(function, "");
	}
}

std::tuple<std::string, std::string, std::string> ConsoleCommandManager::ExecuteCommand(std::string text)
{
	auto functionParam = ParseFunctionAndArgumentList(text);

	return std::make_tuple(std::string(ExecuteArgumentFunction(text)), functionParam.first, functionParam.second);
}

std::vector<std::string> ConsoleCommandManager::Match(const std::string& text) const
{
	return commandDictionary.Match(text);
}

ConsoleVariable* ConsoleCommandManager::GetVariable(const std::string& variable) const
{
	if(commandMap.count(variable) == 0)
		return nullptr;

	return dynamic_cast<ConsoleVariable*>(commandMap.at(variable).get());
}

ConsoleCommand* ConsoleCommandManager::GetCommand(const std::string& command) const
{
	if(commandMap.count(command) == 0)
		return nullptr;

	return commandMap.at(command).get();
}

Argument ConsoleCommandManager::ExecuteArgumentFunction(std::string text)
{
	try
	{
		auto functionParam = ParseFunctionAndArgumentList(text);

		if(commandMap.find(functionParam.first) == commandMap.end())
			throw std::invalid_argument("Evaluated \"" + text + "\" to be a function call to \"" + functionParam.first + "\" , but no such function was found");

		std::vector<Argument> arguments;
		if(commandMap[functionParam.first]->GetForceStringArguments() == FORCE_STRING_ARGUMENTS::ALL)
		{
			arguments.push_back(functionParam.second);
		}
		else if(commandMap[functionParam.first]->GetForceStringArguments() == FORCE_STRING_ARGUMENTS::PER_ARGUMENT)
		{
			if(!functionParam.second.empty())
			{
				std::string argument;
				auto lastCharacter = functionParam.second.front();

				for(const auto& character : functionParam.second)
				{
					if(character == ','
					   && lastCharacter != '\\')
					{
						arguments.push_back(argument);
						argument.clear();
					}
					else
						argument += character;

					lastCharacter = character;
				}

				if(!argument.empty())
					arguments.push_back(argument);
			}

			//std::vector<std::string> textArguments = SplitArg(functionParam.second);
			//for(const std::string& argument : textArguments)
			//	arguments.emplace_back(argument);
		}
		else
		{
			functionParam.second = TrimText(functionParam.second);

			std::vector<std::string> textArguments = SplitArg(functionParam.second);
			for(const std::string& argument : textArguments)
				arguments.emplace_back(EvaluateExpression(argument));
		}

		return commandMap[functionParam.first]->Execute(&contextPointers, arguments);
	}
	catch(std::invalid_argument& ex)
	{
		Argument exceptionArgument(ex.what());
		exceptionArgument.origin = "invalid_argument exception";

		return exceptionArgument;
	}
}

Argument ConsoleCommandManager::EvaluateExpression(std::string expression) /*throws invalid_argument */
{
	Argument returnArgument;
	returnArgument.value = expression;

	if(expression.size() == 0)
		returnArgument.type = Argument::TYPE::NONE;
	else
	{
		//There are three different things to extract:
		//"adsf" => string, asdf
		//someFunction(parameters) => call function and return its return value
		//numbers => int32/int64/uint64/float/double, number
		if(expression[0] == '"')
		{
			//string
			for(int i = 1, end = static_cast<int>(expression.size() - 1); i < end; ++i)
			{
				auto character = expression[i];

				if(character == '"' && expression[i - 1] != '\\')
					throw std::invalid_argument("Evaluated \"" + expression + "\" to be a string, but some additional quote (\") was found");
				else if(character == '\\' && expression[i + 1] == '"')
				{
					expression.erase(i, 1);
					--end;
				}
			}

			if(expression.back() != '"')
				throw std::invalid_argument("Evaluated \"" + expression + "\" to be a string, but no closing quote was found");

			returnArgument.type = Argument::TYPE::STRING;
			returnArgument.value = expression.substr(1, expression.size() - 2);
		}
		else
		{
			//At this point it's either misspelled, a number, or a function call.
			//If it's a number or a function call arithmetic operators are allowed, so we need to check for that
			returnArgument = ParseExpression(expression);
		}
	}

	returnArgument.origin = expression;
	return returnArgument;
}

Argument ConsoleCommandManager::ParseExpression(std::string expression)
{
	Argument returnArgument;

	std::queue<std::string> expressionParts = SplitExpression(expression);

	if(expressionParts.size() > 1)
		returnArgument = CalculateExpression(expressionParts);
	else if(std::isalpha(expression[0]))
	{
		if(expression == "true" || expression == "TRUE")
		{
			returnArgument.type = Argument::TYPE::BOOL;
			returnArgument.value = "1";
		}
		else if(expression == "false" || expression == "FALSE")
		{
			returnArgument.type = Argument::TYPE::BOOL;
			returnArgument.value = "0";
		}

		if(expression.back() == ',')
			expression.pop_back();

		returnArgument = ExecuteArgumentFunction(expression);
	}
	else
	{
		//int32/int64/uint64/float/double
		std::stringstream sstream(expression);

		if(expression.back() == 'f')
		{
			if(TryExtract<float>(sstream))
			{
				returnArgument.type = Argument::TYPE::FLOAT;
				returnArgument.value = sstream.str();
				//returnArgument.values.emplace_back(expression.substr(0, expression.size() - 1));
			}
			else
				throw std::invalid_argument("Evaluated \"" + expression + "\" to be a float, but extraction failed");
		}
		else if(expression.find('.') != expression.npos)
		{
			if(std::count(expression.begin(), expression.end(), '.') == 1)
			{
				if(TryExtract<double>(sstream))
				{
					returnArgument.type = Argument::TYPE::DOUBLE;
					returnArgument.value = sstream.str();
				}
				else
					throw std::invalid_argument("Evaluated \"" + expression + "\" to be a double, but extraction failed");
			}
			else
				throw std::invalid_argument("Evaluated \"" + expression + "\" to be a double, but more than one decimal point was found");
		}
		else
		{
			for(char character : expression)
			{
				if(!std::isdigit(character)
					&& character != '-'
					&& character != '+')
					throw std::invalid_argument("Evaluated \"" + expression + "\" to be an integer, but " + character + " was not a number or a +/-");
			}

			if(TryExtract<int32_t>(sstream))
				returnArgument.type = Argument::TYPE::INT32;
			else if(TryExtract<int64_t>(sstream))
				returnArgument.type = Argument::TYPE::INT64;
			else if(TryExtract<uint64_t>(sstream))
				returnArgument.type = Argument::TYPE::UINT64;
			else
				throw std::invalid_argument("Evaluated \"" + expression + "\" to be a integer, but extraction failed");

			returnArgument.value = sstream.str();

		}
	}

	returnArgument.origin = expression;
	return returnArgument;
}

Argument ConsoleCommandManager::CalculateExpression(std::queue<std::string>& expressionParts)
{
	//Console_Print(1.0 / 2.0f + 5.0 * 2.0 - 1 * 3 / 5)

	//1.0 / 2.0f + 5.0 * 2.0 - 1 * 3 / 5
	//0.5 + 5.0 * 2.0 - 1 * 3 / 5
	//0.5 + 10.0 - 1 * 3 / 5
	//0.5 + 10.0 - 3 / 5
	//0.5 + 10.0 - 0.6
	//10.5 - 0.6
	//9.9


	//Console_Print(1 * 2 / 3)

	std::stack<Argument> operands;
	std::stack<OPERATORS> operators;

	const static bool execute[5][5] =
	{//     +    -      *      /     NONE
		{ true, true, true, true, false }, // +
		{ true, true, true, true, false }, // -
		{ false, false, true, true, false }, // *
		{ false, false, true, true, false },  // /
		{ false, false, false, false, false }  // NONE
	};

	while(expressionParts.size() > 0)
	{
		std::string part = expressionParts.front();
		expressionParts.pop();

		if(part == "(" || part == ")")
		{
			if(part == "(")
			{
				Argument newOperand = CalculateExpression(expressionParts);

				operands.emplace(std::move(newOperand));
			}
			else
			{
				while(operators.size() > 0)
					ExecuteSingleCalculation(operands, operators);

				return operands.top();
			}
		}
		else
		{
			OPERATORS current = OPERATORS::NONE;

			switch(part[0])
			{
				case '+':
					current = OPERATORS::PLUS;
					break;
				case '-':
					current = OPERATORS::MINUS;
					break;
				case '*':
					current = OPERATORS::MULT;
					break;
				case '/':
					current = OPERATORS::DIV;
					break;
				default:
					break;
			}

			if(current == OPERATORS::NONE)
			{
				//This is either a function call or a number/string
				if(std::isalpha(part[0]))
				{
					//Function call
					//Check for arguments
					if(expressionParts.size() > 0
						&& expressionParts.front() == "(")
					{
						//Function has arguments, so collect all of them and then execute the function
						std::string functionWithArgs = part;

						int depth = 0;

						while(expressionParts.size() > 0)
						{
							std::string currentPart = expressionParts.front();
							expressionParts.pop();

							if(currentPart == "(")
								++depth;
							else if(currentPart == ")")
							{
								--depth;

								if(depth == 0)
								{
									functionWithArgs += currentPart;
									break;
								}
							}

							functionWithArgs += currentPart;
						}

						Argument newArg = ExecuteArgumentFunction(functionWithArgs);
						operands.emplace(std::move(newArg));
					}
					else
					{
						//No arguments given to the function
						Argument newArg = ExecuteArgumentFunction(part + "()");
						operands.emplace(std::move(newArg));
					}
				}
				else
					operands.emplace(std::move(part)); //Number/string
			}
			else
			{
				if(operators.size() > 0)
				{
					if(execute[static_cast<int>(current)][static_cast<int>(operators.top())])
						ExecuteSingleCalculation(operands, operators);
				}

				operators.emplace(current);
			}
		}
	}

	while(operators.size() > 0)
		ExecuteSingleCalculation(operands, operators);

	return operands.top();
}

void ConsoleCommandManager::ExecuteSingleCalculation(std::stack<Argument>& operands, std::stack<OPERATORS>& operators)
{
	OPERATORS opToPerform = operators.top();
	operators.pop();

	Argument rhs;
	if(operands.size() > 0)
	{
		rhs = EvaluateExpression(operands.top().value);
		operands.pop();
	}
	else
	{
		rhs.type = Argument::TYPE::INT32;
		rhs.value = "0";
	}

	Argument lhs;
	if(operands.size() > 0)
	{
		lhs = EvaluateExpression(operands.top().value);
		operands.pop();
	}
	else
	{
		lhs.type = Argument::TYPE::INT32;
		lhs.value = "0";
	}

	Argument result;

	switch(opToPerform)
	{
		case OPERATORS::PLUS:
			result = lhs + rhs;
			break;
		case OPERATORS::MINUS:
			result = lhs - rhs;
			break;
		case OPERATORS::MULT:
			result = lhs * rhs;
			break;
		case OPERATORS::DIV:
			result = lhs / rhs;
			break;
		case OPERATORS::NONE:
			break;
		default:
			//This won't ever happen since it would throw in the previous switch statement
			break;
	}

	operands.push(result);
}

std::vector<std::string> ConsoleCommandManager::SplitArg(std::string args)
{
	std::vector<std::string> returnVector;

	if(args.empty())
		return returnVector;

	bool insideString = false;

	unsigned int lastCharacter = 0;

	int depth = 0;

	std::string currentArgument;

	bool add = true;

	for(int i = 0, end = static_cast<int>(args.size()); i < end; ++i)
	{
		if(args[i] == '"' && lastCharacter != '\\')
			insideString = !insideString;
		else if(!insideString)
		{
			if(args[i] == '(')
			{
				++depth;
			}
			else if(args[i] == ')')
			{
				--depth;
			}
			else if(args[i] == ',')
			{
				if(depth == 0)
				{
					returnVector.emplace_back(std::move(currentArgument));
					currentArgument.clear();
					add = false;
				}
			}
		}

		if(add)
			currentArgument += args[i];
		else
			add = true;

		lastCharacter = static_cast<unsigned int>(args[i]);
	}

	returnVector.emplace_back(currentArgument);

	return returnVector;
}

std::queue<std::string> ConsoleCommandManager::SplitExpression(std::string expression)
{
	const static std::string arithmeticOps = "+-/*()";

	std::queue<std::string> returnVector;

	//2.0f+Print()*2.0f

	//Print()*2.0f
	//)*2.0f

	auto index = expression.find_first_of(arithmeticOps);
	while(index != expression.npos)
	{
		if(index == 0)
		{
			//Value
			returnVector.emplace(expression.substr(0, 1));
			expression.erase(0, 1);
		}
		else
		{
			//Value
			returnVector.emplace(expression.substr(0, index));

			//Op
			returnVector.emplace(expression.substr(index, 1));
			expression.erase(0, index + 1);
		}


		index = expression.find_first_of(arithmeticOps);
	}

	if(!expression.empty())
		returnVector.emplace(expression.substr(0, index));

	return returnVector;
}

std::string ConsoleCommandManager::TrimText(std::string text)
{
	if(text.size() == 0)
		return "";

	text = TrimTextFrontBack(text);

	std::string returnText;

	unsigned int lastCharacter = 0;

	//Remove any spaces between arguments etc.
	bool inside = false; //If the character is within quotes
	for(int i = 0, end = static_cast<int>(text.size()); i < end; ++i)
	{
		auto character = text[i];

		if(character == '"' && lastCharacter != '\\')
		{
			inside = !inside;

			returnText += character;
		}
		else if(std::isspace(character))
		{
			if(inside)
				returnText += character;
		}
		else
			returnText += character;

		lastCharacter = static_cast<unsigned int>(character);
	}

	return returnText;
}

std::string ConsoleCommandManager::TrimTextFrontBack(const std::string& text)
{
	size_t firstNotOf = text.find_first_not_of(" \t");
	if(firstNotOf == text.npos)
		return text;

	size_t lastNotOf = text.find_last_not_of(" \t");

	return text.substr(firstNotOf, lastNotOf - firstNotOf + 1);
}

bool ConsoleCommandManager::VerifyCommandName(const std::string& name) const
{
	if(!isalpha(name[0]))
		return false;

	for(auto character : name)
	{
		if(!isalpha(character)
			&& !isdigit(character)
			&& character != '_')
		{
			return false;
		}
	}

	return true;
}

bool ConsoleCommandManager::VerifyParenthesesBalance(const std::string& text) const
{
	int balance = 0;

	bool insideQuotes = false;
	char lastCharacter = 0;

	for(char character : text)
	{
		if(character == '"' && lastCharacter != '\\')
			insideQuotes = !insideQuotes;
		else if(!insideQuotes)
		{
			if(character == '(')
				++balance;
			else if(character == ')')
				--balance;
		}

		lastCharacter = character;
	}

	return balance == 0;
}
