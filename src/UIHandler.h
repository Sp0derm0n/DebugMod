#pragma once

#include "DebugUIMenu.h"

class UIHandler
{
	using BUTTON = DebugMenu::BUTTON;
	using BUTTON_STATE = DebugMenu::BUTTON_STATE;
	using MenuItems = DebugMenu::MenuItems;
	using MenuButton = DebugMenu::MenuButton;

	public:
		RE::GPtr<DebugMenu> g_DebugMenu;
		RE::MenuCursor* g_Cursor;

		static UIHandler* GetSingleton()
		{
			static UIHandler singleton;
			return std::addressof(singleton);
		}

		MenuItems menuItems;

		float canvasWidth;
		float canvasHeight;
		float cursorWidthInPercentage;
		float cursorHeightInPercentage;

		bool isMenuOpen;

		bool mouseReleased;
		bool mousePressed;

		void Init();
		void Unload();
		void GetDebugMenu();
		void Update();
		void ProcessButtonClick(BUTTON a_button, bool a_isActive);





};