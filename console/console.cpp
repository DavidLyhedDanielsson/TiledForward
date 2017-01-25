#include <limits>
#include <sstream>
#include <algorithm>
#include <functional>
#include <memory>

#include "button.h"
#include "../window.h"
#include "../input.h"
#include "../contentManager.h"
#include "colors.h"
#include "outlineBackground.h"
#include "emptyBackground.h"
#include "common.h"
#include "outlineBackgroundStyle.h"

#include "console.h"

#include "commandHelp.h"
#include "commandDumpConsole.h"
#include "commandPrint.h"
#include "commandCallMethod.h"
#include "colorBackgroundStyle.h"

const glm::vec4 Console::defaultBackgroundColor = glm::vec4(glm::vec3(COLORS::ivoryblack), 0.5f);
const glm::vec4 Console::defaultSecondaryColor = COLORS::gray88;
const glm::vec4 Console::defaultTextColor = COLORS::white;
const glm::vec4 Console::defaultHighlightColor = defaultBackgroundColor;
const glm::vec4 Console::defaultHighlightBackgroundColor = defaultTextColor;

Console::Console()
	: completeListMode(COMPLETE_LIST_MODE::HISTORY)
	, contextPointers(nullptr)
	, commandManager(&input, &output, &completeList, &autoexecManager, this)
	, actualDraw(false)
	, completeListBackground(nullptr)
	, historyIndex(-1)
	, suggestionIndex(-1)
	, lastMessagesDuration(0)
{
	draw = true;
	update = true;
	this->clip = false;
}

Console::~Console()
{
	if(style.get() != nullptr
		&& !style->autoexecFile.empty())
	{
		try
		{
			autoexecManager.WriteAutoexec(style->autoexecFile);
		}
		catch(std::exception& ex)
		{
			Logger::LogLine(LOG_TYPE::FATAL, "Caught exception in Write: " + std::string(ex.what()));
		}
	}
}

bool Console::Init(ContentManager* contentManager
				   , Rect area
				   , const std::shared_ptr<ConsoleStyle>& style
				   , const std::shared_ptr<GUIBackgroundStyle>& backgroundStyle
				   , bool allowMove
				   , bool updateWhenMoving
				   , bool allowResize
				   , bool updateWhenResizing
				   , MOUSE_BUTTON moveResizeButton /*= MOUSE_BUTTON::LEFT*/
				   , KEY_MODIFIERS moveResizeModifierKeys /*= KEY_MODIFIERS::CONTROL*/
				   , GUI_ANCHOR_POINT anchorPoint /*= GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT*/
				   , glm::vec2 minPosition /*= glm::vec2(std::numeric_limits<float>::min(), std::numeric_limits<float>::min())*/
				   , glm::vec2 maxPosition /*= glm::vec2(std::numeric_limits<float>::max(), std::numeric_limits<float>::max())*/
				   , glm::vec2 minSize /*= glm::vec2(std::numeric_limits<float>::min(), std::numeric_limits<float>::min())*/
				   , glm::vec2 maxSize /*= glm::vec2(std::numeric_limits<float>::max(), std::numeric_limits<float>::max())*/)
{
	return Init(contentManager
				, area.GetMinPosition()
				, area.GetSize()
				, style
				, backgroundStyle
				, allowMove
				, updateWhenMoving
				, allowResize
				, updateWhenResizing
				, moveResizeButton
				, moveResizeModifierKeys
				, anchorPoint
				, minPosition
				, maxPosition
				, minSize
				, maxSize);
}

bool Console::Init(ContentManager* contentManager
					, glm::vec2 position
					, glm::vec2 size
					, const std::shared_ptr<ConsoleStyle>& style
					, const std::shared_ptr<GUIBackgroundStyle>& backgroundStyle
					, bool allowMove
					, bool updateWhenMoving
					, bool allowResize
					, bool updateWhenResizing
					, MOUSE_BUTTON moveResizeButton /*= MOUSE_BUTTON::LEFT*/
					, KEY_MODIFIERS moveResizeModifierKeys /*= KEY_MODIFIERS::CONTROL*/
					, GUI_ANCHOR_POINT anchorPoint /*= GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT*/
					, glm::vec2 minPosition /*= glm::vec2(std::numeric_limits<float>::min(), std::numeric_limits<float>::min())*/
					, glm::vec2 maxPosition /*= glm::vec2(std::numeric_limits<float>::max(), std::numeric_limits<float>::max())*/
					, glm::vec2 minSize /*= glm::vec2(std::numeric_limits<float>::min(), std::numeric_limits<float>::min())*/
					, glm::vec2 maxSize /*= glm::vec2(std::numeric_limits<float>::max(), std::numeric_limits<float>::max())*/)
{
	this->style = style;

	if(this->style->characterSet == nullptr)
	{
		Logger::LogLine(LOG_TYPE::FATAL, "ConsoleStyle->characterSet is nullptr!");
		return false;
	}

	minSize.x = this->style->padding.x * 2.0f + 128; //128 was chosen arbitrarily
	minSize.y = this->style->padding.y * 2.0f + this->style->characterSet->GetLineHeight() * 5 + this->style->inputOutputPadding;

	glm::vec2 newSize = this->style->padding * 2.0f + size;

	if(newSize.x < minSize.x)
		newSize.x = minSize.x;
	if(newSize.y < minSize.y)
		newSize.y = minSize.y;

	area.SetSize(newSize);

	////////////////////////////////////////
	//Output
	////////////////////////////////////////
	Rect outputRect;

	glm::vec2 newPos(position);

	newPos.x += this->style->padding.x;
	newPos.y += this->style->padding.y;

	outputRect.SetPos(newPos);
	outputRect.SetSize(area.GetSize().x - this->style->padding.x * 2.0f
					   , area.GetSize().y - this->style->padding.y * 2.0f - this->style->characterSet->GetLineHeight() - this->style->inputOutputPadding);

	output.Init(outputRect
				, this->style->outputStyle
				, this->style->outputBackgroundStyle);

	output.allowEdit = true;

	////////////////////////////////////////
	//Label
	////////////////////////////////////////
	Rect labelRect;
	labelRect.SetPos(output.GetArea().GetMinPosition().x
					 , output.GetArea().GetMaxPosition().y + this->style->inputOutputPadding);

	std::unique_ptr<GUIBackground> labelBackground = std::make_unique<EmptyBackground>();

	promptLabel.Init(labelRect, this->style->labelStyle, nullptr, this->style->labelText);

	////////////////////////////////////////
	//Input
	////////////////////////////////////////
	Rect inputRect;
	inputRect.SetPos(output.GetArea().GetMinPosition().x + promptLabel.GetArea().GetWidth()
					 , output.GetArea().GetMaxPosition().y + this->style->inputOutputPadding);
	inputRect.SetSize(area.GetWidth() - this->style->padding.x * 2.0f, static_cast<float>(this->style->characterSet->GetLineHeight()));

	input.Init(inputRect
			   , this->style->inputStyle
			   , this->style->inputBackgroundStyle);

	input.SetJumpSeparators(" (),");

	area.SetSize(area.GetWidth(), output.GetArea().GetHeight() + input.GetArea().GetHeight() + this->style->padding.y * 2.0f + this->style->inputOutputPadding);

	////////////////////////////////////////
	//CompleteList
	////////////////////////////////////////
	completeList.Init(Rect::empty
					  , this->style->completeListStyle
					  , this->style->completeListBackgroundStyle);

	this->completeListBackground = completeList.GetBackground();

	UpdateCompleteListArea();
	HideCompleteList();

	manager.AddContainer(&input);
	manager.AddContainer(&output);
	manager.AddContainer(&completeList);

	std::string helpCommandName = this->style->preferLowercaseFunctions ? "console_help" : "Console_Help";
	CommandHelp* commandHelp = new CommandHelp(helpCommandName);
	if(!AddCommand(commandHelp))
		delete commandHelp;

	std::string printCommandName = this->style->preferLowercaseFunctions ? "print" : "Print";
	CommandPrint* commandPrint = new CommandPrint(printCommandName);
	if(!AddCommand(commandPrint))
		delete commandPrint;

	std::string dumpCommandName = this->style->preferLowercaseFunctions ? "console_dump" : "Console_Dump";
	CommandDumpConsole* commandDumpConsole = new CommandDumpConsole(dumpCommandName);
	if(!AddCommand(commandDumpConsole))
		delete commandDumpConsole;

	if(this->style->autoexecFile != "")
	{
		std::string pauseAutoexecName = this->style->preferLowercaseFunctions ? "console_pauseAutoexec" : "Console_PauseAutoexec";
		CommandCallMethod* PauseAutoexec = new CommandCallMethod(pauseAutoexecName, std::bind(&Console::PauseAutoexecInternal, this, std::placeholders::_1), FORCE_STRING_ARGUMENTS::PER_ARGUMENT);
		if(!AddCommand(PauseAutoexec))
			delete PauseAutoexec;

		std::string resumeAutoexecName = this->style->preferLowercaseFunctions ? "console_resumeAutoexec" : "Console_ResumeAutoexec";
		CommandCallMethod* ResumeAutoexec = new CommandCallMethod(resumeAutoexecName, std::bind(&Console::ResumeAutoexecInternal, this, std::placeholders::_1), FORCE_STRING_ARGUMENTS::PER_ARGUMENT);
		if(!AddCommand(ResumeAutoexec))
			delete ResumeAutoexec;

		std::string applyPausedAutoexecWatchesName = this->style->preferLowercaseFunctions ? "console_applyPausedAutoexecWatches" : "Console_ApplyPausedAutoexecWatches";
		CommandCallMethod* applyPausedAutoexecWatches = new CommandCallMethod(applyPausedAutoexecWatchesName, std::bind(&Console::ApplyPausedAutoexecWatchesInteral, this, std::placeholders::_1), FORCE_STRING_ARGUMENTS::PER_ARGUMENT);
		if(!AddCommand(applyPausedAutoexecWatches))
			delete applyPausedAutoexecWatches;

		std::string addAutoexecWatchName = this->style->preferLowercaseFunctions ? "console_addAutoexecWatch" : "Console_AddAutoexecWatch";
		CommandCallMethod* addAutoexecWatch = new CommandCallMethod(addAutoexecWatchName, std::bind(&Console::AddAutoexecWatchInternal, this, std::placeholders::_1), FORCE_STRING_ARGUMENTS::PER_ARGUMENT);
		if(!AddCommand(addAutoexecWatch))
			delete addAutoexecWatch;

		std::string removeAutoexecWatchName = this->style->preferLowercaseFunctions ? "console_removeAutoexecWatch" : "Console_RemoveAutoexecWatch";
		CommandCallMethod* RemoveAutoexecWatch = new CommandCallMethod(removeAutoexecWatchName, std::bind(&Console::RemoveAutoexecWatchInternal, this, std::placeholders::_1), FORCE_STRING_ARGUMENTS::PER_ARGUMENT);
		if(!AddCommand(RemoveAutoexecWatch))
			delete RemoveAutoexecWatch;

		std::string printAutoexecWatchesName = this->style->preferLowercaseFunctions ? "console_printAutoexecWatches" : "Console_PrintAutoexecWatches";
		CommandCallMethod* PrintAutoexecWatches = new CommandCallMethod(printAutoexecWatchesName, std::bind(&Console::PrintAutoexecWatchesInternal, this, std::placeholders::_1), FORCE_STRING_ARGUMENTS::PER_ARGUMENT);
		if(!AddCommand(PrintAutoexecWatches))
			delete PrintAutoexecWatches;
	}

	////////////////////////////////////////
	//Last messages
	////////////////////////////////////////
	if(this->style->lastMessagesStyle != nullptr)
	{
		lastMessages.Init(Rect(0.0f, 0.0f
							   , static_cast<float>(this->style->lastMessagesTextFieldWidth)
							   , static_cast<float>(this->style->characterSet->GetLineHeight() * this->style->lastMessagesToDraw))
						  , this->style->lastMessagesStyle
						  , this->style->lastMessagesBackgroundStyle);

		if(this->style->lastMessagesDuration < 0)
			this->style->lastMessagesDuration = 0;
	}

	autoexecManager.Init(&commandManager);

	if(!GUIWidgetStyled::Init(contentManager
							, area.GetMinPosition()
							, area.GetSize()
							, backgroundStyle
							, allowMove
							, updateWhenMoving
							, allowResize
							, updateWhenResizing
							, moveResizeButton
							, moveResizeModifierKeys
							, anchorPoint
							, minPosition
							, maxPosition
							, minSize
							, maxSize
	))
		return false;

	return true;
}

void Console::SubUpdate(std::chrono::nanoseconds delta)
{
	manager.Update(delta);

	if(Input::MouseMoved())
		completeList.SetIgnoreMouse(false);

	if(!actualDraw)
	{
		if(style->lastMessagesDuration > 0)
		{
			float ms = delta.count() * 1e-6f;

			lastMessagesDuration -= ms;

			if(lastMessagesDuration < 0.0f)
				lastMessagesDuration = 0.0f;
		}
	}
	else
		lastMessagesDuration = 0.0f;
}

void Console::SubDraw(SpriteRenderer* spriteRenderer)
{
	if(actualDraw)
	{
		background->Draw(spriteRenderer);

		output.Draw(spriteRenderer);

		input.Draw(spriteRenderer);

		promptLabel.Draw(spriteRenderer);

		if(completeList.GetDraw())
			completeList.Draw(spriteRenderer);
	}
	else
	{
		//TODO: Add this back after game test
		if(lastMessagesDuration > 0.0f)
			lastMessages.Draw(spriteRenderer);
	}
}

std::string Console::ExecuteCommand(const std::string& command)
{
	try
	{
		auto commandResult = commandManager.ExecuteCommand(command);

		autoexecManager.FunctionExecuted(std::get<1>(commandResult), std::get<2>(commandResult));

		return std::get<0>(commandResult);
	}
	catch(std::invalid_argument& ex)
	{
		return ex.what();
	}
}

bool Console::AddCommand(ConsoleCommand* command)
{
	std::string errorString = commandManager.AddCommand(command);

	if(!errorString.empty())
		AddText(errorString);

	return errorString.empty();
}

bool Console::RemoveCommand(const std::string& commandName)
{
	return commandManager.RemoveCommand(commandName);
}

void Console::AddText(const std::string& text)
{
	output.AddText(text);

	if(style->lastMessagesStyle != nullptr)
	{
		int linesBefore = lastMessages.GetLineCount();

		lastMessages.AddText(text);

		int linesAfter = lastMessages.GetLineCount();

		lastMessagesTexts.push(linesAfter - linesBefore);

		if(lastMessagesTexts.size() > style->lastMessagesToDraw)
		{
			lastMessages.EraseLines(0, lastMessagesTexts.front());
			lastMessagesTexts.pop();
		}

		if(style->lastMessagesDuration == 0)
			lastMessagesDuration = 1.0f;
		else
			lastMessagesDuration = static_cast<float>(style->lastMessagesDuration);
	}
}

void Console::SetPosition(float x, float y, GUI_ANCHOR_POINT anchorPoint)
{
	GUIWidgetStyled::SetPosition(x, y, anchorPoint);

	output.SetPosition(x + style->padding.x, y + style->padding.y);
	input.SetPosition(x + style->padding.x, output.GetArea().GetMaxPosition().y + this->style->inputOutputPadding);

	UpdateCompleteListPosition();
}

void Console::SetSize(float x, float y, GUI_ANCHOR_POINT anchorPoint)
{
	GUIWidgetStyled::SetSize(x, y, anchorPoint);

	HideCompleteList();

	output.SetSize(x - style->padding.x * 2.0f, y - style->padding.y * 2.0f - input.GetSize().y - style->inputOutputPadding);
	input.SetSize(x - style->padding.x * 2.0f, input.GetArea().GetHeight());

	promptLabel.SetPosition(output.GetArea().GetMinPosition().x, output.GetArea().GetMaxPosition().y + style->inputOutputPadding);
	input.SetPosition(promptLabel.GetArea().GetMaxPosition().x, promptLabel.GetArea().GetMinPosition().y);
}

void Console::SetDraw(bool draw)
{
	//GUIContainer::SetDraw(draw);

	actualDraw = draw;

	if(actualDraw)
	{
		input.Activate();
		output.Deactivate();
	}
}

void Console::Activate()
{
	SetDraw(true);

	receiveAllEvents = true;
}

void Console::Deactivate()
{
	SetDraw(false);

	receiveAllEvents = false;
}

bool Console::GetActive() const
{
	return receiveAllEvents;
}

void Console::Autoexec()
{
	if(style->autoexecFile == "")
	{
		AddText("No autoexec file set");
		return;
	}

	if(autoexecManager.ParseAutoexec(style->autoexecFile, commandManager))
		AddText("Done parsing autoexec");
	else
		AddText("No autoexec found");
}

bool Console::AddAutoexecWatch(const std::string& command)
{
	return autoexecManager.AddAutoexecWatch(command);
}

bool Console::RemoveAutoexecWatch(const std::string& command)
{
	return autoexecManager.RemoveAutoexecWatch(command);
}

void Console::PrintAutoexecWatches()
{
	auto watches = autoexecManager.GetWatches();

	for(const auto& pair : watches)
		AddText(pair.first + "(" + pair.second + ")");
}

bool Console::OnMouseEnter()
{
	if(actualDraw)
	{
		receiveAllEvents = true;
		return true;
	}

	return false;
}

void Console::OnMouseExit()
{
	//Yes, this should be empty
}

bool Console::SubOnMouseDown(const MouseButtonState& keyState, const glm::vec2& mousePosition)
{
	if(area.Contains(mousePosition) || completeList.GetArea().Contains(mousePosition))
	{
		if(completeList.GetDraw()
			&& completeList.GetArea().Contains(mousePosition))
		{
			completeList.OnMouseDown(keyState, mousePosition);

			//Logger::LogLine(LOG_TYPE::INFO_NOWRITE, static_cast<Button*>(completeList.GetHighlitElement())->GetText());
		}
		else
			HideCompleteList();

		if(output.GetReceiveAllEvents())
			output.OnMouseDown(keyState, mousePosition);
		if(input.GetReceiveAllEvents())
			input.OnMouseDown(keyState, mousePosition);

		return true;
	}

	return false;
}

void Console::SubOnMouseUp(const MouseButtonState& keyState, const glm::vec2& mousePosition)
{
	if(completeList.GetDraw()
	   && completeList.GetArea().Contains(mousePosition))
	{
		if(!completeList.GetIsScrolling())
		{
			Button* button = static_cast<Button*>(completeList.GetHighlitElement());

			AcceptText(button->GetText());
			HideCompleteList();
			input.Activate();
		}

		completeList.OnMouseUp(keyState, mousePosition);
	}
	else
	{
		if(completeList.GetDraw())
			completeList.OnMouseUp(keyState, mousePosition);
		if(output.GetReceiveAllEvents())
			output.OnMouseUp(keyState, mousePosition);
		if(input.GetReceiveAllEvents())
			input.OnMouseUp(keyState, mousePosition);
	}
}

bool Console::SubOnKeyDown(const KeyState& keyState)
{
	std::string beforeEvent = input.GetText();

	bool returnValue = false;

	if(output.GetReceiveAllEvents())
		returnValue = output.OnKeyDown(keyState);
	if(input.GetReceiveAllEvents() && !returnValue)
		returnValue = input.OnKeyDown(keyState);

	switch(keyState.key)
	{
		case KEY_CODE::UP:
			UpPressed();
			break;
		case KEY_CODE::DOWN:
			DownPressed();
			break;
		case KEY_CODE::LEFT:
		case KEY_CODE::RIGHT:
			HideCompleteList();
			break;
		case KEY_CODE::BACKSPACE:
			if(beforeEvent != input.GetText())
				BackspacePressed();
			break;
		case KEY_CODE::SPACE:
			if(keyState.mods == KEY_MODIFIERS::CONTROL)
			{
				completeList.ClearElements();
				GenerateSuggestions(input.GetText());
				MoveSuggestionButtonsToCompleteList();
				ShowCompleteList();
			}
			break;
		case KEY_CODE::DELETE:
			if(beforeEvent != input.GetText())
				DeletePressed();
			break;
		case KEY_CODE::ENTER:
			EnterPressed();
			break;
		case KEY_CODE::END:
		case KEY_CODE::HOME:
			HideCompleteList();
			break;
		case KEY_CODE::TAB:
			TabPressed();
			break;
		default:
			return returnValue;
	}

	//Switch will return false if need be
	return true;
}

void Console::SubOnKeyUp(const KeyState& keyState)
{
	if(output.GetReceiveAllEvents())
		output.OnKeyUp(keyState);
	if(input.GetReceiveAllEvents())
		input.OnKeyUp(keyState);
	if(completeList.GetReceiveAllEvents())
		completeList.OnKeyUp(keyState);
}

bool Console::OnChar(unsigned int keyCode)
{
	if(input.GetIsActive())
	{
		SwitchCompleteListMode(COMPLETE_LIST_MODE::COMPLETION);

		std::string textBeforeModification = input.GetActiveCharacterBlockText();

		input.OnChar(keyCode);

		std::string textAfterModification = GenerateSuggestionText();

		completeList.ClearElements();

		GenerateSuggestions(input.GetText());

		MoveSuggestionButtonsToCompleteList();
		ShowCompleteList();

		if(completeList.GetReceiveAllEvents())
			completeList.OnChar(keyCode);

		return true;
	}
	else if(output.GetIsActive())
		return output.OnChar(keyCode);

	return false;
}

bool Console::OnScroll(int distance)
{
	distance /= -120;

	if(completeList.GetArea().Contains(Input::GetMousePosition()))
	{
		if(completeList.GetReceiveAllEvents())
			completeList.OnScroll(distance);
	}
	else
	{
		if(output.GetReceiveAllEvents())
			output.OnScroll(distance);
		if(input.GetReceiveAllEvents())
			input.OnScroll(distance);
	}

	return true;
}

void Console::UpPressed()
{
	if(input.GetReceiveAllEvents()
		|| completeList.GetReceiveAllEvents())
	{
		if(completeListMode == COMPLETE_LIST_MODE::HISTORY)
		{
			if(history.size() > 0)
			{
				if(completeList.GetDraw())
					historyIndex = completeList.GetHighlitElementIndex();

				if(historyIndex == -1)
					historyIndex = 0;
				else
				{
					historyIndex--;

					if(historyIndex == -1)
						historyIndex = static_cast<int>(history.size() - 1);
				}

				HighlightCompleteListIndex(historyIndex);

				ShowCompleteList();
			}
			else
				HideCompleteList();
		}
		else
		{
			if(suggestions.size() > 0)
			{
				if(completeList.GetDraw())
					suggestionIndex = completeList.GetHighlitElementIndex();

				if(suggestionIndex == -1)
					suggestionIndex = static_cast<int>(suggestions.size() - 1);
				else
				{
					suggestionIndex--;

					if(suggestionIndex == -1)
						suggestionIndex = static_cast<int>(suggestions.size() - 1);
				}

				HighlightCompleteListIndex(suggestionIndex);
				ShowCompleteList();
			}
			else
				HideCompleteList();
		}
	}
}

void Console::DownPressed()
{
	if(input.GetReceiveAllEvents()
		|| completeList.GetReceiveAllEvents())
	{
		if(completeListMode == COMPLETE_LIST_MODE::HISTORY)
		{
			if(history.size() > 0)
			{
				if(completeList.GetDraw())
					historyIndex = completeList.GetHighlitElementIndex();

				if(historyIndex == -1)
					historyIndex = 0;
				else
				{
					historyIndex++;

					if(historyIndex > static_cast<int>(history.size() - 1))
						historyIndex = 0;
				}

				HighlightCompleteListIndex(historyIndex);
				ShowCompleteList();
			}
			else
				HideCompleteList();
		}
		else
		{
			if(suggestions.size() > 0)
			{
				if(completeList.GetDraw())
					suggestionIndex = completeList.GetHighlitElementIndex();

				if(suggestionIndex == -1)
					suggestionIndex = 0;
				else
				{
					suggestionIndex++;
					suggestionIndex %= suggestions.size();
				}

				HighlightCompleteListIndex(suggestionIndex);
				ShowCompleteList();
			}
			else
				HideCompleteList();
		}
	}
}

void Console::EnterPressed()
{
	if(!input.GetIsEmpty())
	{
		if(historyIndex == -1)
			AddToHistoryIfNeeded(input.GetText());
		else
			MoveToFrontOfHistory(historyIndex);

		std::string text = input.GetText();

		try
		{
			if(text.find_first_not_of(' ') != text.npos)
			{
				std::string newText = ExecuteCommand(text);

				if(!newText.empty())
					AddText(newText);
			}
		}
		catch(std::invalid_argument& ex)
		{
			AddText(ex.what());
		}
		catch(std::exception& ex)
		{
			AddText("An unknown exception was caught when trying to execute\"" + text + "\": " + std::string(ex.what()));
		}
			
		unsigned int linesAfter = output.GetLineCount();

		if(linesAfter > style->maxLines
			&& !style->dumpFile.empty())
		{
			std::ofstream out(style->dumpFile, std::ofstream::app);

			std::vector<std::string> lineRange = output.GetLines(0, linesAfter - style->maxLines);
			output.EraseLines(0, linesAfter - style->maxLines);

			for(auto line : lineRange)
				out << line << '\n';
		}

		input.SetText("");

		//Doesn't directly call SwitchCompleteListModes but the end result is the same.
		//Keep these comments in case someone searches for SwitchCompleteList usages
		//SwitchCompleteListMode(COMPLETE_LIST_MODE::HISTORY);
		completeListMode = COMPLETE_LIST_MODE::HISTORY;

		HideCompleteList();
		MoveHistoryButtonsToCompleteList();

		historyIndex = -1;
		suggestionIndex = -1;
	}
}

void Console::BackspacePressed()
{
	if(input.GetIsEmpty())
	{
		SwitchCompleteListMode(COMPLETE_LIST_MODE::HISTORY);

		HideCompleteList();
	}
	else
	{
		SwitchCompleteListMode(COMPLETE_LIST_MODE::COMPLETION);

		GenerateSuggestions(input.GetText());
		MoveSuggestionButtonsToCompleteList();
		ShowCompleteList();
	}
}

void Console::DeletePressed()
{
	if(input.GetIsEmpty())
	{
		SwitchCompleteListMode(COMPLETE_LIST_MODE::HISTORY);

		HideCompleteList();
	}
	else
		SwitchCompleteListMode(COMPLETE_LIST_MODE::COMPLETION);

	GenerateSuggestions(input.GetText());
	MoveSuggestionButtonsToCompleteList();
	ShowCompleteList();
}

void Console::TabPressed()
{
	if(completeList.GetDraw()
		&& completeList.GetElementsSize() > 0)
	{
		if(historyIndex != -1
			|| suggestionIndex != -1)
			HighlightCompleteListIndex(std::max(historyIndex, suggestionIndex));
		else
			HighlightCompleteListIndex(0);
	}
}

void Console::AddToHistoryIfNeeded(const std::string& text)
{
	auto iter = std::find(history.begin(), history.end(), text);
	if(iter != history.end())
		MoveToFrontOfHistory(iter);
	else
		AddToHistory(text);
}

void Console::AddToHistory(const std::string& text)
{
	if(history.size() == style->historySize)
	{
		history.pop_back();

		history.emplace_front(text);

		if(style->historySize > 0)
		{
			Button* lastButton = static_cast<Button*>(historyButtons.back().release());

			lastButton->SetText(text);

			historyButtons.pop_back();
			historyButtons.insert(historyButtons.begin(), std::unique_ptr<Button>(lastButton));
		}
	}
	else
	{
		history.emplace_front(text);

		Button* button = new Button;

		button->Init(Rect(glm::vec2(), glm::vec2(completeListBackground->GetWorkArea().GetWidth(), static_cast<float>(this->style->characterSet->GetLineHeight())))
					 , style->completeListButtonStyle
					 , style->completeListButtonBackgroundStyle
					 , nullptr
					 , history.front());

		historyButtons.insert(historyButtons.begin(), std::unique_ptr<Button>(button));
	}
}

void Console::MoveToFrontOfHistory(int index)
{
	std::string newFront = history[index];

	history.erase(history.begin() + index);
	history.emplace_front(newFront);

	std::unique_ptr<GUIContainer> newFirst = std::unique_ptr<GUIContainer>(historyButtons[index].release());

	historyButtons.erase(historyButtons.begin() + index);
	historyButtons.insert(historyButtons.begin(), std::move(newFirst));
}

void Console::MoveToFrontOfHistory(std::deque<std::string>::iterator iter)
{
	MoveToFrontOfHistory(static_cast<int>(std::distance(history.begin(), iter)));
}

void Console::MoveHistoryButtonsToCompleteList()
{
	completeList.ClearElements();

	std::vector<GUIContainer*> completeListElements;
	completeListElements.reserve(historyButtons.size());

	for(const auto& pointer : historyButtons)
		completeListElements.push_back(pointer.get());

	UpdateCompleteListArea();
	completeList.SetElements(std::move(completeListElements));
}

void Console::MoveSuggestionButtonsToCompleteList()
{
	completeList.ClearElements();

	std::vector<GUIContainer*> completeListElements;
	completeListElements.reserve(suggestionButtons.size());

	for(const auto& pointer : suggestionButtons)
		completeListElements.push_back(pointer.get());

	UpdateCompleteListArea();
	completeList.SetElements(std::move(completeListElements));
}

void Console::HideCompleteList()
{
	completeList.SetReceiveAllEvents(false);
	completeList.SetDraw(false);
}

void Console::ShowCompleteList()
{
	completeList.SetReceiveAllEvents(true);
	completeList.SetDraw(true);
}

void Console::UpdateCompleteListArea()
{
	UpdateCompleteListSize();
	UpdateCompleteListPosition();
}

void Console::UpdateCompleteListPosition()
{
	Rect newArea = completeList.GetArea();
	Rect compListArea = completeList.GetArea();

	float newHeight = -1.0f;

	glm::vec2 windowSize = glm::vec2(Input::GetListenWindow()->GetResolutionX(), Input::GetListenWindow()->GetResolutionY());
	glm::vec2 newPosition = glm::vec2(input.GetArea().GetMinPosition().x, input.GetArea().GetMaxPosition().y);
	newArea.SetPos(newPosition);

	if(newArea.GetMinPosition().x + compListArea.GetWidth() > windowSize.x)
	{
		newPosition.x -= newArea.GetMaxPosition().x - windowSize.x;

		if(newPosition.x < 0)
			newPosition.x = 0;
	}

	OutlineBackgroundStyle* outlineStyle = dynamic_cast<OutlineBackgroundStyle*>(style->completeListBackgroundStyle.get());
	OutlineBackground* outlineBackground = dynamic_cast<OutlineBackground*>(completeListBackground);
	if(outlineStyle != nullptr && completeListBackground != nullptr)
	{
		outlineStyle->outlineSides = static_cast<DIRECTIONS>(DIRECTIONS::BOTTOM | DIRECTIONS::LEFT | DIRECTIONS::RIGHT);
		outlineBackground->UpdateOutlineRect();
	}

	if(newArea.GetMinPosition().y + compListArea.GetHeight() > windowSize.y)
	{
		//Put list on top of box if there is room.
		//Othwerwise cap height
		if(input.GetArea().GetMinPosition().y - compListArea.GetHeight() >= 0)
		{
			newPosition.y = input.GetArea().GetMinPosition().y - compListArea.GetHeight();

			if(outlineStyle != nullptr && completeListBackground != nullptr)
			{
				outlineStyle->outlineSides = DIRECTIONS::TOP | DIRECTIONS::LEFT | DIRECTIONS::RIGHT;
				outlineBackground->UpdateOutlineRect();
			}
		}
		else
		{
			newHeight = windowSize.y - newPosition.y;
			newHeight -= static_cast<int>(newHeight) % style->characterSet->GetLineHeight();
		}
	}

	if(newHeight != -1.0f)
		newArea.SetSize(compListArea.GetWidth(), newHeight);

	completeList.SetPosition(newPosition);
}

void Console::UpdateCompleteListSize()
{
	glm::vec2 newSize;

	if(completeListMode == COMPLETE_LIST_MODE::HISTORY)
		newSize.y = static_cast<float>(style->characterSet->GetLineHeight() * (history.size() < style->completeListMaxSize ? static_cast<int>(history.size()) : style->completeListMaxSize));
	else
		newSize.y = static_cast<float>(style->characterSet->GetLineHeight() * (suggestions.size() < style->completeListMaxSize ? static_cast<int>(suggestions.size()) : style->completeListMaxSize));

	newSize.x = 256.0f; //TODO: Auto generate this

	completeList.SetSize(newSize);
}

void Console::SwitchCompleteListMode(COMPLETE_LIST_MODE mode)
{
	historyIndex = -1;
	suggestionIndex = -1;

	if(completeListMode != mode)
	{
		completeList.ClearElements();
		completeListMode = mode;

		if(mode == COMPLETE_LIST_MODE::HISTORY)
			MoveHistoryButtonsToCompleteList();
	}
}

void Console::GenerateSuggestions(const std::string& text)
{
	//Need to clear here since some pointers may be removed
	completeList.ClearElements();

	std::string suggestionText = GenerateSuggestionText();

	auto functionAndArgument = commandManager.ParseFunctionAndArgumentList(text);

	ConsoleCommand* command = commandManager.GetCommand(functionAndArgument.first);
	if(command != nullptr)
	{
		switch(command->GetAutocompleteType())
		{
			case AUTOCOMPLETE_TYPE::NONE:
				return;
			case AUTOCOMPLETE_TYPE::ALL:
			{
				suggestions = command->GetAutocompleteDictionary()->Match(suggestionText);

				auto moreSuggestions = commandManager.Match(suggestionText);
				suggestions.insert(suggestions.end(), moreSuggestions.begin(), moreSuggestions.end());
			}
				break;
			case AUTOCOMPLETE_TYPE::ONLY_CUSTOM:
				suggestions = command->GetAutocompleteDictionary()->Match(suggestionText);
				break;
			default:
				break;
		}
	}
	else
		suggestions = commandManager.Match(text);

	if(suggestions.size() > 0)
	{
		if(suggestions.size() >= suggestionButtons.size())
		{
			for(auto i = suggestionButtons.size(), end = suggestions.size(); i < end; i++)
			{
				std::unique_ptr<Button> button = std::unique_ptr<Button>(new Button);

				button->Init(Rect(glm::vec2(), glm::vec2(completeListBackground->GetWorkArea().GetWidth(), static_cast<float>(this->style->characterSet->GetLineHeight())))
							 , style->completeListButtonStyle
							 , style->completeListButtonBackgroundStyle
							 , nullptr
							 , "");

				suggestionButtons.push_back(std::move(button));
			}

			for(int i = 0, end = static_cast<int>(suggestions.size()); i < end; i++)
				static_cast<Button*>(suggestionButtons[i].get())->SetText(suggestions[i]);
		}
		else
		{
			//Trim excess strings
			suggestionButtons.erase(suggestionButtons.begin() + suggestions.size(), suggestionButtons.end());

			//Simply change text
			for(int i = 0, end = static_cast<int>(suggestions.size()); i < end; i++)
				static_cast<Button*>(suggestionButtons[i].get())->SetText(suggestions[i]);
		}
	}
	else
		suggestionButtons.clear();
}

void Console::HighlightCompleteListIndex(int index)
{
	if(completeListMode == COMPLETE_LIST_MODE::HISTORY)
		AcceptText(history[index]);
	else
		AcceptText(suggestions[index]);

	completeList.HighlightElement(index);
	completeList.SetIgnoreMouse(true);
}

void Console::AcceptText(std::string newText)
{
	if(completeListMode == COMPLETE_LIST_MODE::HISTORY)
	{
		input.SetText(newText);
		input.SetCursorIndex(static_cast<unsigned int>(newText.size()));
	}
	else
	{
		//Local cursor blockIndex represents the cursor blockIndex of the current active character block
		unsigned int globalCursorIndex = input.GetCursorIndex();
		unsigned int characterBlockCursorIndex = 0;
		std::string currentText = input.GetActiveCharacterBlockText(characterBlockCursorIndex);
		unsigned int localCursorIndex = globalCursorIndex - characterBlockCursorIndex;

		//Construct a string with separators to turn
		//Foo(bar0,bar1) into
		//Foo
		//(
		//bar0
		//,
		//bar1
		//)
		//It is within a character block so there will be no spaces
		std::vector<CharacterBlock> splitText = style->characterSet->Split(currentText.c_str(), "(),", true);

		int currentIndex = 0;
		for(const CharacterBlock& characterBlock : splitText)// TODO: Make sure this works as intended
		{
			if(currentIndex + characterBlock.length >= localCursorIndex)
			{
				localCursorIndex = currentIndex;
				currentText.replace(currentIndex, characterBlock.length, newText);
				break;
			}

			currentIndex += characterBlock.length;
		}

		input.ReplaceActiveCharacterBlockText(currentText);
		input.SetCursorIndex(characterBlockCursorIndex + localCursorIndex + static_cast<unsigned int>(newText.size()));
	}
}

std::shared_ptr<ConsoleStyle>Console::GenerateDoomStyle(ContentManager* contentManager, CharacterSet* characterSet)
{
	auto style = std::make_shared<ConsoleStyle>();

	style->characterSet = characterSet;
	style->padding = glm::vec2(0.0f, 0.0f);
	style->inputOutputPadding = 2.0f;
	style->labelText = ">";

	////////////////////////////////////////
	//Label
	////////////////////////////////////////
	std::shared_ptr<LabelStyle> labelStyle = std::make_shared<LabelStyle>();

	labelStyle->characterSet = style->characterSet;
	labelStyle->textColor = defaultTextColor;

	style->labelStyle = labelStyle;

	////////////////////////////////////////
	//Output
	////////////////////////////////////////
	std::shared_ptr<TextFieldStyle> outputStyle = std::make_shared<TextFieldStyle>();
	outputStyle->textColorNormal = defaultTextColor;
	outputStyle->characterSet = style->characterSet;
	outputStyle->cursorSize = glm::vec2(2.0f, style->characterSet->GetLineHeight());
    outputStyle->textHighlightColor = defaultHighlightBackgroundColor; // TODO

    std::shared_ptr<ScrollbarStyle> scrollbarStyle = std::make_shared<ScrollbarStyle>();

	//Scrollbar
	scrollbarStyle->thumbColor = defaultSecondaryColor;
	scrollbarStyle->thumbWidth = 8;
	scrollbarStyle->thumbMinSize = 16;

	outputStyle->scrollbarBackground = std::make_unique<EmptyBackground>();
	outputStyle->scrollbarBackgroundStyle = nullptr;
	outputStyle->scrollBarStyle = scrollbarStyle;

	style->outputStyle = outputStyle;
	style->outputBackgroundStyle = nullptr;

	////////////////////////////////////////
	//Input
	////////////////////////////////////////
	auto inputStyle = std::make_shared<TextBoxStyle>();
	inputStyle->textColorNormal = defaultTextColor;
	inputStyle->characterSet = style->characterSet;
	inputStyle->cursorSize = glm::vec2(2.0f, style->characterSet->GetLineHeight());
	inputStyle->cursorColorNormal = defaultTextColor;
    inputStyle->textHighlightColor = defaultHighlightBackgroundColor;
    inputStyle->textColorSelected = defaultHighlightColor;

	style->inputStyle = inputStyle;
	style->inputBackgroundStyle = nullptr;

	////////////////////////////////////////
	//Completelist
	////////////////////////////////////////
	auto completeListBackgroundStyle = std::make_shared<ColorBackgroundStyle>();
	completeListBackgroundStyle->colors.push_back(defaultBackgroundColor);

	auto completeListStyle = std::make_shared<ListStyle>();
	completeListStyle->scrollDistance = style->characterSet->GetLineHeight();
	completeListStyle->scrollbarBackgroundStyle = nullptr;
	completeListStyle->scrollbarStyle = scrollbarStyle;

	style->completeListStyle = completeListStyle;
	style->completeListBackgroundStyle = completeListBackgroundStyle;

	auto buttonStyle = std::make_shared<ButtonStyle>();

	buttonStyle->textColors[static_cast<int>(ButtonStyle::BUTTON_STATES::NORMAL)] = defaultTextColor;
	buttonStyle->textColors[static_cast<int>(ButtonStyle::BUTTON_STATES::CLICK)] = defaultBackgroundColor;
	buttonStyle->textColors[static_cast<int>(ButtonStyle::BUTTON_STATES::HOVER)] = defaultBackgroundColor;

	buttonStyle->characterSet = style->characterSet;

	style->completeListButtonStyle = buttonStyle;

	auto buttonBackgroundStyle = std::make_shared<ColorBackgroundStyle>();

	//Normal, Click, Hover
    buttonBackgroundStyle->colors.push_back(COLORS::transparent);
    buttonBackgroundStyle->colors.push_back(defaultTextColor);
    buttonBackgroundStyle->colors.push_back(defaultTextColor);

	style->completeListButtonBackgroundStyle = buttonBackgroundStyle;

	////////////////////////////////////////
	//Last messages
	////////////////////////////////////////
	auto lastMessagesStyle = std::make_shared<TextFieldStyle>();
	lastMessagesStyle->textColorNormal = defaultTextColor;
	lastMessagesStyle->characterSet = style->characterSet;
	lastMessagesStyle->cursorSize = glm::vec2(2.0f, 16.0f);
	lastMessagesStyle->cursorColorNormal = defaultTextColor;

	lastMessagesStyle->scrollbarBackground = std::make_unique<EmptyBackground>();
	lastMessagesStyle->scrollbarBackgroundStyle = nullptr;
	lastMessagesStyle->scrollBarStyle = nullptr;

	style->lastMessagesStyle = lastMessagesStyle;
	style->lastMessagesBackgroundStyle = nullptr;

	style->lastMessagesDuration = 5000;

	return style;
}

std::unique_ptr<GUIBackground> Console::GenerateDoomStyleBackground(ContentManager* contentManager)
{
	return std::make_unique<OutlineBackground>();
}

std::shared_ptr<GUIBackgroundStyle> Console::GenerateDoomStyleBackgroundStyle(ContentManager* contentManager)
{
	auto style = std::make_shared<ColorBackgroundStyle>();

	style->colors.push_back(defaultBackgroundColor);

	return style;
}

std::string Console::GenerateSuggestionText()
{
	unsigned int localCursorIndex = 0;
	std::string text = input.GetActiveCharacterBlockText(localCursorIndex);
	localCursorIndex = input.GetCursorIndex() - localCursorIndex;

	std::vector<CharacterBlock> blocks = this->style->characterSet->Split(text.c_str(), "(),", true);

	int currentIndex = 0;
	for(const CharacterBlock& characterBlock : blocks)
	{
		if(currentIndex + characterBlock.length >= localCursorIndex)
			return text.substr(currentIndex, characterBlock.length);

		currentIndex += characterBlock.length;
	}

	//This shouldn't be reached
	return "";
}

Argument Console::AddAutoexecWatchInternal(const std::vector<Argument>& arguments)
{
	std::string returnString;

	int count = 0;
	for(const auto& argument : arguments)
	{
		if(AddAutoexecWatch(argument.value))
			++count;
		else
			returnString += "Couldn't add \"" + argument.value + "\" to autoexec since there is no such variable\n";
	}

	returnString += "Added " + std::to_string(count) + " variable" + (count > 1 ? "s" : "") + " to autoexec watches"; //Worth ternary

	Argument returnArgument;
	returnString >> returnArgument;
	return returnArgument;
}

Argument Console::RemoveAutoexecWatchInternal(const std::vector<Argument>& arguments)
{
	Argument returnArgument;

	int count = 0;
	for(const auto& argument : arguments)
	{
		if(RemoveAutoexecWatch(argument.value))
			++count;
	}

	"Removed " + std::to_string(count) + " variable" + (count > 1 ? "s" : "") + " from autoexec watches" >> returnArgument;

	return returnArgument;
}

Argument Console::PrintAutoexecWatchesInternal(const std::vector<Argument>& arguments)
{
	PrintAutoexecWatches();

	return Argument();
}

Argument Console::PauseAutoexecInternal(const std::vector<Argument>& arguments)
{
	autoexecManager.PauseAutoexecWatching();

	return "Paused autoexec watching";
}

Argument Console::ResumeAutoexecInternal(const std::vector<Argument>& arguments)
{
	if(arguments.size() == 0)
	{
		autoexecManager.ResumeAutoexecWatching(false);

		return "Resumed autoexec watching";
	}
	else
	{
		bool applyChanges;

		if(!(arguments.front() >> applyChanges))
			return "Couldn't convert first parameter to a bool";

		autoexecManager.ResumeAutoexecWatching(applyChanges);

		return "Resumed autoexec watching and applied all changes";
	}
}

Argument Console::ApplyPausedAutoexecWatchesInteral(const std::vector<Argument>& arguments)
{
	if(arguments.size() == 0)
	{
		autoexecManager.ApplyPausedChanges();

		return "Successfully applied all paused watches";
	}
	else
	{
		std::vector<std::string> watchesToApply;

		for(const Argument& watch : arguments)
			watchesToApply.emplace_back(watch.value);

		std::vector<std::string> nonappliedWatches = autoexecManager.ApplySelectedPausedChanges(watchesToApply);

		if(nonappliedWatches.size() == 0)
			return "Successfully applied " + std::to_string(watchesToApply.size() - nonappliedWatches.size()) + " paused watch" + (watchesToApply.size() > 0 ? "es" : 0);
		else
		{
			std::string returnString = "Couldn't apply " + std::to_string(nonappliedWatches.size()) + " watches: ";

			for(const std::string& name : nonappliedWatches)
				returnString += name + ", ";

			returnString.erase(returnString.size() - 2, 2);

			return returnString;
		}
	}
}
