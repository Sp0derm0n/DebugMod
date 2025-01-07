#include "EventSink.h"
#include "DrawMenu.h"
#include "DebugHandler.h"
#include "DrawHandler.h"
#include "DebugUIMenu.h"
#include "UIHandler.h"
#include "MCM.h"

RE::BSEventNotifyControl EventSink::ProcessEvent(RE::InputEvent* const* eventPtr, RE::BSTEventSource<RE::InputEvent*>*)
{
	if (!MCM::settings::modActive) return RE::BSEventNotifyControl::kContinue; 

	if (!eventPtr) return RE::BSEventNotifyControl::kContinue;

	for (auto* event = *eventPtr; event; event = event->next) // cycles through all key presses each frame
	{
		if (event->GetEventType() != RE::INPUT_EVENT_TYPE::kButton) continue;//return RE::BSEventNotifyControl::kContinue;

		uint32_t offset = 0;
		if (event->GetDevice() == RE::INPUT_DEVICE::kMouse /* || RE::INPUT_DEVICE::kGamepad*/) offset = 256;

		//if(event->GetDevice() == RE::INPUT_DEVICE::kGamepad) logger::info("keycode: {}", event->AsButtonEvent()->GetIDCode());

		auto* buttonEvent = event->AsButtonEvent();
		auto dxScanCode = buttonEvent->GetIDCode();
		auto inputReleased = buttonEvent->IsUp();


		//////////////////// Configure code below /////////////////////////////////////

		uint32_t keyCode = dxScanCode + offset;

		if (!RE::UI::GetSingleton()->GameIsPaused()) // for what should only run when in game
		{
			
			if ( keyCode == MCM::settings::openMenuHotkey)
			{
				HandleDebugMenu(inputReleased);
			}
			else if (keyCode == keycode2)
			{
				HandlePrimaryClick(inputReleased);
			}
			else if (keyCode == MCM::settings::scrollUpHotkey)
			{
				HandleScrollUp(inputReleased);
			}
			else if (keyCode == MCM::settings::scrollDownHotkey)
			{
				HandleScrollDown(inputReleased);
			}
		}
		// --------- for what can run whenever ----------------------------------------------

		else if (UIHandler::GetSingleton()->isMenuOpen)
		{
			if ( keyCode == MCM::settings::openMenuHotkey)
			{
				HandleDebugMenu(inputReleased);
				key1HasBeenProcessed = false;
			}
		}
	}
	return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl EventSink::ProcessEvent(const RE::TESCellFullyLoadedEvent* a_event, RE::BSTEventSource<RE::TESCellFullyLoadedEvent>*)
{
	if (!MCM::settings::modActive) return RE::BSEventNotifyControl::kContinue; 

	if (!a_event) return RE::BSEventNotifyControl::kContinue; 
	
	RE::TESObjectCELL* cell = a_event->cell;
	if (cell) DebugHandler::GetSingleton()->CacheCellNavmeshes(cell);

	return RE::BSEventNotifyControl::kContinue;
}


void EventSink::HandleDebugMenu(bool a_isInputReleased)
{
	if (a_isInputReleased)
	{
		key1HasBeenProcessed = false;
	}
	else if (!key1HasBeenProcessed)
	{
		key1HasBeenProcessed = true;

		auto g_UI = UIHandler::GetSingleton();

		if (g_UI->isMenuOpen)
		{
			g_UI->Unload();
		}
		else
		{
			UIHandler::GetSingleton()->Init();
		}
	}
}

void EventSink::HandlePrimaryClick(bool a_isInputReleased)
{
	if (a_isInputReleased)
	{
		key2HasBeenProcessed = false;

		UIHandler::GetSingleton()->mouseReleased = true;
		UIHandler::GetSingleton()->mousePressed = false;
	}
	else if (!key2HasBeenProcessed)
	{
		key2HasBeenProcessed = true;
		
		UIHandler::GetSingleton()->mousePressed = true;
	}
}

void EventSink::HandleScrollUp(bool a_isInputReleased)
{
	if (a_isInputReleased)
	{
		key3HasBeenProcessed = false;

	}
	else if (!key3HasBeenProcessed)
	{
		key3HasBeenProcessed = true;
		DrawHandler::GetSingleton()->InfoBoxScrollUp();
	}
}

void EventSink::HandleScrollDown(bool a_isInputReleased)
{
	if (a_isInputReleased)
	{
		key4HasBeenProcessed = false;

	}
	else if (!key4HasBeenProcessed)
	{
		key4HasBeenProcessed = true;
		DrawHandler::GetSingleton()->InfoBoxScrollDown();
	}
}