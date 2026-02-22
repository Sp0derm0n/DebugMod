#pragma once

#include "DebugUIMenu.h"

class UIHandler
{
	using BUTTON = DebugMenuUI::BUTTON;
	using BUTTON_TYPE = DebugMenuUI::Button::BUTTON_TYPE;
	using BUTTON_STATE = DebugMenuUI::Button::BUTTON_STATE;
	using ChildFlags = DebugMenuUI::Button::ChildFlags;
	using MenuItems = DebugMenuUI::MenuItems;
	using Button = DebugMenuUI::Button;

	public:
		RE::GPtr<DebugMenuUI> g_DebugMenu;
		RE::MenuCursor* g_Cursor;

		/*static UIHandler* GetSingleton()
		{
			static UIHandler singleton;
			return std::addressof(singleton);
		}*/

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