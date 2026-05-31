#include "InputHandler.h"
#include "MCM.h"

void ScaleformUI::InputHandler::Init()
{
	deltaTime = (float*)RELOCATION_ID(523660, 410199).address();
	initialized = true;
}

bool ScaleformUI::InputHandler::IsInitialized()
{
	return initialized;
}

void ScaleformUI::InputHandler::AddMenu(Menu* a_menu)
{ 
	menus.emplace_back(a_menu); 
}

void ScaleformUI::InputHandler::SetScrollUpKey(uint32_t a_keycode)
{
	buttonEvents[ButtonName::kScrollUpKey] = ButtonEvent(a_keycode);
}

void ScaleformUI::InputHandler::SetScrollDownKey(uint32_t a_keycode)
{
	buttonEvents[ButtonName::kScrollDownKey] = ButtonEvent(a_keycode);
}


RE::BSEventNotifyControl ScaleformUI::InputHandler::ProcessEvent(RE::InputEvent* const* a_eventPtr, RE::BSTEventSource<RE::InputEvent*>*)
{
	if (!a_eventPtr || !initialized) return RE::BSEventNotifyControl::kContinue;

	bool mouseInput = false;

	for (auto* event = *a_eventPtr; event; event = event->next)
	{
		bool onMouseMove = event->GetEventType() == RE::INPUT_EVENT_TYPE::kMouseMove;
		bool onButtonPress = event->GetEventType() == RE::INPUT_EVENT_TYPE::kButton;
		if ( !onMouseMove && !onButtonPress ) return RE::BSEventNotifyControl::kContinue;

		if (onMouseMove) mouseInput = true;

		if (onButtonPress)
		{
			if (auto* buttonEvent = event->AsButtonEvent())
			{
				uint32_t offset = 0;
				if (event->GetDevice() == RE::INPUT_DEVICE::kMouse) 
				{
					mouseInput = true;
					offset = 256;
				}
				auto dxScanCode = buttonEvent->GetIDCode();

				uint32_t keyCode = dxScanCode + offset;

				if (keyCode != 0)
				{
					for (auto& [keyName, key] : buttonEvents)
					{
						if (key.keyCode == keyCode)
						{
							key.active = true;
							key.duration = buttonEvent->HeldDuration();
							key.isDown = !buttonEvent->IsUp();
							break;
						}
					}
				}

				for (auto& menu : menus)
				{
					// IsDown() is only true the first frame the button is down
					if (keyCode == menu->GetMenuOpenKeyCode() && buttonEvent->IsDown())
					{
						auto uiTask = [&]()
						{
							menu->ToggleMenu();
						};
						SKSE::GetTaskInterface()->AddUITask(uiTask);
					}
				}
			}
		}
	}

	if (mouseInput) durationWithNoMouseInput = 0.0f;

	return RE::BSEventNotifyControl::kContinue;
}

void ScaleformUI::InputHandler::ResetKeys()
{
	for (auto& [keyName, key] : buttonEvents)
	{
		if (key.active) key.Reset();
	}
}

void ScaleformUI::InputHandler::Update()
{
	if (initialized)
	{
		durationWithNoMouseInput += *deltaTime;
		auto uiTask = [&]()
		{
			ProcessInputs();
			UpdateMenus();
		};
		SKSE::GetTaskInterface()->AddUITask(uiTask);
	}
}

ScaleformUI::Size ScaleformUI::InputHandler::GetScrollDistance()
{
	if (buttonEvents[ButtonName::kScrollDownKey].active && buttonEvents[ButtonName::kScrollDownKey].duration == 0.0f)
	{
		return Size(-keyScrollDistance).vh();
	}
	if (buttonEvents[ButtonName::kScrollUpKey].active && buttonEvents[ButtonName::kScrollUpKey].duration == 0.0f)
	{
		return Size(keyScrollDistance).vh();
	}
	if (buttonEvents[ButtonName::kMouseWheelDown].active && buttonEvents[ButtonName::kMouseWheelDown].duration == 0.0f)
	{
		return Size(mouseWheelScrollDistance).vh();
	}
	if (buttonEvents[ButtonName::kMouseWheelUp].active && buttonEvents[ButtonName::kMouseWheelUp].duration == 0.0f)
	{
		return Size(-mouseWheelScrollDistance).vh();
	}
	return Size(0.0f);
}

void ScaleformUI::InputHandler::ProcessInputs()
{
	auto cursor = RE::MenuCursor::GetSingleton();
	float cursorX = cursor->cursorPosX;
	float cursorY = cursor->cursorPosY;

	// The frame time cost of this loop is in single digit microseconds (2), 
	// Tested on 9800x3D
	// An animation costs <10 µs

	auto& primaryClick = buttonEvents[ButtonName::kPrimary];
	Size scrollDistance = GetScrollDistance();
	bool scrollIsByKey = buttonEvents[ButtonName::kScrollUpKey].active || buttonEvents[ButtonName::kScrollDownKey].active;

	// Only allow one container to scroll per frame
	bool hasScrolled = false;

	// Only allow one interaction at a time
	bool hasInteracted = false;

	for (auto& menu : menus)
	{
		if (menu->IsClosed()) continue;
		
		menu->TraverseUIElements([&](Element* a_element)
		{
			if (a_element->IsInteractable())
			{				
				bool isCursorPositionValid = true;
				if (const auto& mask = a_element->GetMask())
				{
					// Use world bounds because that defines what is visible under the mask
					isCursorPositionValid = mask->GetWorldBounds()->IsPointInBBox(cursorX, cursorY);
				}

				bool hover = isCursorPositionValid && a_element->GetInteractableWorldBounds()->IsPointInBBox(cursorX, cursorY);
				bool pressed = hover || a_element->IsPressed() ? primaryClick.active : false;
				if (hasInteracted)
				{
					hover = false;
					pressed = false;
				}
				if (hover) a_element->OnHover(durationWithNoMouseInput);
				else a_element->ResetHover();


				if (pressed) a_element->OnClick(primaryClick.duration, primaryClick.isDown);
				else a_element->ResetClick();

				hasInteracted = hover || pressed; 


			}

			if (!hasScrolled && a_element->IsGroup() && scrollDistance.size != 0.0f)
			{
				auto* group = a_element->AsGroup();
				if (group->IsVerticallyScrollable() && group->IsVisible())
				{
					if (scrollIsByKey || group->GetScrollableArea()->IsPointInBBox(cursorX, cursorY))
					{
						hasScrolled = true;
						group->ScrollVertically(scrollDistance);
					}
				}
			}
		});
	}

	ResetKeys();

}

void ScaleformUI::InputHandler::UpdateMenus()
{
	for (auto& menu : menus)
	{
		if (menu->IsClosed()) continue;
		menu->Update(*deltaTime);

		//menu->DrawInteractableBounds(0x00FF00);
		//menu->DrawGroupBounds(0x1DD5DB);
		//menu->DrawObjectBounds(0xFF0000);

		if (menu->HasDebugDraws())
		{
			menu->ClearDebugDraws();
			menu->DrawDebugInfo();
			//menu->DrawGrid(11, 19, 2.0f, 0xFF0000, 70); // rougly 100 pixels between lines at full HD

		}
	}
}
