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
			else if (keyCode == keycode3)
			{
				HandleScrollWheelUp(inputReleased);
			}
			else if (keyCode == keycode4)
			{
				HandleScrollWheelDown(inputReleased);
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
	if (MCM::settings::modActive && a_event)
	{
		DebugHandler::GetSingleton()->OnCellFullyLoaded(a_event->cell);
	}
	return RE::BSEventNotifyControl::kContinue;
}

static RE::BGSSoundDescriptorForm* GetSoundDescriptorFormFromEditorID(const char* a_editorID) // works better than RE::TESForm::LookupByEditorID
{
	using func_t = decltype(&GetSoundDescriptorFormFromEditorID);
	REL::Relocation<func_t> func{ RELOCATION_ID(51246, 33044) };
	return func(a_editorID);
}

static uint32_t LengthOfEditorID(uint32_t& a_value, const char* a_editorID, bool a_arg3) // works better than RE::TESForm::LookupByEditorID
{
	using func_t = decltype(&LengthOfEditorID);
	REL::Relocation<func_t> func{ RELOCATION_ID(51246, 68216) };
	return func(a_value, a_editorID, a_arg3);
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
			auto player = RE::PlayerCharacter::GetSingleton();
		}
		else
		{
			UIHandler::GetSingleton()->Init();
			auto player = RE::PlayerCharacter::GetSingleton();
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
		UIHandler::GetSingleton()->keyScrollUp = true;
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
		UIHandler::GetSingleton()->keyScrollDown = true;
	}
}

void EventSink::HandleScrollWheelUp(bool a_isInputReleased)
{
	if (!key5HasBeenProcessed)
	{
		key5HasBeenProcessed = true;
		UIHandler::GetSingleton()->mouseScrollUp = true; // it is sat to false after it has been processed
	}
	else
	{
		key5HasBeenProcessed = false;
	}
}

void EventSink::HandleScrollWheelDown(bool a_isInputReleased)
{
	if (!key6HasBeenProcessed)
	{
		key6HasBeenProcessed = true;
		UIHandler::GetSingleton()->mouseScrollDown = true; // it is sat to false after it has been processed
	}
	else
	{
		key6HasBeenProcessed = false;
	}
	
}

