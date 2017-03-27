#include "guiManager.h"

#include "../os/input.h"
#include "../spriteRenderer.h"

#include <algorithm>

GUIManager::GUIManager()
	: lockedContainer(nullptr)
	, lastActiveContainer(nullptr)
{
}

GUIManager::~GUIManager()
{
}

void GUIManager::AddContainer(GUIContainer* container)
{
	containers.push_back(container);
}

void GUIManager::SetContainers(std::vector<GUIContainer*> containers)
{
	this->containers = containers;
}

void GUIManager::SendToFront(GUIContainer* container)
{
	auto iter = std::find(containers.begin(), containers.end(), container);
	if(iter == containers.end())
		return;

	containers.erase(iter);
	containers.insert(containers.begin(), container);
}

void GUIManager::SendToBack(GUIContainer* container)
{
	auto iter = std::find(containers.begin(), containers.end(), container);
	if(iter == containers.end())
		return;

	containers.erase(iter);
	containers.insert(containers.end(), container);
}

void GUIManager::SendToLayer(GUIContainer* container, int layer)
{
	if(layer >= static_cast<int>(containers.size()))
		SendToBack(container);
	else
	{
		auto iter = std::find(containers.begin(), containers.end(), container);
		if(iter == containers.end())
			return;

		containers.erase(iter);
		containers.insert(containers.begin() + layer, container);
	}
}

void GUIManager::Update(std::chrono::nanoseconds delta)
{
	glm::vec2 mousePosition = Input::GetMousePosition();

	//If any container is locked there is no need to check the rest for enter/exit events
	if(lockedContainer != nullptr)
	{
		if(lockedContainer->area.Contains(mousePosition))
		{
			if(!lockedContainer->mouseInside
			   && lockedContainer->OnMouseEnter())
			{
				lockedContainer->mouseInside = true;
			}
		}
		else
		{
			if(lockedContainer->mouseInside)
			{
				lockedContainer->OnMouseExit();
				lockedContainer->mouseInside = false;
			}
		}
	}
	else
	{
		for(GUIContainer* container : containers)
		{
			if(container->GetArea().Contains(mousePosition))
			{
				if(!container->mouseInside
				   && container->OnMouseEnter())
				{
					container->mouseInside = true;
				}
			}
			else
			{
				if(container->mouseInside)
				{
					container->OnMouseExit();
					container->mouseInside = false;
				}
			}
		}
	}

	for(GUIContainer* container : containers)
	{
		if(container->GetUpdate())
			container->Update(delta);
	}
}

void GUIManager::Draw(SpriteRenderer* spriteRenderer)
{
	// TODO
	//deviceContext->RSSetState(RasterizerStates::solidScissor->Get());
	//deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//float blendFactors[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	//deviceContext->OMSetBlendState(BlendStates::singleDefault->Get(), blendFactors, 0xFFFFFFFF);
	//deviceContext->OMSetDepthStencilState(DepthStencilStates::off->Get(), 0xFFFFFFFF);

	//D3D11_RECT originalRect;
	//UINT numRects = 1;
	//deviceContext->RSGetScissorRects(&numRects, &originalRect);

	//for(GUIContainer* container : containers)
	for(auto iter = containers.rbegin(); iter != containers.rend(); ++iter)
	{
		GUIContainer* container = *iter;

		if(container->draw)
		{
			//if(container->clip)
			//{
			//	Rect containerArea = container->GetArea();
//
			//	D3D11_RECT scissorRect;
			//	scissorRect.top = static_cast<LONG>(containerArea.GetMinPosition().y);
			//	scissorRect.bottom = static_cast<LONG>(containerArea.GetMaxPosition().y);
			//	scissorRect.left = static_cast<LONG>(containerArea.GetMinPosition().x);
			//	scissorRect.right = static_cast<LONG>(containerArea.GetMaxPosition().x);
			//	deviceContext->RSSetScissorRects(1, &scissorRect);
			//}
			//else
			//	deviceContext->RSSetScissorRects(1, &originalRect);

			container->Draw(spriteRenderer);

			if(spriteRenderer->AnythingToDraw())
				spriteRenderer->Draw();
		}
	}

	//deviceContext->RSSetScissorRects(1, &originalRect);
}

void GUIManager::KeyEvent(const KeyState& keyState)
{
	if(keyState.action == KEY_ACTION::DOWN
		|| keyState.action == KEY_ACTION::REPEAT)
	{
		for(GUIContainer* container : containers)
			if(container->receiveAllEvents)
				if(container->OnKeyDown(keyState))
					break;
	}
	else
	{
		for(GUIContainer* container : containers)
			if(container->receiveAllEvents)
				container->OnKeyUp(keyState);
	}
}

void GUIManager::MouseEvent(const MouseButtonState& keyState)
{
	glm::vec2 mousePosition = Input::GetMousePosition();

	if(keyState.action == KEY_ACTION::DOWN
		|| keyState.action == KEY_ACTION::REPEAT)
	{
		for(GUIContainer* container : containers)
			if(container->receiveAllEvents)
				if(container->OnMouseDown(keyState, mousePosition))
				{
					if(lastActiveContainer != nullptr
					   && lastActiveContainer != container)
						lastActiveContainer->Deactivate();

					lockedContainer = container;
					lastActiveContainer = container;
					break;
				}
	}
	else
	{
		for(GUIContainer* container : containers)
		{
			if(container == lockedContainer)
				lockedContainer = nullptr;

			if(container->receiveAllEvents)
				container->OnMouseUp(keyState, mousePosition);
		}
	}
}

void GUIManager::CharEvent(unsigned int keyCode)
{
	for(GUIContainer* container : containers)
		if(container->receiveAllEvents)
			if(container->OnChar(keyCode))
				break;
}

void GUIManager::ScrollEvent(int distance)
{
	for(GUIContainer* container : containers)
		if(container->receiveAllEvents)
			if(container->OnScroll(distance))
				break;
}

void GUIManager::ClearContainers()
{
	containers.clear();
}