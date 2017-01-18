#ifndef OPENGLWINDOW_CONTEXTPOINTERS_H
#define OPENGLWINDOW_CONTEXTPOINTERS_H

#include "textBox.h"
#include "textField.h"
#include "list.h"

class Console;
class ConsoleCommandManager;
class ConsoleAutoexecManager;

/**
* Contains pointers to useful objects. Used in ConsoleCommand::Execute
*/
struct ContextPointers
{
	ContextPointers(TextBox* input
		, TextField* output
		, List* completeList
		, ConsoleCommandManager* commandManager
		, ConsoleAutoexecManager* autoexecManager
		, Console* console)
		: input(input)
		, output(output)
		, completeList(completeList)
		, commandManager(commandManager)
		, autoexecManager(autoexecManager)
		, console(console)
	{}
	~ContextPointers() = default;

	TextBox* const input;
	TextField* const output;
	List* const completeList;

	ConsoleCommandManager* const commandManager;
	ConsoleAutoexecManager* const autoexecManager;

	Console* const console;
};

#endif //OPENGLWINDOW_CONTEXTPOINTERS_H
