#pragma once

#include "DebugUIMenu.h"

class UIHandler
{
	using BUTTON = DebugMenu::BUTTON;
	using BUTTON_TYPE = DebugMenu::Button::BUTTON_TYPE;
	using BUTTON_STATE = DebugMenu::Button::BUTTON_STATE;
	using ChildFlags = DebugMenu::Button::ChildFlags;
	using MenuItems = DebugMenu::MenuItems;
	using Button = DebugMenu::Button;

	public:
		RE::GPtr<DebugMenu> g_DebugMenu;
		RE::MenuCursor* g_Cursor;

		static UIHandler* GetSingleton()
		{
			static UIHandler singleton;
			return std::addressof(singleton);
		}

		MenuItems* menuItems;

		float canvasWidth;
		float canvasHeight;
		float cursorWidthInPercentage;
		float cursorHeightInPercentage;

		float maskMinX;
		float maskMaxX;
		float maskMinY;
		float maskMaxY;

		float scrollDistance = 50.0f;

		bool isMenuOpen;

		bool mouseReleased;
		bool mousePressed;
		bool mouseScrollUp;
		bool mouseScrollDown;
		bool keyScrollUp;
		bool keyScrollDown;

		void Init();
		void Unload();
		void GetDebugMenu();
		void Update();
		void ProcessButtonClick(std::shared_ptr<Button>& a_button);
		void UpdateChildButtonsState(std::shared_ptr<Button>& a_parentButton);
		void UpdateParentButtonState(std::shared_ptr<Button>& a_childButton);

		bool InitializeToggleButtonOnMenuOpen(const char* a_buttonName);

		// not used
		void SetNewRange(float& a_range, bool a_increase);





};