#include "DebugMenu.h"
#include "MCM.h"
#include "Interface/UIHandler.h"
#include "FreeCamHandler.h"

namespace DebugMenu
{
	static auto debugMenuHandler = std::make_unique<DebugMenuHandler>();

	std::unique_ptr<DebugMenuHandler>& GetDebugMenuHandler() { return debugMenuHandler; }

	std::unique_ptr<DrawHandler>& GetDrawHandler() { return debugMenuHandler->drawHandler; }
	std::unique_ptr<BoxHandler>& GetBoxHandler() { return debugMenuHandler->boxHandler; }
	std::unique_ptr<CellHandler>& GetCellHandler() { return debugMenuHandler->cellHandler; }
	std::unique_ptr<NavmeshHandler>& GetNavmeshHandler() { return debugMenuHandler->navmeshHandler; }
	std::unique_ptr<MarkerHandler>& GetMarkerHandler() { return debugMenuHandler->markerHandler; }
	std::unique_ptr<CollisionHandler>& GetCollisionHandler() { return debugMenuHandler->collisionHandler; }
	std::unique_ptr<RefInspectorHandler>& GetRefInspectorHandler() { return debugMenuHandler->refInspectorHandler; }

	void DebugMenuHandler::Init()
	{
		drawHandler = std::make_unique<DrawHandler>();

		boxHandler = std::make_unique<BoxHandler>();
		cellHandler = std::make_unique<CellHandler>();
		navmeshHandler = std::make_unique<NavmeshHandler>();
		markerHandler = std::make_unique<MarkerHandler>();
		collisionHandler = std::make_unique<CollisionHandler>();
		refInspectorHandler = std::make_unique<RefInspectorHandler>();

		MCM::DebugMenuMCM::UpdateCollisionColor();

		deltaTime = (float*)RELOCATION_ID(523660, 410199).address();
	}

	void DebugMenuHandler::CloseAndReset()
	{
		markerHandler->Reset();
		collisionHandler->Reset();
		if (FreeCamHandler::GetSingleton()->IsCustomFreeCamEnabled()) FreeCamHandler::GetSingleton()->ToggleTFC();
		//DrawMenu::CloseMenu();
		ScaleformUI::UIHandler::GetSingleton()->drawMenu->Close();
		ScaleformUI::UIHandler::GetSingleton()->debugMenuUI->Close();

		isDebugMenuActive = false; // must be sat false after freecam is toggled


	}

	void DebugMenuHandler::OnDebugMenuUIOpen()
	{
		isDebugMenuActive = true; // Must be sat true before freecam is toggled

		if (!hasDebugMenuBeenOpenedBefore)
		{
			hasDebugMenuBeenOpenedBefore = true;
			MCM::DebugMenuMCM::ReadSettings(true);

			if (MCM::settings::enableFreeCamOnOpen)
			{
				if (const auto& playerCamera = RE::PlayerCamera::GetSingleton())
				{					
					// switch into custom free cam if tfc is active before menu was opened
					if (playerCamera->IsInFreeCameraMode()) FreeCamHandler::GetSingleton()->ToggleTFC();
				}
				FreeCamHandler::GetSingleton()->ToggleTFC();
			}
		}
	}

	void DebugMenuHandler::Update()
	{
		if (!Utils::IsPlayerLoaded()) return;

		if (!hasDebugMenuBeenOpenedBefore || !isDebugMenuActive) return;

		bool isGamePaused = RE::UI::GetSingleton()->GameIsPaused();


		timeSinceLastUpdate += *deltaTime;

		if (drawHandler && drawHandler->g_DrawMenu)
		{
			if (MCM::settings::updateRate == 0 || timeSinceLastUpdate > 1.0f / MCM::settings::updateRate)
			{
				timeSinceLastUpdate = 0;
				drawHandler->alphaMultiplier = GetLightLevel();
				DrawPeriodically(isGamePaused);
			}
			DrawEveryFrame(isGamePaused);
			drawHandler->Update(*deltaTime);

		}
	
		// has to be lowest in this function
		//if (drawHandler && drawHandler->isMenuOpen == false)
		if (ScaleformUI::UIHandler::GetSingleton()->drawMenu->IsClosed())
		{
			if (isAnyDebugON())
				OpenDrawMenu();
			else
				return;
		}
		else if (!isAnyDebugON())
			CloseDrawMenu();
	}

	void DebugMenuHandler::ResetUpdateTimer()
	{
		timeSinceLastUpdate = 1000.0f;
	}

	void DebugMenuHandler::DrawEveryFrame(bool a_isGamePaused)
	{
		ShowCoordinates();

		if (MCM::settings::updateCollisionsEveryFrame)
		{
			// D3D11 should always clear, including when game is paused since it draws on top of the ui
			drawHandler->ClearD3D11();

			// D3D11 should update when the console is open
			if (isConsoleOpen || !a_isGamePaused)
			{
				collisionHandler->Draw();
			}
		}
	}


	void DebugMenuHandler::DrawPeriodically(bool a_isGamePaused)
	{
		// Scaleform draws only need to run when game is running
		if (!a_isGamePaused)
		{
			drawHandler->ClearScaleform();
			boxHandler->Draw();
			cellHandler->Draw();
			navmeshHandler->Draw();
			markerHandler->Draw();
			//refInspectorHandler->Draw();
		}

		

		
		if (!MCM::settings::updateCollisionsEveryFrame)
		{
			// D3D11 should always clear, including when game is paused since it draws on top of the ui
			drawHandler->ClearD3D11();

			// D3D11 should update when the console is open
			if (isConsoleOpen || !a_isGamePaused)
			{
				collisionHandler->Draw();
			}
		}
		


		// DrawTest();
	}

	void DebugMenuHandler::DrawTest()
	{

	}


	RE::BSEventNotifyControl DebugMenuHandler::ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*)
	{
		if (a_event && a_event->menuName == "Console") isConsoleOpen = a_event->opening;
		return RE::BSEventNotifyControl::kContinue;
	}

	void DebugMenuHandler::OpenDrawMenu()
	{
		ScaleformUI::GetDrawMenu()->Open();
		drawHandler->Init();
	}

	void DebugMenuHandler::CloseDrawMenu()
	{
		ScaleformUI::GetDrawMenu()->Close();
		drawHandler->ClearScaleform();
		drawHandler->ClearD3D11();
		drawHandler->g_DrawMenu = nullptr;
	}

	bool DebugMenuHandler::isAnyDebugON()
	{
		return	MCM::settings::showCellBorders || 
				MCM::settings::showNavmesh || 
				MCM::settings::showOcclusion || 
				MCM::settings::showCoordinates || 
				MCM::settings::showMarkers ||
				MCM::settings::showCollision;

	}

	void DebugMenuHandler::ShowCoordinates()
	{
		if (ScaleformUI::GetDrawMenu()->IsOpen())
		{
			if (MCM::settings::showCoordinates)
			{
				ScaleformUI::UIHandler::GetSingleton()->ShowOverlayIfItsHidden();
				auto playerPosition = RE::PlayerCharacter::GetSingleton()->GetPosition();
				if (FreeCamHandler::GetSingleton()->IsCustomFreeCamEnabled())
				{
					RE::PlayerCamera::GetSingleton()->currentState->GetTranslation(playerPosition);
				}
				ScaleformUI::UIHandler::GetSingleton()->SetOverlayPosition(playerPosition.x, playerPosition.y, playerPosition.z);
			}
			else ScaleformUI::UIHandler::GetSingleton()->HideOverlayIfItsVisible();
		}
	}

	float DebugMenuHandler::GetLightLevel()
	{
		float nightMultiplier = 0.5;
		switch (MCM::settings::dayNightIndex)
		{
			case MCM::settings::LightMode::day: //day mode
			{
				return 1;
			}
			case MCM::settings::LightMode::night: // night mode
			{
				return nightMultiplier;
			}
			case MCM::settings::LightMode::autoMode:
			{
				// should be accurate for at least 100000000000 days
				float gameTime = RE::Calendar::GetSingleton()->GetCurrentGameTime(); // 0, 1, 2, ... at midnight, 0.5, 1.5, ... at noon
				float scale = 1 - nightMultiplier;
				float PI = 3.14159265359;
				return (-scale * cosf(2 * gameTime * PI) + 1 + nightMultiplier) / 2;
			}
		}
	}
}