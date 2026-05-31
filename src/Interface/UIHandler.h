#pragma once

#include "Interface/Menu.h"

namespace ScaleformUI
{
	// Interface handler connects the interface with the rest of the program
	// The interface files are otherwise standalone
	class UIHandler
	{
		using MOUSE_STATE = IElement::MOUSE_STATE;

		public:
			static UIHandler* GetSingleton()
			{
				static UIHandler singleton;
				return std::addressof(singleton);
			}

			std::unique_ptr<Menu> drawMenu;
			std::unique_ptr<Menu> debugMenuUI;

			void Init();
			void InitDebugMenuUI();
			void InitDrawMenu();

			

		private:
			const char* const skyrimBooksFont = "SkyrimBooks_Gaelic";
			const char* const skyrimFuturaFont = "Futura CondensedLight";
			const char* skyrimBooksFontTranslated = "SkyrimBooks_Gaelic"; // changes depending on language
			const char* skyrimFuturaFontTranslated = "Futura CondensedLight"; // changes depending on language

			void HandleLanguage();
			void BuildDebugMenu();
			void BuildDrawMenu();	

		public:
			// Element functions, sat when building menus
			std::function<void(float, float, float)>	SetOverlayPosition = nullptr;
			std::function<void()>						ShowOverlayIfItsHidden = nullptr;
			std::function<void()>						HideOverlayIfItsVisible = nullptr;	

			std::function<void()>						ShowInfoBoxIfItsHidden = nullptr;
			std::function<void()>						HideInfoBoxIfItsVisible = nullptr;
			std::function<void(std::string)>			SetInfoText = nullptr;
			std::function<bool()>						IsInfoBoxOpen = nullptr;
			std::function<void()>						SetTFCButtonCorrectOnOffIcon = nullptr;
	};

	static inline Menu* GetDrawMenu()
	{
		return UIHandler::GetSingleton()->drawMenu.get();
	}

	static inline Menu* GetDebugMenuUI()
	{
		return UIHandler::GetSingleton()->debugMenuUI.get();
	}
}