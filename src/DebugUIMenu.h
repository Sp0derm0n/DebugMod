#pragma once

class DebugMenu : public RE::IMenu
{
	public:
		constexpr static const char* MENU_PATH = "DebugMenu";
		constexpr static const char* MENU_NAME = "DebugMenu";

		enum BUTTON
		{
			kDayNight,
			kCellBorder,
			kCellWalls,
			kCellQuads,
			kNavMesh,
			kNavMeshPlus,
			kNavMeshMinus,
			kNavMeshMode,
			kOcclusion,
			kOcclusionPlus,
			kOcclusionMinus,
		};

		enum class BUTTON_STATE
		{
			kHOVER,
			kHIT,
			kON,
			kOFF,
		};

		struct MenuButton
		{
			float xMin;
			float xMax;
			float yMin;
			float yMax;
			const char* name;
			BUTTON type;
			BUTTON_STATE state;
			bool isActive;
		};

		struct MenuItems
		{
			std::vector<MenuButton> buttons;
		};

		MenuItems menuItems;

		void Init();
		void RegisterButton(const char* a_buttonName, BUTTON a_buttonType);
		void ShowMenu();
		void GetIcon(const char* a_iconName, RE::GFxValue& a_icon);
		void SetText(const char* a_textBoxName, float a_range);
		void ButtonMethod(const char* a_buttonName, const char* a_methodName);

		MenuItems GetMenuItems();


		RE::GPtr<RE::GFxMovieView> movie;

		DebugMenu();

		static void Register();
		static void OpenMenu();
		static void CloseMenu();

		RE::UI_MESSAGE_RESULTS ProcessMessage(RE::UIMessage& a_message);
		
		static RE::stl::owner<RE::IMenu*> Creator() { return new DebugMenu(); }


	private:
		class Logger : public RE::GFxLog
		{
			public:
			void LogMessageVarg(LogMessageType, const char* a_fmt, std::va_list a_argList) override
			{
				std::string fmt(a_fmt ? a_fmt : "");
				while (!fmt.empty() && fmt.back() == '\n') {
					fmt.pop_back();
				}

				std::va_list args;
				va_copy(args, a_argList);
				std::vector<char> buf(static_cast<std::size_t>(std::vsnprintf(0, 0, fmt.c_str(), a_argList) + 1));
				std::vsnprintf(buf.data(), buf.size(), fmt.c_str(), args);
				va_end(args);

				logger::info("{}"sv, buf.data());
			}
		};

		void GetRoot(RE::GFxValue& root);

};

