#include "textField.h"
#include "colors.h"
#include "outlineBackgroundStyle.h"
#include "outlineBackground.h"

#include "../input.h"
#include "../spriteRenderer.h"

#include "../constructedString.h"

#include <algorithm>
//#include <windows.h>

TextField::TextField()
	: GUIContainerStyled()
	, allowEdit(true)
	, textColor(0.0f, 0.0f, 0.0f, 1.0f)
	, drawCursor(false)
	, mouseDown(false)
	, focus(false)
	, yOffset(0)
	, cursorIndex(0)
	, cursorLineIndex(0)
	, selectionIndex(-1)
	, selectionStartIndex(-1)
	, selectionEndIndex(-1)
	, selectionLineIndex(0)
	, selectionStartLineIndex(-1)
	, selectionEndLineIndex(-1)
	, visibleStringsBegin(0)
	, visibleStringsEnd(0)
	, visibleStringsMax(0)
	, matchWidth(-1)
	, jumpSeparators(" ")
	, jumpToBeforeSeparator(true)
{
	this->clip = true;
}

void TextField::Init(Rect area
					 , const std::shared_ptr<TextFieldStyle>& style
					 , const std::shared_ptr<GUIBackgroundStyle>& backgroundStyle
					 , GUI_ANCHOR_POINT anchorPoint /*= GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT*/)
{
	Init(area.GetMinPosition(), area.GetSize(), style, backgroundStyle, anchorPoint);
}

void TextField::Init(glm::vec2 position
					 , glm::vec2 size
					 , const std::shared_ptr<TextFieldStyle>& style
					 , const std::shared_ptr<GUIBackgroundStyle>& backgroundStyle
					 , GUI_ANCHOR_POINT anchorPoint /*= GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT*/)
{
		this->style = style;

	if(this->style->characterSet == nullptr)
	{
		Logger::LogLine(LOG_TYPE::FATAL, "TextFieldStyle->characterSet == nullptr!");
		return;
	}

	GUIContainerStyled::Init(position
							 , size
							 , style
							 , backgroundStyle
							 , anchorPoint);

	cursorColor = this->style->cursorColorNormal;

	if(this->style->scrollBarStyle != nullptr)
	{
		scrollbar.Init(this->background->GetWorkArea()
					   , this->style->scrollBarStyle
					   , this->style->scrollbarBackgroundStyle
					   , std::bind(&TextField::ScrollbarScrolled, this));
	}
	else
	{
		scrollbar.Init(this->background->GetWorkArea()
					   , std::make_shared<ScrollbarStyle>() //TODO is this a good idea?
					   , this->style->scrollbarBackgroundStyle
					   , std::bind(&TextField::ScrollbarScrolled, this));
	}

	visibleStringsMax = static_cast<int>(ceil(this->background->GetWorkArea().GetHeight() / this->style->characterSet->GetLineHeight()));
	scrollbar.SetVisibleItems(visibleStringsMax);
}

void TextField::Update(std::chrono::nanoseconds delta)
{
	scrollbar.Update(delta);

	if(mouseDown && Input::MouseMoved())
	{
		glm::vec2 mousePosition = Input::GetMousePosition();
		Rect workArea = background->GetWorkArea();

		int lineIndex = (static_cast<int>(((mousePosition.y - workArea.GetMinPosition().y) + yOffset) / style->characterSet->GetLineHeight() + visibleStringsBegin));

		if(!SelectionMade())
			BeginSelection();

		SetCursorLineIndex(lineIndex);
		SetCursorIndex(GetCursorIndex(static_cast<int>(mousePosition.x - workArea.GetMinPosition().x), cursorLineIndex));
		ExtendSelectionToCursor();

		matchWidth = static_cast<int>(cursorPosition.x - workArea.GetMinPosition().x);
		mouseDown = true;
	}
}

void TextField::Draw(SpriteRenderer* spriteRenderer)
{
	DrawBackground(spriteRenderer);
	DrawMiddle(spriteRenderer);
	DrawForeground(spriteRenderer);

	if(scrollbar.GetDraw())
		scrollbar.Draw(spriteRenderer);
}

void TextField::DrawBackground(SpriteRenderer* spriteRenderer)
{
	background->Draw(spriteRenderer);

	spriteRenderer->EnableScissorTest(background->GetWorkArea());

	glm::vec2 originalDrawPos = background->GetWorkArea().GetMinPosition();

	originalDrawPos.y = background->GetWorkArea().GetMinPosition().y - yOffset;

	glm::vec2 drawPos = originalDrawPos;

	for(int i = visibleStringsBegin; i < visibleStringsEnd; ++i)
	{
		//TODO: fix this temp
		std::string temp(std::get<0>(lines[i]));

		spriteRenderer->DrawString(style->characterSet, temp, drawPos, style->textColorNormal);

		drawPos.y += style->characterSet->GetLineHeight();
	}

	spriteRenderer->DisableScissorTest();
}

void TextField::DrawMiddle(SpriteRenderer* spriteRenderer)
{
	if(SelectionMade())
	{
		glm::vec2 drawPosition = background->GetWorkArea().GetMinPosition();

		drawPosition.y += style->characterSet->GetLineHeight() * static_cast<float>((std::max(selectionStartLineIndex, visibleStringsBegin) - visibleStringsBegin)) - yOffset;

		glm::vec2 drawSize = glm::vec2(0.0f, static_cast<float>(style->characterSet->GetLineHeight()));

		//This is needed to cap the highlight rectangle so it doens't show up above the textField
		if(drawPosition.y < background->GetWorkArea().GetMinPosition().y)
		{
			drawPosition.y = background->GetWorkArea().GetMinPosition().y;
			drawSize.y -= yOffset;
		}

		for(int i = std::max(selectionStartLineIndex, visibleStringsBegin), endI = std::min(selectionEndLineIndex, visibleStringsEnd); i <= endI; ++i)
		{
			drawPosition.x = background->GetWorkArea().GetMinPosition().x;

			int begin = i == selectionStartLineIndex ? selectionStartIndex : 0;
			int end = i == selectionEndLineIndex ? selectionEndIndex : static_cast<int>(std::get<0>(lines[i]).size());

			unsigned int widthAtBegin = style->characterSet->GetWidthAtIndex(std::get<0>(lines[i]).c_str(), begin);
			unsigned int widthAtEnd = style->characterSet->GetWidthAtIndex(std::get<0>(lines[i]).c_str(), end);

			drawPosition.x += static_cast<float>(widthAtBegin);
			drawSize.x = static_cast<float>(widthAtEnd - widthAtBegin);

			//This can probably be done somehow without a branch, but I can't be arsed right now
			if(drawPosition.y + drawSize.y > background->GetWorkArea().GetMaxPosition().y)
				drawSize.y -= drawPosition.y + drawSize.y - background->GetWorkArea().GetMaxPosition().y;

			spriteRenderer->Draw(Rect(drawPosition.x, drawPosition.y, drawSize.x, drawSize.y), style->textHighlightColor);

			drawPosition.y += drawSize.y;

			drawSize = glm::vec2(0.0f, static_cast<float>(style->characterSet->GetLineHeight()));
		}
	}
}


void TextField::DrawForeground(SpriteRenderer* spriteRenderer)
{
	if(drawCursor)
	{
		if(cursorBlinkTimer.GetTime() >= cursorBlinkTime)
		{
			cursorBlinkTimer.Reset();

			cursorColor = (cursorColor == style->cursorColorNormal ? style->cursorColorBlink : style->cursorColorNormal);
		}

		glm::vec2 drawPosition(cursorPosition);

		drawPosition.x += style->cursorOffset.x;
		drawPosition.y += style->cursorOffset.y;

		spriteRenderer->Draw(Rect(drawPosition.x, drawPosition.y, style->cursorSize.x, style->cursorSize.y), cursorColor);
	}
}

void TextField::AddText(std::string text)
{
	if(text.empty())
	{
		lines.emplace_back("", false);
		return;
	}

	std::vector<std::string> newLines;

	auto lnIndex = text.find('\n');
	while(lnIndex != text.npos)
	{
		std::string cutText = text.substr(0, lnIndex);
		newLines.emplace_back(cutText);

		text.erase(text.begin(), text.begin() + cutText.size() + 1);
		lnIndex = text.find('\n');
	}

	newLines.emplace_back(text);

	for(const std::string& newLine : newLines)
	{
		std::vector<std::tuple<std::string, bool>> newLineData = CreateLineData(newLine);
		lines.insert(lines.end(), std::make_move_iterator(newLineData.begin()), std::make_move_iterator(newLineData.end()));
	}

	scrollbar.SetMaxItems(static_cast<int>(lines.size()));

	UpdateVisibleStrings();
}

void TextField::AppendText(const std::string& text)
{
	if(lines.size() > 0)
	{
		std::get<0>(lines.back()) += text;
		UpdateText(static_cast<int>(lines.size() - 1));
	}
	else
		AddText(text);

	cursorLineIndex = static_cast<int>(lines.size() - 1);
	SetCursorIndex(static_cast<int>(std::get<0>(lines.back()).size()));
}

void TextField::InsertText(const std::string& text, int lineIndex, int index)
{
	if(lines.size() == 0)
	{
		AddText(text);
		cursorLineIndex = static_cast<int>(lines.size() - 1);
		SetCursorIndex(static_cast<int>(std::get<0>(lines.back()).size()));
	}
	else
	{
		if(lineIndex > static_cast<int>(lines.size() - 1))
			lineIndex = static_cast<int>(lines.size() - 1);

		if(index > static_cast<int>(std::get<0>(lines[lineIndex]).size()))
			index = static_cast<int>(std::get<0>(lines[lineIndex]).size());

		std::get<0>(lines[lineIndex]).insert(index, text);
		std::string line = std::get<0>(lines[lineIndex]);

		UpdateText(lineIndex);

		if(std::get<0>(lines[lineIndex]).size() >= index + text.size())
		{
			cursorLineIndex = lineIndex;
			SetCursorIndex(index + static_cast<int>(text.size()));
		}
		else
		{
			//If this is the case then the character block was moved to the next line,
			//so the cursor has to be moved
			int lastIndex = -1;
			for(int i = index; i > 1; --i)
			{
				if(line[i] == ' ' && line[i - 1] != ' ')
				{
					lastIndex = i;
					break;
				}
			}

			if(lastIndex == -1)
			{
				if(line[0] == ' ')
					lastIndex = 0;
				else
					lastIndex = index - 1;
			}

			cursorLineIndex = lineIndex + 1;
			SetCursorIndex(index - lastIndex + static_cast<int>(text.size() - 1));
		}
	}
}

std::vector<std::string> TextField::GetText() const
{
	return GetLines(0, static_cast<int>(lines.size()));
}

std::vector<std::string> TextField::GetLines(int begin, int count) const
{
	std::vector<std::string> lineRange;

	if(lines.size() == 0)
		return lineRange;

	if(begin + count > lines.size())
		count = static_cast<int>(lines.size()) - begin;

	lineRange.emplace_back(std::get<0>(lines[begin]));
	++begin;

	for(int i = begin, end = count; i < end; ++i)
	{
		if(!std::get<1>(lines[i - 1]))
			lineRange.emplace_back("");

		lineRange.back() += std::get<0>(lines[i]);
	}

	return lineRange;
}

std::string TextField::GetLine(int stringIndex) const
{
	return std::get<0>(lines[stringIndex]);
}

std::string TextField::GetSelectedText() const
{
	if(!SelectionMade())
		return "";

	if(selectionStartLineIndex == selectionEndLineIndex)
	{
		return std::get<0>(lines[cursorLineIndex]).substr(selectionStartIndex, selectionEndIndex - selectionStartIndex);
	}
	else
	{
		std::string returnString;

		for(int i = selectionStartLineIndex;; ++i) //No comparison needed, check last line of loop
		{
			int begin = i == selectionStartLineIndex ? selectionStartIndex : 0;
			int end = i == selectionEndLineIndex ? selectionEndIndex : static_cast<int>(std::get<0>(lines[i]).size());

			returnString += std::get<0>(lines[i]).substr(begin, end - begin);

			if(i != selectionEndLineIndex)
			{
				if(!std::get<1>(lines[i]))
					returnString += '\n';
			}
			else
				break;
		}

		return returnString;
	}
}

void TextField::EraseLines(int begin, int count)
{
	std::vector<std::string> lineRange;

	if(begin + count > lines.size() - 1)
		count = static_cast<int>(lines.size() - 1) - begin;

	lines.erase(lines.begin() + begin, lines.begin() + (begin + count));

	scrollbar.SetMaxItems(static_cast<int>(lines.size()));

	UpdateVisibleStrings();
}

int TextField::GetLineCount() const
{
	return static_cast<int>(lines.size());
}

void TextField::SetText(const std::string& text)
{
	Clear();
	AddText(text);
}

void TextField::Clear()
{
	lines.clear();

	scrollbar.SetMaxItems(0);
	cursorLineIndex = 0;

	scrollbar.ScrollTo(cursorLineIndex);
	UpdateVisibleStrings();

	cursorIndex = 0;
	cursorPosition.x = background->GetWorkArea().GetMinPosition().x;
}

void TextField::Activate()
{
	focus = true;

	cursorBlinkTimer.Reset();
	cursorBlinkTimer.Start();

	cursorColor = style->cursorColorNormal;

	drawCursor = true;
	receiveAllEvents = true;
}

void TextField::Deactivate()
{
	Deselect();

	focus = false;
	drawCursor = false;
	update = false;
	receiveAllEvents = false;
}

bool TextField::GetIsActive() const
{
	return drawCursor;
}

int TextField::GetCursorIndex(int width, int line) const
{
	if(width <= 0)
		return 0;

	return style->characterSet->GetIndexAtWidth(std::get<0>(lines[line]).c_str(), width);
}

bool TextField::OnKeyDown(const KeyState& keyState)
{
	if(focus)
	{
		cursorBlinkTimer.Reset();
		cursorColor = style->cursorColorNormal;

		switch(keyState.key)
		{
			case KEY_CODE::UP:
				UpPressed(keyState);
				break;
			case KEY_CODE::DOWN:
				DownPressed(keyState);
				break;
			case KEY_CODE::LEFT:
				LeftPressed(keyState);
				break;
			case KEY_CODE::RIGHT:
				RightPressed(keyState);
				break;
			case KEY_CODE::X:
				if(keyState.mods == KEY_MODIFIERS::CONTROL
				   && SelectionMade())
				{
					//glfwSetClipboardString(Input::GetListenWindow(), GetSelectedText().c_str());
//					std::string selectedText = GetSelectedText();
//
//					if(OpenClipboard(Input::GetListenWindow()))
//					{
//						HGLOBAL clipboardBuffer;
//						char* buffer;
//
//						EmptyClipboard();
//
//						clipboardBuffer = GlobalAlloc(GMEM_DDESHARE, selectedText.length() + 1);
//
//						buffer = (char*)GlobalLock(clipboardBuffer);
//						strcpy_s(buffer, selectedText.size(), selectedText.c_str());
//						GlobalUnlock(clipboardBuffer);
//
//						SetClipboardData(CF_TEXT, clipboardBuffer);
//
//						CloseClipboard();
//
//						if(allowEdit)
//							EraseSelection();
//					}
//					else
						Logger::LogLine(LOG_TYPE::WARNING, "Couldn't open clipboard");
				}
				break;
			case KEY_CODE::C:
				if(keyState.mods == KEY_MODIFIERS::CONTROL)
				{
//					std::string selectedText = GetSelectedText();
//
//					if(OpenClipboard(Input::GetListenWindow()))
//					{
//						HGLOBAL clipboardBuffer;
//						char* buffer;
//
//						EmptyClipboard();
//
//						clipboardBuffer = GlobalAlloc(GMEM_DDESHARE, selectedText.size() + 1);
//
//						buffer = (char*)GlobalLock(clipboardBuffer);
//						strcpy_s(buffer, selectedText.size() + 1, selectedText.c_str());
//						GlobalUnlock(clipboardBuffer);
//
//						SetClipboardData(CF_TEXT, clipboardBuffer);
//
//						CloseClipboard();
//					}
//					else
						Logger::LogLine(LOG_TYPE::WARNING, "Couldn't open clipboard");
				}
				break;
			case KEY_CODE::V:
				if(keyState.mods == KEY_MODIFIERS::CONTROL)
				{
					const char* text;

//					if(OpenClipboard(Input::GetListenWindow()))
//					{
//						text = (char*)GetClipboardData(CF_TEXT);
//
//						if(allowEdit)
//						{
//							if(SelectionMade())
//								EraseSelection();
//
//							InsertText(std::string(text), cursorLineIndex, cursorIndex);
//						}
//					}
//					else
						Logger::LogLine(LOG_TYPE::WARNING, "Couldn't open clipboard");
				}
				break;
			case KEY_CODE::A:
				if(lines.size() == 0)
					break;

				if(keyState.mods == KEY_MODIFIERS::CONTROL)
				{
					cursorLineIndex = 0;
					cursorIndex = 0;

					BeginSelection();

					cursorLineIndex = static_cast<unsigned int>(lines.size() - 1);
					SetCursorIndex(static_cast<int>(std::get<0>(lines[cursorLineIndex]).size()));

					ExtendSelectionToCursor();
				}
				break;
			case KEY_CODE::HOME:
				if(lines.size() == 0)
					break;

				HomePressed(keyState);
				break;
			case KEY_CODE::END:
				if(lines.size() == 0)
					break;

				EndPressed(keyState);
				break;
			case KEY_CODE::BACKSPACE:
				BackspacePressed(keyState);
				break;
			case KEY_CODE::DELETE:
				DeletePressed(keyState);
				break;
			default:
				break;
		}

		return true;
	}

	return false;
}

bool TextField::OnMouseEnter()
{
	receiveAllEvents = true;

	return true;
}

void TextField::OnMouseExit()
{
	//Yes, this should be empty
}

bool TextField::OnMouseDown(const MouseButtonState& keyState, const glm::vec2& mousePosition)
{
	if(keyState.button == MOUSE_BUTTON::LEFT)
	{
		if(area.Contains(mousePosition))
		{
			update = true;

			if(scrollbar.Contains(mousePosition))
				scrollbar.OnMouseDown(keyState, mousePosition);
			else
			{
				if(lines.size() > 0)
				{
					matchWidth = -1;

					if(keyState.action == KEY_ACTION::DOWN)
					{
						if(!scrollbar.Contains(mousePosition))
						{
							focus = true;

							cursorBlinkTimer.Reset();
							cursorBlinkTimer.Start();

							cursorColor = style->cursorColorNormal;

							Rect workArea = background->GetWorkArea();

							unsigned int lineIndex = (static_cast<unsigned int>(((mousePosition.y - workArea.GetMinPosition().y) + yOffset) / style->characterSet->GetLineHeight() + visibleStringsBegin));

							if(keyState.mods == KEY_MODIFIERS::SHIFT)
							{
								if(!SelectionMade())
									BeginSelection();

								SetCursorLineIndex(lineIndex);
								SetCursorIndex(GetCursorIndex(static_cast<int>(mousePosition.x - workArea.GetMinPosition().x), cursorLineIndex));
								ExtendSelectionToCursor();
							}
							else
							{
								Deselect();
								SetCursorLineIndex(lineIndex);
								SetCursorIndex(GetCursorIndex(static_cast<int>(mousePosition.x - workArea.GetMinPosition().x), cursorLineIndex));
							}

							matchWidth = static_cast<int>(cursorPosition.x - workArea.GetMinPosition().x);
							mouseDown = true;
						}
						else
						{
							scrollbar.OnMouseDown(keyState, mousePosition);
						}
					}
					else if(keyState.action == KEY_ACTION::REPEAT)
					{
						if(!scrollbar.Contains(mousePosition))
						{
							if(!SelectionMade())
							{
								JumpCursorLeft();
								BeginSelection();
								JumpCursorRight();
								ExtendSelectionToCursor();
							}
						}
					}
				}
				else
				{
					//Do this manually here
					cursorLineIndex = 0;
					scrollbar.ScrollTo(cursorLineIndex);
					UpdateVisibleStrings();

					cursorIndex = 0;
					cursorPosition = background->GetWorkArea().GetMinPosition();
					drawCursor = true;

					focus = true;
					cursorBlinkTimer.Reset();
					cursorBlinkTimer.Start();

					cursorColor = style->cursorColorNormal;
				}
			}

			return true;
		}
		else
			Deactivate();
	}

	return false;
}

void TextField::OnMouseUp(const MouseButtonState& keyState, const glm::vec2& mousePosition)
{
	update = false;

	if(keyState.button == MOUSE_BUTTON::LEFT)
	{
		mouseDown = false;
		scrollbar.OnMouseUp(keyState, mousePosition);

		if(!area.Contains(mousePosition))
		{
			receiveAllEvents = false;
			focus = false;
		}
	}
}

bool TextField::OnChar(unsigned keyCode)
{
	if(allowEdit)
	{
		InsertText(std::string(1, static_cast<char>(keyCode)), cursorLineIndex, cursorIndex);
		return true;
	}

	return false;
}

bool TextField::OnScroll(int distance)
{
	scrollbar.Scroll(distance);

	UpdateVisibleStrings();

	return true;
}

void TextField::SetPosition(const glm::vec2& newPosition)
{
	SetPosition(newPosition.x, newPosition.y);
}

void TextField::SetPosition(float x, float y)
{
	glm::vec2 diff = cursorPosition -  background->GetWorkArea().GetMinPosition();

	GUIContainer::SetPosition(x, y);

	scrollbar.SetPosition(background->GetWorkArea().GetMaxPosition().x, std::move(background)->GetWorkArea().GetMinPosition().y);

	cursorPosition = background->GetWorkArea().GetMinPosition() + diff;
}

void TextField::SetSize(const glm::vec2& newSize)
{
	SetSize(newSize.x, newSize.y);
}

void TextField::SetSize(float x, float y)
{
	Rect workArea = background->GetWorkArea();
	float xDelta = workArea.GetWidth() - x;

	GUIContainer::SetSize(x, y);

	workArea = background->GetWorkArea();

	//TODO: Improve this. Good enough for now
	if(xDelta != 0)
	{
		scrollbar.SetPosition(workArea.GetMaxPosition().x, workArea.GetMinPosition().y);

		std::vector<std::string> text = GetText();
		Clear();
		for(int i = 0, end = static_cast<int>(text.size()); i < end; ++i)
			AddText(std::move(text[i]));

		scrollbar.SetMaxItems(static_cast<int>(lines.size()));
	}

	scrollbar.SetSize(glm::vec2(scrollbar.GetSize().x, y));
	scrollbar.SetVisibleItems(static_cast<int>(ceil(workArea.GetHeight() / style->characterSet->GetLineHeight())));
	UpdateVisibleStrings();
}

void TextField::SetJumpSeparators(const std::string& separators)
{
	jumpSeparators = separators;
}

void TextField::SetJumpToBeforeSeparator(bool jump)
{
	jumpToBeforeSeparator = jump;
}

void TextField::UpdateText(int beginAtLine)
{
	std::string newString;

	int endIndex = beginAtLine;
	for(int i = beginAtLine; std::get<1>(lines[i]); ++i)
	{
		newString += std::get<0>(lines[i]);
		endIndex = i + 1;
	}

	newString += std::get<0>(lines[endIndex]);

	lines.erase(lines.begin() + beginAtLine, lines.begin() + (endIndex + 1));

	auto newLines = CreateLineData(newString);
	lines.insert(lines.begin() + beginAtLine, std::make_move_iterator(newLines.begin()), std::make_move_iterator(newLines.end()));

	scrollbar.SetMaxItems(static_cast<int>(lines.size()));
}

void TextField::UpPressed(const KeyState& keyState)
{
	if(matchWidth == -1)
		matchWidth = static_cast<int>(cursorPosition.x - background->GetWorkArea().GetMinPosition().x);

	switch(keyState.mods)
	{
		case KEY_MODIFIERS::NONE:
			if(SelectionMade())
				Deselect();

			MoveCursorUp();
			scrollbar.ScrollTo(cursorLineIndex);
			break;
		case KEY_MODIFIERS::SHIFT:
			if(!SelectionMade())
				BeginSelection();

			MoveCursorUp();
			scrollbar.ScrollTo(cursorLineIndex);
			ExtendSelectionToCursor();
			break;
		case KEY_MODIFIERS::CONTROL:
			scrollbar.Scroll(-1);
			UpdateVisibleStrings();
			break;
		default:
			break;
	}

}

void TextField::DownPressed(const KeyState& keyState)
{
	if(matchWidth == -1)
		matchWidth = static_cast<int>(cursorPosition.x - background->GetWorkArea().GetMinPosition().x);

	switch(keyState.mods)
	{
		case KEY_MODIFIERS::NONE:
			if(SelectionMade())
				Deselect();

			MoveCursorDown();
			scrollbar.ScrollTo(cursorLineIndex);
			break;
		case KEY_MODIFIERS::SHIFT:
			if(!SelectionMade())
				BeginSelection();

			MoveCursorDown();
			scrollbar.ScrollTo(cursorLineIndex);
			ExtendSelectionToCursor();
			break;
		case KEY_MODIFIERS::CONTROL:
			scrollbar.Scroll(1);
			UpdateVisibleStrings();
			break;
		default:
			break;
	}
}

void TextField::LeftPressed(const KeyState& keyState)
{
	matchWidth = -1;

	switch(keyState.mods)
	{
		case KEY_MODIFIERS::NONE:
			if(!SelectionMade())
				MoveCursorLeft();
			else
			{
				SetCursorLineIndex(selectionStartLineIndex);
				SetCursorIndex(selectionStartIndex);
				Deselect();
			}
			break;
		case KEY_MODIFIERS::SHIFT:
			if(!SelectionMade())
				BeginSelection();

			MoveCursorLeft();
			ExtendSelectionToCursor();
			break;
		case KEY_MODIFIERS::CONTROL:
			if(SelectionMade())
				Deselect();

			JumpCursorLeft();
			break;
		case static_cast<KEY_MODIFIERS>(static_cast<int>(KEY_MODIFIERS::SHIFT) | static_cast<int>(KEY_MODIFIERS::CONTROL)):
			if(!SelectionMade())
				BeginSelection();

			JumpCursorLeft();
			ExtendSelectionToCursor();
			break;
		default:
			break;
	}
}

void TextField::RightPressed(const KeyState& keyState)
{
	matchWidth = -1;

	switch(keyState.mods)
	{
		case KEY_MODIFIERS::NONE:
			if(!SelectionMade())
			{
				MoveCursorRight();
			}
			else
			{
				SetCursorLineIndex(selectionEndLineIndex);

				SetCursorIndex(selectionEndIndex);
				Deselect();
			}
			break;
		case KEY_MODIFIERS::SHIFT:
			if(!SelectionMade())
				BeginSelection();

			MoveCursorRight();
			ExtendSelectionToCursor();
			break;
		case KEY_MODIFIERS::CONTROL:
			if(SelectionMade())
				Deselect();

			JumpCursorRight();
			break;
		case static_cast<KEY_MODIFIERS>(static_cast<int>(KEY_MODIFIERS::SHIFT) | static_cast<int>(KEY_MODIFIERS::CONTROL)) :
			if(!SelectionMade())
				BeginSelection();

			JumpCursorRight();
			ExtendSelectionToCursor();
			break;
		default:
			break;
	}
}

void TextField::BackspacePressed(const KeyState& keyState)
{
	if(allowEdit)
	{
		matchWidth = -1;

		if(keyState.mods == KEY_MODIFIERS::CONTROL)
		{
			BeginSelection();
			JumpCursorLeft();
			ExtendSelectionToCursor();
			EraseSelection();
		}
		else
		{
			if(SelectionMade())
				EraseSelection();
			else
			{
				if(cursorIndex > 0)
				{
					std::get<0>(lines[cursorLineIndex]).erase(cursorIndex - 1, 1);
					UpdateText(cursorLineIndex);

					SetCursorIndex(cursorIndex - 1);
				}
				else if(cursorLineIndex > 0)
				{
					if(std::get<1>(lines[cursorLineIndex - 1]))
						std::get<0>(lines[cursorLineIndex - 1]).pop_back();

					int newCursorIndex = static_cast<int>(std::get<0>(lines[cursorLineIndex - 1]).size());

					std::get<1>(lines[cursorLineIndex - 1]) = true;
					UpdateText(cursorLineIndex - 1);

					if(std::get<0>(lines[cursorLineIndex - 1]).size() >= newCursorIndex)
					{
						--cursorLineIndex;
						SetCursorIndex(newCursorIndex);
					}
					else
						SetCursorIndex(newCursorIndex - static_cast<int>(std::get<0>(lines[cursorLineIndex - 1]).size()));
				}

				if(lines.size() == 1
					&& std::get<0>(lines.back()).empty())
				{
					Clear();
				}
			}
		}
	}
}

void TextField::DeletePressed(const KeyState& keyState)
{
	if(allowEdit)
	{
		matchWidth = -1;

		if(keyState.mods == KEY_MODIFIERS::CONTROL)
		{
			BeginSelection();
			JumpCursorLeft();
			ExtendSelectionToCursor();
			EraseSelection();
		}
		else
		{
			if(SelectionMade())
				EraseSelection();
			else
			{
				if(cursorIndex < std::get<0>(lines[cursorLineIndex]).size())
				{
					std::get<0>(lines[cursorLineIndex]).erase(cursorIndex, 1);
					UpdateText(cursorLineIndex);

					SetCursorIndex(cursorIndex);
				}
				else if(cursorLineIndex < lines.size() - 1)
				{
					if(std::get<1>(lines[cursorLineIndex]))
						std::get<0>(lines[cursorLineIndex + 1]).erase(std::get<0>(lines[cursorLineIndex + 1]).begin());
					else
						std::get<1>(lines[cursorLineIndex]) = true;

					UpdateText(cursorLineIndex);

					std::get<1>(lines[cursorLineIndex]) = true;
					SetCursorIndex(cursorIndex);
				}

				if(lines.size() == 1
					&& std::get<0>(lines.back()).empty())
				{
					Clear();
				}
			}
		}
	}
}

void TextField::HomePressed(const KeyState & keyState)
{
	matchWidth = -1;

	switch(keyState.mods)
	{
		case KEY_MODIFIERS::NONE:
			Deselect();
			SetCursorIndex(0);
			break;
		case KEY_MODIFIERS::SHIFT:
			if(!SelectionMade())
				BeginSelection();

			SetCursorIndex(0);
			ExtendSelectionToCursor();
			break;
		case KEY_MODIFIERS::CONTROL:
			cursorLineIndex = 0;
			SetCursorIndex(0);
			break;
		case static_cast<KEY_MODIFIERS>(static_cast<int>(KEY_MODIFIERS::SHIFT) | static_cast<int>(KEY_MODIFIERS::CONTROL)) :
			if(!SelectionMade())
				BeginSelection();

			cursorLineIndex = 0;
			SetCursorIndex(0);
			ExtendSelectionToCursor();
			break;
		default:
			break;
	}
}

void TextField::EndPressed(const KeyState & keyState)
{
	matchWidth = -1;

	switch(keyState.mods)
	{
		case KEY_MODIFIERS::NONE:
			Deselect();
			SetCursorIndex(static_cast<int>(std::get<0>(lines[cursorLineIndex]).size()));
			break;
		case KEY_MODIFIERS::SHIFT:
			if(!SelectionMade())
				BeginSelection();

			SetCursorIndex(static_cast<int>(std::get<0>(lines[cursorLineIndex]).size()));
			ExtendSelectionToCursor();
			break;
		case KEY_MODIFIERS::CONTROL:
			cursorLineIndex = static_cast<int>(lines.size() - 1);
			SetCursorIndex(static_cast<int>(std::get<0>(lines[cursorLineIndex]).size()));
			break;
		case static_cast<KEY_MODIFIERS>(static_cast<int>(KEY_MODIFIERS::SHIFT) | static_cast<int>(KEY_MODIFIERS::CONTROL)) :
			if(!SelectionMade())
				BeginSelection();

			cursorLineIndex = static_cast<int>(lines.size() - 1);
			SetCursorIndex(static_cast<int>(std::get<0>(lines[cursorLineIndex]).size()));
			ExtendSelectionToCursor();
			break;
		default:
			break;
	}
}

void TextField::SetCursorIndex(int newIndex)
{
	//Unsigned int means there's no need to check values below 0
	scrollbar.ScrollTo(cursorLineIndex);
	UpdateVisibleStrings();

	if(newIndex > std::get<0>(lines[cursorLineIndex]).size())
		newIndex = static_cast<int>(std::get<0>(lines[cursorLineIndex]).size());
	else if(newIndex < 0)
		newIndex = 0;

	cursorIndex = newIndex;
	cursorPosition.x = background->GetWorkArea().GetMinPosition().x + style->characterSet->GetWidthAtIndex(std::get<0>(lines[cursorLineIndex]).c_str(), cursorIndex);
}

void TextField::SetCursorLineIndex(int newIndex)
{
	if(newIndex >= static_cast<int>(lines.size()))
		newIndex = static_cast<int>(lines.size() - 1);
	else if(newIndex < 0)
		newIndex = 0;

	cursorLineIndex = newIndex;

	scrollbar.ScrollTo(newIndex);
	UpdateVisibleStrings();
}

void TextField::MoveCursorUp()
{
	if(cursorLineIndex > 0)
	{
		cursorLineIndex--;

		if(cursorLineIndex < visibleStringsBegin)
		{
			scrollbar.Scroll(-1);

			UpdateVisibleStrings();
		}

		SetCursorIndex(style->characterSet->GetIndexAtWidth(std::get<0>(lines[cursorLineIndex]).c_str(), matchWidth));
	}
}

void TextField::MoveCursorDown()
{
	if(cursorLineIndex < lines.size() - 1)
	{
		cursorLineIndex++;

		if(cursorLineIndex >= visibleStringsEnd)
		{
			scrollbar.Scroll(1);

			UpdateVisibleStrings();
		}

		SetCursorIndex(style->characterSet->GetIndexAtWidth(std::get<0>(lines[cursorLineIndex]).c_str(), matchWidth));
	}
}

void TextField::MoveCursorLeft()
{
	if(cursorIndex > 0)
		SetCursorIndex(cursorIndex - 1);
	else if(cursorLineIndex > 0)
	{
		int before = cursorLineIndex;

		MoveCursorUp();

		if(before != cursorLineIndex)
			SetCursorIndex(static_cast<int>(std::get<0>(lines[cursorLineIndex]).size()));
	}
}

void TextField::MoveCursorRight()
{
	if(static_cast<unsigned int>(cursorIndex) < std::get<0>(lines[cursorLineIndex]).size())
		SetCursorIndex(cursorIndex + 1);
	else if(cursorLineIndex < lines.size() - 1)
	{
		int before = cursorLineIndex;

		MoveCursorDown();

		if(before != cursorLineIndex)
			SetCursorIndex(0);
	}
}

void TextField::JumpCursorLeft()
{
	//cursorIndex is used "locally" here, so make sure to call SetCursorIndex before returning!
	while(true)
	{
		--cursorIndex;
		if(cursorIndex < 0)
		{
			if(cursorLineIndex > 0)
			{
				--cursorLineIndex;
				if(cursorLineIndex < visibleStringsBegin)
				{
					scrollbar.Scroll(-1);

					UpdateVisibleStrings();
				}

				cursorIndex = static_cast<int>(std::get<0>(lines[cursorLineIndex]).size());
			}
			else
			{
				SetCursorIndex(0);
				return;
			}
		}

		const std::string& line = std::get<0>(lines[cursorLineIndex]);

		//If the cursor is at index 0 and it moved to a NEW line, then cursorIndex will be at the end
		//and line[cursorIndex] will read out of bounds
		if(cursorIndex == line.size())
			--cursorIndex;

		//If it starts at a separator all separators should be ignored until a non-separator is found.
		//e.g. "asdf               asdf" should jump to the beginning if the cursor is in the middle of all the spaces
		//(and jumpSeparators.find(' ') != jumpSeparators.npos)
		bool nonSeparator = jumpSeparators.find(line[cursorIndex]) == jumpSeparators.npos;

		for(--cursorIndex; cursorIndex >= 0; --cursorIndex)
		{
			if(jumpSeparators.find(line[cursorIndex]) != jumpSeparators.npos)
			{
				if(nonSeparator)
				{
					SetCursorIndex(cursorIndex + static_cast<int>(jumpToBeforeSeparator));

					return;
				}
			}
			else
				nonSeparator = true;
		}

		//If this point is reached no separators were found when wandering left from wherever the cursor was at the start.
		//Then either it's at the first row
		if(cursorLineIndex == 0)
		{
			SetCursorIndex(0);
			return;
		}
		//Or it needs to  go up one line and keep looking
		else
		{
			//But only if the line above this is connected to this line
			//and if the last character of the line above isn't a separator
			if(!std::get<1>(lines[cursorLineIndex - 1])
				|| jumpSeparators.find(std::get<0>(lines[cursorLineIndex - 1]).back()) != jumpSeparators.npos)
			{
				SetCursorIndex(0);
				return;
			}
		}
	}
}

void TextField::JumpCursorRight()
{
	//cursorIndex is used "locally" here, so make sure to call SetCursorIndex before returning!
	//See JumpCursorLeft() for more comments
	while(true)
	{
		++cursorIndex;
		if(cursorIndex > std::get<0>(lines[cursorLineIndex]).size())
		{
			if(cursorLineIndex < lines.size() - 1)
			{
				++cursorLineIndex;
				if(cursorLineIndex >= visibleStringsEnd)
				{
					scrollbar.Scroll(1);

					UpdateVisibleStrings();
				}

				cursorIndex = 0;
			}
			else
			{
				SetCursorIndex(static_cast<int>(std::get<0>(lines[cursorLineIndex]).size()));
				return;
			}
		}

		//a b c d e

		const std::string& line = std::get<0>(lines[cursorLineIndex]);

		--cursorIndex;
		bool nonSeparator = jumpSeparators.find(line[cursorIndex]) == jumpSeparators.npos;

		for(++cursorIndex; cursorIndex < line.size(); ++cursorIndex)
		{
			if(jumpSeparators.find(line[cursorIndex]) != jumpSeparators.npos)
			{
				if(nonSeparator)
				{
					SetCursorIndex(cursorIndex + static_cast<int>(!jumpToBeforeSeparator));

					return;
				}
			}
			else
				nonSeparator = true;
		}

		if(cursorLineIndex == lines.size() - 1)
		{
			SetCursorIndex(static_cast<int>(line.size()));
			return;
		}
		else
		{
			if(!std::get<1>(lines[cursorLineIndex])
				|| jumpSeparators.find(std::get<0>(lines[cursorLineIndex + 1]).front()) != jumpSeparators.npos)
			{
				SetCursorIndex(static_cast<int>(line.size()));
				return;
			}
		}
	}
}

void TextField::ExtendSelectionToCursor()
{
	//Make sure selectionEndIndex is always after selectionStartIndex
	//Same thing with selectionEndLineIndex and selectionStartLineIndex5
	if(selectionLineIndex < cursorLineIndex)
	{
		selectionStartIndex = selectionIndex;
		selectionEndIndex = cursorIndex;

		selectionStartLineIndex = selectionLineIndex;
		selectionEndLineIndex = cursorLineIndex;
	}
	else if(selectionLineIndex > cursorLineIndex)
	{
		selectionStartIndex = cursorIndex;
		selectionEndIndex = selectionIndex;

		selectionStartLineIndex = cursorLineIndex;
		selectionEndLineIndex = selectionLineIndex;
	}
	else
	{
		if(cursorIndex > selectionIndex)
		{
			selectionStartIndex = selectionIndex;
			selectionEndIndex = cursorIndex;
		}
		else
		{
			selectionStartIndex = cursorIndex;
			selectionEndIndex = selectionIndex;
		}

		selectionStartLineIndex = cursorLineIndex;
		selectionEndLineIndex = cursorLineIndex;
	}
}

void TextField::BeginSelection()
{
	selectionIndex = cursorIndex;
	selectionLineIndex = cursorLineIndex;

	selectionStartIndex = selectionIndex;
	selectionStartLineIndex = selectionLineIndex;
}

void TextField::Deselect()
{
	selectionEndIndex = -1;
	selectionStartIndex = -1;

	selectionEndLineIndex = -1;
	selectionStartLineIndex = -1;
}

void TextField::EraseSelection()
{
	if(selectionStartLineIndex == selectionEndLineIndex)
	{
		std::get<0>(lines[selectionStartLineIndex]).erase(selectionStartIndex, selectionEndIndex - selectionStartIndex);
		UpdateText(selectionStartLineIndex);
	}
	else
	{
		//The first line will now be connected to the last one, even if it doesn't want to
		std::get<1>(lines[selectionStartLineIndex]) = true;

		std::get<0>(lines[selectionStartLineIndex]).erase(selectionStartIndex, std::get<0>(lines[selectionStartLineIndex]).size());
		std::get<0>(lines[selectionEndLineIndex]).erase(0, selectionEndIndex);

		if(selectionEndLineIndex - selectionStartLineIndex >= 2)
			lines.erase(lines.begin() + (selectionStartLineIndex + 1), lines.begin() + (selectionEndLineIndex));

		UpdateText(selectionStartLineIndex);
	}

	cursorLineIndex = selectionStartLineIndex;
	SetCursorIndex(selectionStartIndex);
	Deselect();

	if(lines.size() == 1
		&& std::get<0>(lines.back()).empty())
	{
		Clear();
	}
}

bool TextField::SelectionMade() const
{
	return selectionStartIndex != -1 && selectionEndIndex != -1;
}

void TextField::UpdateVisibleStrings()
{
	visibleStringsBegin = scrollbar.GetMinIndex();
	visibleStringsEnd = scrollbar.GetMaxIndex();

	if(visibleStringsBegin != 0)
		yOffset = static_cast<int>(this->style->characterSet->GetLineHeight()) - (static_cast<int>(background->GetWorkArea().GetHeight()) % static_cast<int>(this->style->characterSet->GetLineHeight()));
	else
		yOffset = 0;

	if(cursorLineIndex >= visibleStringsBegin && cursorLineIndex < visibleStringsEnd)
	{
		drawCursor = focus;

		if(visibleStringsBegin > 0)
			cursorPosition.y = background->GetWorkArea().GetMaxPosition().y - (visibleStringsEnd - cursorLineIndex) * style->characterSet->GetLineHeight();
		else
			cursorPosition.y = background->GetWorkArea().GetMinPosition().y + (cursorLineIndex - visibleStringsBegin) * style->characterSet->GetLineHeight();
	}
	else
		drawCursor = false;

	scrollbar.SetDraw(style->scrollBarStyle != nullptr 
		&& scrollbar.GetMaxItems() > scrollbar.GetVisibleItems());
}

std::vector<std::tuple<std::string, bool>> TextField::CreateLineData(const std::string& text) const
{
	std::vector<std::tuple<std::string, bool>> returnVector;

	int width = 0;
	int drawStartIndex = 0;
	int drawCount = 0;

	std::vector<CharacterBlock> blocks = style->characterSet->Split(text.c_str());
	for(const CharacterBlock& block : blocks)
	{
		if(width + block.width <= background->GetWorkArea().GetWidth() - scrollbar.GetSize().x - style->scrollBarPadding)
		{
			//Block fits on the current line
			drawCount += block.length + 1;
			width += block.width + style->characterSet->GetSpaceXAdvance();
		}
		else
		{
			if(block.width > background->GetWorkArea().GetWidth() - scrollbar.GetSize().x - style->scrollBarPadding)
			{
				//Block needs to be split into several lines
				for(const Character* character : block.characters)
				{
					if(width + character->xAdvance < background->GetWorkArea().GetWidth() - scrollbar.GetSize().x - style->scrollBarPadding)
					{
						width += character->xAdvance;
						drawCount++;
					}
					else
					{
						returnVector.emplace_back(text.substr(drawStartIndex, drawCount), true);

						width = character->xAdvance;
						drawStartIndex += drawCount;
						drawCount = 1;
					}
				}

				width += style->characterSet->GetSpaceXAdvance();
				drawCount++;
			}
			else
			{
				//Block will fit on a NEW line
				returnVector.emplace_back(text.substr(drawStartIndex, drawCount), true);

				width = block.width + style->characterSet->GetSpaceXAdvance();
				drawStartIndex += drawCount;
				drawCount = block.length + 1;
			}
		}
	}

	returnVector.emplace_back(text.substr(drawStartIndex, drawCount - 1), false); //Skip last space since it's not actually there

	return returnVector;
}

void TextField::ScrollbarScrolled()
{
	UpdateVisibleStrings();
}

std::shared_ptr<TextFieldStyle> TextField::GenerateDefaultStyle(ContentManager* contentManager)
{
	auto style = std::make_shared<TextFieldStyle>();

	style->characterSet = contentManager->Load<CharacterSet>("Calibri16");

	style->textColorNormal = COLORS::black;
	style->textHighlightColor = COLORS::white;

	style->cursorSize = glm::vec2(2.0f, static_cast<float>(style->characterSet->GetLineHeight()));
	style->cursorColorNormal = COLORS::black;
	style->cursorColorBlink = COLORS::black;
	style->cursorColorBlink.w = 0.0f;

	style->scrollBarStyle = Scrollbar::GenerateDefaultStyle(contentManager);
	style->scrollbarBackground = Scrollbar::GenerateDefaultBackground(contentManager);
	style->scrollbarBackgroundStyle = Scrollbar::GenerateDefaultBackgroundStyle(contentManager);

	return style;
}

std::unique_ptr<GUIBackground> TextField::GenerateDefaultBackground(ContentManager* contentManager)
{
	return std::make_unique<OutlineBackground>();
}

std::shared_ptr<GUIBackgroundStyle> TextField::GenerateDefaultBackgroundStyle(ContentManager* contentManager)
{
	auto style = std::make_shared<OutlineBackgroundStyle>();

	style->AddColor(COLORS::snow4, COLORS::snow4);

	return style;
}