#include "textBox.h"
#include "colors.h"
#include "outlineBackgroundStyle.h"
#include "outlineBackground.h"

#include "../logger.h"

#include "../os/input.h"
#include "../spriteRenderer.h"
#include <algorithm>

TextBox::TextBox()
	: GUIContainerStyled()
	, drawCursor(false)
	, cursorIndex(0)
	, allowEdit(true)
	, mouseDown(false)
	, selectionIndex(-1)
	, selectionStartIndex(-1)
	, selectionEndIndex(-1)
	, xOffset(0.0f)
	, jumpSeparators(" ")
	, jumpToBeforeSeparator(true)
{
	receiveAllEvents = false;
	update = false;

	this->clip = true;
}

TextBox::~TextBox()
{
}

void TextBox::Init(Rect area
				   , const std::shared_ptr<TextBoxStyle>& style
				   , const std::shared_ptr<GUIBackgroundStyle>& backgroundStyle
				   , GUI_ANCHOR_POINT anchorPoint /*= GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT*/)
{
	Init(area.GetMinPosition(), area.GetSize(), style, backgroundStyle, anchorPoint);
}


void TextBox::Init(glm::vec2 position
				   , glm::vec2 size
				   , const std::shared_ptr<TextBoxStyle>& style
				   , const std::shared_ptr<GUIBackgroundStyle>& backgroundStyle
				   , GUI_ANCHOR_POINT anchorPoint /*= GUI_ANCHOR_POINT::TOP | GUI_ANCHOR_POINT::LEFT*/)
{
		this->style = style;

	if(this->style == nullptr)
		return;
	else if(this->style->characterSet == nullptr)
	{
		Logger::LogLine(LOG_TYPE::FATAL, "TextBoxStyle->characterSet is nullptr!");
		return;
	}

	GUIContainerStyled::Init(position
							 , size
							 , style
							 , backgroundStyle
							 , anchorPoint);

	cursorColor = this->style->cursorColorNormal;

	SetText("");
}

void TextBox::Update(std::chrono::nanoseconds delta)
{
	if(mouseDown)
	{
		if(Input::MouseMoved())
		{
			glm::vec2 mousePos = Input::GetMousePosition();

			if(SelectionMade())
			{
				unsigned int newSelectionIndex = style->characterSet->GetIndexAtWidth(text.c_str(), static_cast<unsigned int>(std::max(mousePos.x - background->GetWorkArea().GetMinPosition().x - xOffset, 0.0f)));

				SetCursorIndex(newSelectionIndex);
				ExtendSelectionToCursor();
				SetXOffset();
			}
			else
			{
				unsigned int index = style->characterSet->GetIndexAtWidth(text.c_str(), static_cast<int>(mousePos.x - background->GetWorkArea().GetMinPosition().x + - xOffset));

				if(index != cursorIndex)
				{
					BeginSelection();
					SetCursorIndex(index);
					ExtendSelectionToCursor();
				}
			}
		}
	}
}

void TextBox::Draw(SpriteRenderer* spriteRenderer)
{
	background->Draw(spriteRenderer);

	Rect workArea = background->GetWorkArea();

	//TODO: Find a way to get rid of this scissor test
	spriteRenderer->EnableScissorTest(workArea);
	if(text != "")
	{
		//Draw text
		if(SelectionMade())
		{
			unsigned int selectionMinX = style->characterSet->GetWidthAtIndex(text.c_str(),selectionStartIndex);
			unsigned int selectionMaxX = style->characterSet->GetWidthAtIndex(text.c_str(),selectionEndIndex);

			//Selection highlight
			glm::vec2 minPosition(workArea.GetMinPosition());
			glm::vec2 drawPosition(minPosition);

			drawPosition.x = minPosition.x + xOffset + selectionMinX;

			spriteRenderer->Draw(Rect(drawPosition.x, drawPosition.y, static_cast<float>(selectionMaxX - selectionMinX), workArea.GetHeight()), style->textHighlightColor);

			drawPosition.x = minPosition.x + xOffset;

			spriteRenderer->DrawString(style->characterSet, text, drawPosition, 0, selectionStartIndex, style->textColorNormal); //Text before selection

			drawPosition.x = minPosition.x + xOffset + selectionMinX;
			spriteRenderer->DrawString(style->characterSet, text, drawPosition, selectionStartIndex, selectionEndIndex - selectionStartIndex, style->textColorSelected); //Selected text

			drawPosition.x = minPosition.x + xOffset + selectionMaxX;
			spriteRenderer->DrawString(style->characterSet, text, drawPosition, selectionEndIndex, static_cast<unsigned int>(text.size()) - selectionEndIndex, style->textColorNormal); //Text after selection
		}
		else
		{
			glm::vec2 minPosition(workArea.GetMinPosition());
			glm::vec2 drawPosition(minPosition);

			drawPosition.x += xOffset;

			spriteRenderer->DrawString(style->characterSet, text, drawPosition, style->textColorNormal); //TODO: Improve this, use ranged drawing instead
		}
	}
	spriteRenderer->DisableScissorTest();

	if(drawCursor)
	{
		if(cursorBlinkTimer.GetTime() >= cursorBlinkTime)
		{
			cursorBlinkTimer.Reset();

			cursorColor = cursorColor == style->cursorColorNormal ? style->cursorColorBlink : style->cursorColorNormal;
		}

		glm::vec2 drawPosition(background->GetWorkArea().GetMinPosition());
		drawPosition.x += style->cursorOffset.x;
		drawPosition.y += style->cursorOffset.y;

		if(text != "")
		{
			int width = style->characterSet->GetWidthAtIndex(text.c_str(), cursorIndex);

			drawPosition.x += width + xOffset;

			spriteRenderer->Draw(Rect(drawPosition.x, drawPosition.y, style->cursorSize.x, style->cursorSize.y), cursorColor);
		}
		else
			spriteRenderer->Draw(Rect(drawPosition.x, drawPosition.y, style->cursorSize.x, style->cursorSize.y), cursorColor);
	}
}

void TextBox::Insert(int index, unsigned int character)
{
	if(SelectionMade())
		EraseSelection();
	
	text.insert(index, 1, character);

	cursorIndex++;
	SetXOffset();
}

void TextBox::Insert(unsigned int index, const std::string& newText)
{
	if(SelectionMade())
		EraseSelection();

	if(index > text.size())
		index = text.size();

	text.insert(index, newText);

	cursorIndex += static_cast<int>(text.size());
	SetXOffset();
}

void TextBox::Insert(unsigned int index, const char* newText)
{
	std::string textString(newText);
	Insert(index, textString);
}

void TextBox::Erase(unsigned int startIndex, unsigned int count)
{
	text.erase(startIndex, count);

	if(count > 0)
	{
		cursorIndex -= count;

		if(cursorIndex < 0)
			SetCursorIndex(0);
	}

	SetXOffset();
}

void TextBox::Activate()
{
	cursorBlinkTimer.Reset();
	cursorBlinkTimer.Start();

	cursorColor = style->cursorColorNormal;

	drawCursor = true;
	receiveAllEvents = true;
}

void TextBox::Deactivate()
{
	Deselect();

	drawCursor = false;
	update = false;
	receiveAllEvents = false;
}

bool TextBox::OnMouseEnter()
{
	receiveAllEvents = true;

	return true;
}

bool TextBox::OnMouseDown(const MouseButtonState& keyState, const glm::vec2& mousePosition)
{
	if(area.Contains(mousePosition))
	{
		if(keyState.button == MOUSE_BUTTON::LEFT)
		{
			if(keyState.action == KEY_ACTION::DOWN)
			{
				cursorBlinkTimer.Reset();
				cursorBlinkTimer.Start();

				cursorColor = style->cursorColorNormal;

				drawCursor = true;

				unsigned int newCursorIndex = style->characterSet->GetIndexAtWidth(text.c_str(), static_cast<unsigned int>(mousePosition.x - background->GetWorkArea().GetMinPosition().x + -(xOffset)));

				if(keyState.mods == KEY_MODIFIERS::SHIFT)
				{
					if(!SelectionMade())
					{
						BeginSelection();
						SetCursorIndex(newCursorIndex);
						ExtendSelectionToCursor();
					}
					else
					{
						SetCursorIndex(newCursorIndex);
						ExtendSelectionToCursor();
					}
				}
				else
				{
					Deselect();
					SetCursorIndex(newCursorIndex);
				}

				update = true;
				mouseDown = true;

				return true;
			}
			else if(keyState.action == KEY_ACTION::REPEAT)
			{
				if(!SelectionMade())
				{
					JumpCursorLeft();
					BeginSelection();
					JumpCursorRight();
					ExtendSelectionToCursor();
				}

				return true;
			}
		}
	}
	else
		Deactivate();

	return false;
}

void TextBox::OnMouseUp(const MouseButtonState& keyState, const glm::vec2& mousePosition)
{
	if(keyState.button == MOUSE_BUTTON::LEFT
		&& keyState.action == KEY_ACTION::UP)
	{
		mouseDown = false;
		update = false;
	}
}

bool TextBox::OnKeyDown(const KeyState& keyState)
{
	if(drawCursor)
	{
		cursorBlinkTimer.Reset();
		cursorColor = style->cursorColorNormal;

		switch(keyState.key)
		{
			case KEY_CODE::BACKSPACE:
				if(allowEdit)
					BackspacePressed(keyState);
				break;
			case KEY_CODE::LEFT:
				LeftPressed(keyState);
				break;
			case KEY_CODE::RIGHT:
				RightPressed(keyState);
				break;
			case KEY_CODE::DELETE:
				if(allowEdit)
					DeletePressed(keyState);
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
//						strcpy_s(buffer, selectedText.size() + 1, selectedText.c_str());
//						GlobalUnlock(clipboardBuffer);
//
//						SetClipboardData(CF_TEXT, clipboardBuffer);
//
//						CloseClipboard();
//
//						if(allowEdit)
//						{
//							EraseSelection();
//							SetXOffset();
//						}
//					}
//					else
						Logger::LogLine(LOG_TYPE::WARNING, "Couldn't open clipboard");
				}
				break;
			case KEY_CODE::C:
				if(keyState.mods == KEY_MODIFIERS::CONTROL)
				{
					std::string selectedText = GetSelectedText();

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
//							Insert(cursorIndex, text);
//							SetXOffset();
//						}
//					}
//					else
						Logger::LogLine(LOG_TYPE::WARNING, "Couldn't open clipboard");
				}
				break;
			case KEY_CODE::A:
				if(keyState.mods == KEY_MODIFIERS::CONTROL)
				{
					SetCursorIndex(0);
					BeginSelection();
					SetCursorIndex(text.size());
					ExtendSelectionToCursor();
					SetXOffset();
				}
				break;
			case KEY_CODE::HOME:
				if(keyState.mods == KEY_MODIFIERS::SHIFT)
				{
					if(!SelectionMade())
						BeginSelection();

					SetCursorIndex(0);

					ExtendSelectionToCursor();
				}
				else
				{
					Deselect();
					SetCursorIndex(0);
				}

				SetXOffset();
				break;
			case KEY_CODE::END:

				if(keyState.mods == KEY_MODIFIERS::SHIFT)
				{
					if(!SelectionMade())
						BeginSelection();

					SetCursorIndex(text.size());

					ExtendSelectionToCursor();
				}
				else
				{
					Deselect();
					SetCursorIndex(text.size());
				}

				SetXOffset();
				break;
			default:
				break;
		}

		return true;
	}

	return false;
}

void TextBox::OnKeyUp(const KeyState& keyState)
{

}

bool TextBox::OnChar(unsigned int keyCode)
{
	if(drawCursor && allowEdit)
	{
		if(!SelectionMade())
			Insert(cursorIndex, keyCode);
		else
			Insert(selectionStartIndex, keyCode);

		cursorBlinkTimer.Reset();
		cursorColor = style->cursorColorNormal;

		return true;
	}

	return false;
}

void TextBox::LeftPressed(const KeyState& keyState)
{
	switch(keyState.mods)
	{
		case KEY_MODIFIERS::NONE:
			if(!SelectionMade())
			{
				MoveCursorLeft();
				SetXOffset();
			}
			else
			{
				int newCursorIndex = selectionStartIndex;
				Deselect();
				SetCursorIndex(newCursorIndex);
			}
			break;
		case KEY_MODIFIERS::SHIFT:
			if(!SelectionMade())
				BeginSelection();

			MoveCursorLeft();
			ExtendSelectionToCursor();
			SetXOffset();
			break;
		case KEY_MODIFIERS::CONTROL:
			Deselect();
			JumpCursorLeft();
			SetXOffset();
			break;
		case static_cast<KEY_MODIFIERS>(static_cast<int>(KEY_MODIFIERS::SHIFT) | static_cast<int>(KEY_MODIFIERS::CONTROL)):
			if(!SelectionMade())
				BeginSelection();

			JumpCursorLeft();
			ExtendSelectionToCursor();
			SetXOffset();
			break;
		default:
			break;
	}
}

void TextBox::RightPressed(const KeyState& keyState)
{
	switch(keyState.mods)
	{
		case KEY_MODIFIERS::NONE:
			if(!SelectionMade())
			{
				MoveCursorRight();
				SetXOffset();
			}
			else
			{
				int newCursorIndex = selectionEndIndex;
				Deselect();
				SetCursorIndex(newCursorIndex);
			}
			break;
		case KEY_MODIFIERS::SHIFT:
			if(!SelectionMade())
				BeginSelection();

			MoveCursorRight();
			ExtendSelectionToCursor();
			SetXOffset();
			break;
		case KEY_MODIFIERS::CONTROL:
			Deselect();
			JumpCursorRight();
			SetXOffset();
			break;
		case static_cast<KEY_MODIFIERS>(static_cast<int>(KEY_MODIFIERS::SHIFT) | static_cast<int>(KEY_MODIFIERS::CONTROL)) :
			if(!SelectionMade())
				BeginSelection();

			JumpCursorRight();
			ExtendSelectionToCursor();
			SetXOffset();
			break;
		default:
			break;
	}
}

void TextBox::BackspacePressed(const KeyState& keyState)
{
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
			if(cursorIndex == 0)
				return;

			text.erase(cursorIndex - 1, 1);

			MoveCursorLeft();
			unsigned int index = cursorIndex;

			JumpCursorLeft();
			SetXOffset();
			SetCursorIndex(index);
		}
	}

	SetXOffset();
}

void TextBox::DeletePressed(const KeyState& keyState)
{
	if(keyState.mods == KEY_MODIFIERS::CONTROL)
	{
		BeginSelection();
		JumpCursorRight();
		ExtendSelectionToCursor();
		EraseSelection();
	}
	else
	{
		if(SelectionMade())
			EraseSelection();
		else
			text.erase(cursorIndex, 1);
	}

	SetXOffset();
}

void TextBox::MoveCursorLeft()
{
	if(cursorIndex > 0)
		cursorIndex--;
}

void TextBox::MoveCursorRight()
{
	if(cursorIndex < static_cast<int>(text.size()))
		cursorIndex++;
}

void TextBox::JumpCursorLeft()
{
	if(cursorIndex <= 1)
	{
		SetCursorIndex(0);
		return;
	}

	//cursorIndex is used "locally" here, so make sure to call SetCursorIndex before returning!
	--cursorIndex;

	bool nonSpace = jumpSeparators.find(text[cursorIndex]) == jumpSeparators.npos;

	for(; cursorIndex > 0; --cursorIndex)
	{
		if(jumpSeparators.find(text[cursorIndex - 1]) != jumpSeparators.npos)
		{
			if(nonSpace)
			{
				SetCursorIndex(cursorIndex);
				return;
			}
		}
		else
			nonSpace = true;
	}

	SetCursorIndex(0);
}

void TextBox::JumpCursorRight()
{
	//TODO: Rewrite this? Look at JumpCursorLeft
	auto iter = text.begin() + cursorIndex;

	if(iter == text.end())
		return;

	bool nonSpace = jumpSeparators.find(*iter) == jumpSeparators.npos;

	for(MoveCursorRight(), ++iter; cursorIndex < static_cast<int>(text.size()); cursorIndex++)
	{
		//if(*iter == CharacterSet::SPACE_CHARACTER)
		if(jumpSeparators.find(*iter) != jumpSeparators.npos)
		{
			if(nonSpace)
			{
				if(!jumpToBeforeSeparator)
					MoveCursorRight();

				return;
			}
		}
		else
			nonSpace = true;

		++iter;
	}
}

void TextBox::ExtendSelectionToCursor()
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
}

void TextBox::BeginSelection()
{
	selectionIndex = cursorIndex;
	selectionStartIndex = selectionIndex;
}

void TextBox::Deselect()
{
	selectionEndIndex = -1;
	selectionStartIndex = -1;
}

void TextBox::EraseSelection()
{
	if(SelectionMade())
	{
		text.erase(selectionStartIndex, selectionEndIndex - selectionStartIndex);
		SetCursorIndex(selectionStartIndex);
		Deselect();
	}
}

bool TextBox::SelectionMade() const
{
	return selectionStartIndex != -1 && selectionEndIndex != -1;
}

void TextBox::SetXOffset()
{
	int widthAtCursor = style->characterSet->GetWidthAtIndex(text.c_str(), cursorIndex);

	if(widthAtCursor > background->GetWorkArea().GetWidth() - (xOffset + style->cursorSize.x))
		xOffset = -(widthAtCursor - background->GetWorkArea().GetWidth() + style->cursorSize.x);
	else if(widthAtCursor < -xOffset)
		xOffset = static_cast<float>(-widthAtCursor);
}

void TextBox::SetText(const std::string& newText)
{
	if(!style->characterSet->IsLoaded())
	{
		Logger::LogLine(LOG_TYPE::FATAL, "Trying to set text to \"" + text + "\" before setting a font!");
		return;
	}

	//constructedString = ConstructedString(style->characterSet, text);
	text = newText;
	//SetCursorIndex(static_cast<unsigned int>(utf8::unchecked::distance(text.begin(), text.end())));
	SetCursorIndex(static_cast<unsigned int>(text.size()));

	SetXOffset();
}

std::string TextBox::GetText() const
{
	return text;
}

std::string TextBox::GetSelectedText() const
{
	if(!SelectionMade())
		return "";

	auto beginIter = text.begin() + selectionStartIndex;
	//for(int i = 0; i < selectionStartIndex; i++)
	//	utf8::unchecked::next(beginIter);

	auto endIter = beginIter + (selectionEndIndex - selectionStartIndex);
	//for(int i = selectionStartIndex; i < selectionEndIndex; i++)
	//	utf8::unchecked::next(endIter);

	std::string returnString = text.substr(static_cast<unsigned int>(std::distance(text.begin(), beginIter)), static_cast<unsigned int>(std::distance(beginIter, endIter)));

	return returnString;
}

int TextBox::GetCharacterAt(unsigned int index) const
{
	if(text.size() == 0
	   || index > text.size())
		return 0;

	auto iter = text.begin() + index;
	//utf8::unchecked::advance(iter, blockIndex);

	return *iter;
}

int TextBox::GetTextLength() const
{
	return text.size();
}

int TextBox::GetCursorIndex() const
{
	return cursorIndex;
}

void TextBox::SetCursorIndex(unsigned int newIndex)
{
	if(newIndex > text.size())
		newIndex = text.size();

	cursorIndex = newIndex;
}

void TextBox::SetJumpSeparators(const std::string& separators)
{
	jumpSeparators = separators;
}

void TextBox::SetJumpToBeforeSeparator(bool jump)
{
	jumpToBeforeSeparator = jump;
}

void TextBox::SetCharacterSet(const std::string& fontPath, ContentManager* contentManager)
{
	SetCharacterSet(contentManager->Load<CharacterSet>(fontPath));
}

void TextBox::SetCharacterSet(const CharacterSet* characterSet)
{
	style->characterSet = characterSet;

	//constructedString = ConstructedString(style->characterSet, constructedString.text);
}

bool TextBox::GetIsEmpty() const
{
	return text.empty();
}

bool TextBox::GetSelectionMade() const
{
	return SelectionMade();
}

bool TextBox::GetIsActive() const
{
	return drawCursor;
}

void TextBox::ReplaceActiveCharacterBlockText(std::string newText) //TODO: Restore selection?
{
	Deselect();

	unsigned int index = 0;

	std::vector<CharacterBlock> blocks = style->characterSet->Split(text.c_str());
	for(const CharacterBlock& block : blocks)
	{
		if(static_cast<int>(index + block.length) >= cursorIndex)
		{
			SetCursorIndex(index);
			BeginSelection();
			SetCursorIndex(index + block.length);
			ExtendSelectionToCursor();

			Insert(static_cast<unsigned int>(selectionStartIndex), newText);

			return;
		}
		else
			index += block.length + 1;
	}
}

std::string TextBox::GetActiveCharacterBlockText()
{
	unsigned int index = 0;
	std::vector<CharacterBlock> blocks = style->characterSet->Split(text.c_str());
	for(const CharacterBlock& block : blocks)
	{
		if(static_cast<int>(index + block.length) >= cursorIndex)
			return text.substr(index, block.length);
		else
			index += block.length + 1;
	}

	return "";
}

std::string TextBox::GetActiveCharacterBlockText(unsigned int& index)
{
	index = 0;

	std::vector<CharacterBlock> blocks = style->characterSet->Split(text.c_str());
	for(const CharacterBlock& block : blocks)
	{
		if(static_cast<int>(index + block.length) >= cursorIndex)
			return text.substr(index, block.length);
		else
			index += block.length + 1;
	}

	return "";
}

std::shared_ptr<TextBoxStyle> TextBox::GenerateDefaultStyle(ContentManager* contentManager)
{
	auto style = std::make_shared<TextBoxStyle>();

	style->characterSet = contentManager->Load<CharacterSet>("../graphics/textures/fonts/calibri16.fnt");

	style->textColorNormal = COLORS::black;
	style->textColorSelected = COLORS::black;
	style->textHighlightColor = COLORS::white;

	style->cursorSize = glm::vec2(2.0f, static_cast<float>(style->characterSet->GetLineHeight()));
	style->cursorColorNormal = COLORS::black;
	style->cursorColorBlink = COLORS::black;
	style->cursorColorBlink.w = 0.0f;

	return style;
}

std::unique_ptr<GUIBackground> TextBox::GenerateDefaultBackground(ContentManager* contentManager)
{
	return std::make_unique<OutlineBackground>();
}

std::shared_ptr<GUIBackgroundStyle> TextBox::GenerateDefaultBackgroundStyle(ContentManager* contentManager)
{
	auto style = std::make_shared<OutlineBackgroundStyle>();
	
	style->AddColor(COLORS::snow4, COLORS::snow3);
	style->outlineThickness = 2.0f;

	return style;
}