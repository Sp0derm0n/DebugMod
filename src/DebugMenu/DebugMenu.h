#pragma once

#include "DrawHandler.h"
#include "UIHandler.h"
#include "OcclusionHandler.h"
#include "CellHandler.h"
#include "NavmeshHandler.h"
#include "MarkerHandler.h"
#include "CollisionHandler.h"
#include "RefInspectorHandler.h"
#include "InfoHandler.h"

namespace DebugMenu
{
	class DebugMenuHandler : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
	{
		public:
			std::unique_ptr<DrawHandler>			drawHandler;
			std::unique_ptr<UIHandler>				uiHandler;
			std::unique_ptr<OcclusionHandler>		occlusionHandler;
			std::unique_ptr<CellHandler>			cellHandler;
			std::unique_ptr<NavmeshHandler>			navmeshHandler;
			std::unique_ptr<MarkerHandler>			markerHandler;
			std::unique_ptr<CollisionHandler>		collisionHandler;
			std::unique_ptr<RefInspectorHandler>	refInspectorHandler;
			
			bool isDrawMenuOpen = false;
			bool isCoordinatesBoxVisible = false;
			bool hasDebugMenuBeenOpenedBefore = false;
			bool isDebugMenuClosed = true;
			
			void Init();
			void CloseAndReset();
			void Update();
			void ResetUpdateTimer();
			void DrawEveryFrame(bool a_isGamePaused);
			void DrawPeriodically(bool a_isGamePaused);
			void DrawTest();
			void OpenDrawMenu();
			void CloseDrawMenu();
			bool isAnyDebugON();

			RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*);


		private:
			bool isConsoleOpen = false;

			float timeSinceLastUpdate = 2.0f; // updates at least once per second, so 2 seconds means it updates next frame
			float* deltaTime = nullptr;

			void	ShowCoordinates();
			float	GetLightLevel();

	};

	std::unique_ptr<DebugMenuHandler>&	GetDebugMenuHandler();
	std::unique_ptr<DrawHandler>&		GetDrawHandler();
	std::unique_ptr<UIHandler>&			GetUIHandler();
	std::unique_ptr<OcclusionHandler>&	GetOcclusionHandler();
	std::unique_ptr<CellHandler>&		GetCellHandler();
	std::unique_ptr<NavmeshHandler>&	GetNavmeshHandler();
	std::unique_ptr<MarkerHandler>&		GetMarkerHandler();
	std::unique_ptr<CollisionHandler>&	GetCollisionHandler();
	std::unique_ptr<RefInspectorHandler>&	GetRefInspectorHandler();
	
}