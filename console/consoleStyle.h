#ifndef OPENGLWINDOW_CONSOLESTYLE_H
#define OPENGLWINDOW_CONSOLESTYLE_H

#include "guiStyle.h"
#include "buttonStyle.h"

struct ConsoleStyle
	: public GUIStyle
{
	ConsoleStyle()
		: characterSet(nullptr)
		, outputStyle(nullptr)
		, outputBackgroundStyle(nullptr)
		, inputStyle(nullptr)
		, inputBackgroundStyle(nullptr)
		, completeListStyle(nullptr)
		, completeListBackgroundStyle(nullptr)
		, completeListButtonStyle(nullptr)
		, completeListButtonBackgroundStyle(nullptr)
		, inputOutputPadding(0.0f)
		, padding(0.0f, 0.0f)
		, historySize(15)
		, completeListMaxSize(10)
		, preferLowercaseFunctions(false)
		, lastMessagesToDraw(10)
		, lastMessagesDuration(5000)
		, lastMessagesTextFieldWidth(720)
		, lastMessagesStyle(nullptr)
		, lastMessagesBackgroundStyle(nullptr)
		, maxLines(1024)
		, dumpFile("ConsoleDump.txt")
		, autoexecFile("Autoexec")
	{}
	~ConsoleStyle() = default;

	CharacterSet* characterSet;

	std::shared_ptr<TextFieldStyle> outputStyle;
	std::shared_ptr<GUIBackgroundStyle> outputBackgroundStyle;

	std::shared_ptr<TextBoxStyle> inputStyle;
	std::shared_ptr<GUIBackgroundStyle> inputBackgroundStyle;

	std::shared_ptr<ListStyle> completeListStyle;
	std::shared_ptr<GUIBackgroundStyle> completeListBackgroundStyle;

	std::shared_ptr<ButtonStyle> completeListButtonStyle;
	std::shared_ptr<GUIBackgroundStyle> completeListButtonBackgroundStyle;

	std::shared_ptr<LabelStyle> labelStyle;

	float inputOutputPadding;
	glm::vec2 padding;

	int historySize;
	int completeListMaxSize; //In indexes

	bool preferLowercaseFunctions; //Names default functions "help" instead of "Help"

	//If lastMessagesStyle != nullptr, then the last X messages will be drawn even though the console is not open
	int lastMessagesToDraw;
	int lastMessagesDuration; //In milliseconds, 0 for infinite
	int lastMessagesTextFieldWidth;
	std::shared_ptr<TextFieldStyle> lastMessagesStyle;
	std::shared_ptr<GUIBackgroundStyle> lastMessagesBackgroundStyle;

	unsigned int maxLines; //After this many lines the oldest lines will be removed and written to "dumpFile" instead
	std::string dumpFile;

	std::string autoexecFile;

	std::string labelText;
};

#endif //OPENGLWINDOW_CONSOLESTYLE_H
