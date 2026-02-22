#include "DebugMenu.h"
#include "MCM.h"

namespace DebugMenu
{
	static auto debugMenuHandler = std::make_unique<DebugMenuHandler>();

	std::unique_ptr<DebugMenuHandler>& GetDebugMenuHandler() { return debugMenuHandler; }

	std::unique_ptr<DrawHandler>& GetDrawHandler() { return debugMenuHandler->drawHandler; }
	std::unique_ptr<UIHandler>& GetUIHandler() { return debugMenuHandler->uiHandler; }
	std::unique_ptr<OcclusionHandler>& GetOcclusionHandler() { return debugMenuHandler->occlusionHandler; }
	std::unique_ptr<CellHandler>& GetCellHandler() { return debugMenuHandler->cellHandler; }
	std::unique_ptr<NavmeshHandler>& GetNavmeshHandler() { return debugMenuHandler->navmeshHandler; }
	std::unique_ptr<MarkerHandler>& GetMarkerHandler() { return debugMenuHandler->markerHandler; }
	std::unique_ptr<CollisionHandler>& GetCollisionHandler() { return debugMenuHandler->collisionHandler; }
	std::unique_ptr<RefInspectorHandler>& GetRefInspectorHandler() { return debugMenuHandler->refInspectorHandler; }

	void DebugMenuHandler::Init()
	{
		drawHandler = std::make_unique<DrawHandler>();
		uiHandler = std::make_unique<UIHandler>();

		occlusionHandler = std::make_unique<OcclusionHandler>();
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
		isDebugMenuClosed = true;
		markerHandler->Reset();
		collisionHandler->Reset();
		DrawMenu::CloseMenu();
		DebugMenuUI::CloseMenu();
	}

	void DebugMenuHandler::Update()
	{
		if (!Utils::IsPlayerLoaded()) return;

		if (uiHandler && uiHandler->isMenuOpen)
		{
			uiHandler->Update();
		}

		// Must come after uiHandler update
		if (isDebugMenuClosed) return;


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
			drawHandler->Update(*deltaTime);
			DrawEveryFrame(isGamePaused);

		}


		//if (isGamePaused) return;
		
		// has to be lowest in this function
		if (drawHandler && drawHandler->isMenuOpen == false)
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
			occlusionHandler->Draw();
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
		DrawMenu::OpenMenu();
		auto uiTask = [&]()
		{
			drawHandler->Init();
		};
		SKSE::GetTaskInterface()->AddUITask(uiTask);
	}

	void DebugMenuHandler::CloseDrawMenu()
	{
		DrawMenu::CloseMenu();
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
		if (!drawHandler->isMenuOpen) return;// should only try to open the coordinatebox whe the drawMenu is open

		if (MCM::settings::showCoordinates)
		{
			if (!isCoordinatesBoxVisible)
			{
				drawHandler->g_DrawMenu->ShowBox("coordinatesBox");
				isCoordinatesBoxVisible = true;
			}
			RE::NiPoint3 playerPosition = RE::PlayerCharacter::GetSingleton()->GetPosition();
			drawHandler->g_DrawMenu->SetCoordinates(playerPosition.x, playerPosition.y, playerPosition.z);
		}
		else if (isCoordinatesBoxVisible)
		{
			drawHandler->g_DrawMenu->HideBox("coordinatesBox");
			isCoordinatesBoxVisible = false;
		}
	}

	float DebugMenuHandler::GetLightLevel()
	{
		float nightMultiplier = 0.5;
		switch (MCM::settings::dayNightIndex)
		{
			case 0: //day mode
			{
				return 1;
			}
			case 1: // night mode
			{
				return nightMultiplier;
			}
			case 2:
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