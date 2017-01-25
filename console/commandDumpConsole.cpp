#include "commandDumpConsole.h"

#include <fstream>

CommandDumpConsole::CommandDumpConsole(const std::string& name) 
	: ConsoleCommand(name, FORCE_STRING_ARGUMENTS::NONE)
{
	
}

Argument CommandDumpConsole::Execute(const ContextPointers* const contextPointers, const std::vector<Argument>& arguments)
{
	Argument returnArgument;

	if(arguments.size() == 0)
		returnArgument = GetUsage();
	else if(arguments.size() == 1)
	{
		std::string outPath = arguments.front().value;

		std::ofstream out(outPath, std::ios_base::app);

		if(out.is_open())
		{
			std::vector<std::string> outputText = contextPointers->output->GetText();
			for(std::string text : outputText)
				out << text << '\n';

			returnArgument = "Dumped lines to \"" + outPath + "\"";
		}
		else
			returnArgument = "Couldn't open file at \"" + outPath + "\"";
	}
	else
	{
		std::string outPath = arguments.front().value;

		std::ofstream out(outPath, std::ios_base::app);

		if(out.is_open())
		{
			for(int i = 1, end = static_cast<int>(arguments.size()); i < end; ++i)
				out << arguments[i].origin + " = " + arguments[i] + "\n";

			returnArgument = "Dumped " + std::to_string(arguments.size() - 1) + " uniforms to \"" + outPath + "\"";
		}
		else
			returnArgument = "Couldn't open file at \"" + outPath + "\"";
	}

	return returnArgument;
}

std::string CommandDumpConsole::GetHelp() const
{
	return "Dumps all current text in the console's output window to the given file";
}

std::string CommandDumpConsole::GetUsage() const
{
	return GetName() + "(<fileName>)\nWhere fileName is the path to the file to append data to.";
}

std::string CommandDumpConsole::GetExample() const
{
	return GetName() + "(\"out.txt\")";
}