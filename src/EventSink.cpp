#include "EventSink.h"
#include "DrawMenu.h"
#include "HUDHandler.h"
#include "DebugHandler.h"

static void TPSGetCurrentRotation(RE::ThirdPersonState* a_tps, RE::NiQuaternion& a_rotation)
		{
			using func_t = decltype(&TPSGetCurrentRotation);
			REL::Relocation<func_t> func{ RELOCATION_ID(51246, 50897) }; //SE ID NOT CORRECT
			return func(a_tps, a_rotation);
		}

static void FPSGetCurrentRotation(RE::FirstPersonState* a_fps, RE::NiQuaternion& a_rotation)
		{
			using func_t = decltype(&FPSGetCurrentRotation);
			REL::Relocation<func_t> func{ RELOCATION_ID(51246, 50725) }; //SE ID NOT CORRECT
			return func(a_fps, a_rotation);
		}

RE::BSEventNotifyControl EventSink::ProcessEvent(RE::InputEvent* const* eventPtr, RE::BSTEventSource<RE::InputEvent*>*)
{
	if (!eventPtr) return RE::BSEventNotifyControl::kContinue;

	for (auto* event = *eventPtr; event; event = event->next) // cycles through all key presses each frame
	{
		if (event->GetEventType() != RE::INPUT_EVENT_TYPE::kButton) continue;//return RE::BSEventNotifyControl::kContinue;

		auto* buttonEvent = event->AsButtonEvent();
		auto dxScanCode = buttonEvent->GetIDCode();
		auto inputReleased = buttonEvent->IsUp();

		//////////////////// Configure code below /////////////////////////////////////

		if (!RE::UI::GetSingleton()->GameIsPaused()) // for what should only run when in game
		{
			if (dxScanCode == keycode1)
			{
				if (inputReleased)
					key1HasBeenProcessed = false;
				if (!key1HasBeenProcessed && buttonEvent->IsDown()) // calls the first frame when the key is pressed
				{
					logger::info("");
					logger::info("key 1 was pressed");
					key1HasBeenProcessed = true;

					if (isDrawMenuOpen)
					{
						logger::info("Closing menu");
						DrawMenu::CloseMenu();
						HUDHandler::GetSingleton()->g_DrawMenu = nullptr;
						DebugHandler::GetSingleton()->ClearAll();
						isDrawMenuOpen = false;
					}
					else
					{
						logger::info("Opening menu");
						DrawMenu::OpenMenu();
						auto uiTask = []() 
						{
							RE::UI* ui = RE::UI::GetSingleton();
							auto g_HUDHandler = HUDHandler::GetSingleton();
							g_HUDHandler->g_DrawMenu = HUDHandler::GetDrawMenuHud();
							if (g_HUDHandler->g_DrawMenu)
							{
								g_HUDHandler->canvasWidth = g_HUDHandler->g_DrawMenu->canvasWidth;
								g_HUDHandler->canvasHeight = g_HUDHandler->g_DrawMenu->canvasHeight;
								logger::info("w, h {}, {}", g_HUDHandler->canvasWidth,g_HUDHandler->canvasHeight);
							}
						};
						SKSE::GetTaskInterface()->AddUITask(uiTask);
						auto pos = RE::PlayerCharacter::GetSingleton()->GetPosition();
						logger::info("x,y,z {}, {}, {}", pos.x, pos.y, pos.z);
						DebugHandler::GetSingleton()->DrawAll();
						isDrawMenuOpen = true;
					}
				}

				else if (inputReleased) // calls the first frame when the key is released
				{
					//DrawMenu::CloseMenu();

					//logger::info(" -key 1 was released");
					//DebugHandler::GetSingleton()->g_HUD->updateHUD();
				}
			}
		}

		// --------- for what can run whenever ----------------------------------------------

	}

	return RE::BSEventNotifyControl::kContinue;

}