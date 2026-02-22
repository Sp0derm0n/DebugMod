#pragma once

#include "MCM.h"

class DebugMenuUI : public RE::IMenu
{
	public:
		constexpr static const char* MENU_PATH = "DebugMenu";
		constexpr static const char* MENU_NAME = "DebugMenu";

		class Menu;
		class Button;
		class TextBox;

		using MenuPtr = std::shared_ptr<Menu>;
		using ButtonActionFunction = std::function<void(Button*)>;
		using ButtonPtr = std::shared_ptr<Button>;
		using TextBoxPtr = std::shared_ptr<TextBox>;

		bool& logUI = MCM::settings::logUI;

		enum BUTTON // Register button in Init()
		{
			kDayNight,
			kCellBorder,
			kCellWalls,
			kCellQuads,
			kNavMesh,
			kNavMeshPlus,
			kNavMeshMinus,
			kNavMeshMode,
			kNavMeshTriangles,
			kNavMeshCover,
			kOcclusion,
			kOcclusionPlus,
			kOcclusionMinus,
			kCoordinates,
			kMarkers,
			kMarkersPlus,
			kMarkersMinus,
			kSelectMarkers
		};

		enum MENU
		{
			kMainMenu,
			kSelectMarkersMenu
		};
		
		struct Menu 
		{
			//Menu(float a_x, float a_y, bool a_hasMask, float a_maskXMin, float a_maskXMax, float a_maskYMin, float a_maskYMax)

			uint32_t layer;

            float x;
            float y;

			float xMin;
			float xMax;
			float yMin;
			float yMax;

			bool hasMask;

			float maskXMin;
			float maskXMax;
			float maskYMin;
			float maskYMax;
			float maskHeight;

			float maskItemsDefaultY;
			float maskItemsCurrentY;
			float maskItemsHeight;

            std::string name;

			bool isActive = true;

			std::vector<MenuPtr> menus;
			std::vector<ButtonPtr> buttons;
			std::vector<TextBoxPtr> textBoxes;


			const char* GetName() { return name.c_str(); }
        };



		class Button 
		{
			public: 
				enum class BUTTON_TYPE
				{
					kToggle,
					kClick,
					kCyclic
				};

				enum class BUTTON_STATE
				{
					kHOVER,
					kHIT,
					kON,
					kOFF,
				};
				
				enum class ChildFlags
				{
					kNone = 0,
					kInteractable = 1 << 0
				};

				Button(float a_xMin, float a_xMax, float a_yMin, float a_yMax, std::string a_name, BUTTON_TYPE a_buttonType, MenuPtr a_parentMenu, bool a_isInMask, ButtonActionFunction a_buttonAction);


				float xMin;
				float xMax;
				float yMin;
				float yMax;

				std::string name;

				BUTTON_TYPE type;
				BUTTON_STATE state = BUTTON_STATE::kOFF;

				bool isActive = false;
				bool isEnabled = true;
				bool isInMask;

				MenuPtr parentMenu;

				Button* parent = nullptr;
				std::vector<ButtonPtr> children;

				ChildFlags childFlags = ChildFlags::kNone;

				void ButtonAction();

				void AddChild(ButtonPtr& a_child);
				void SetChildFlag(ChildFlags a_flag);

				bool HasChildFlag(ChildFlags a_flag);
				bool IsChildButton() { return parent ? true : false; }

				const char* GetName() const { return name.c_str(); }

			private: 
				ButtonActionFunction ButtonAction_;


		};

		struct TextBox
		{
			TextBox(std::string a_name, bool a_isInMask, MenuPtr a_parentMenu) : 
				name(a_name), 
				isInMask(a_isInMask), 
				parentMenu(a_parentMenu) 
			{}

            std::string name;
			bool isInMask;
			MenuPtr parentMenu;

			const char* GetName() { return name.c_str(); }

		};
		
		using BUTTON_STATE = Button::BUTTON_STATE;
		using BUTTON_TYPE = Button::BUTTON_TYPE;
		using ChildFlags = Button::ChildFlags;
		


		struct MenuItems
		{
			std::vector<MenuPtr> menus;
			std::vector<ButtonPtr> buttons;
			std::vector<TextBoxPtr> textBoxes;

			MenuPtr		GetMenuByName(const std::string& a_name);
			ButtonPtr	GetButtonByName(const std::string& a_name);
			TextBoxPtr	GetTextBoxByName(const std::string& a_name);
		};

		struct MarkerLabel
		{
			std::string name;
			bool isHeadline;
			bool* setting;
			ButtonPtr button;
		};


		/*float menuItemsDefaultY;
		float menuItemsCurrentY;
		float menuItemsYLocalOffset;
		float menuItemsHeight;

		float maskMinX;
		float maskMaxX;
		float maskMinY;
		float maskMaxY;
		float maskHeight;*/

		MenuItems menuItems;
		std::vector<MarkerLabel> selectableMarkers; // string = label name, bool = headline? 

		void		Init();
		void		OrderMenuItems();
		void		InitButtonActionFunctions();
		void		InitSelectableMarkerButtons();
		void		AddMarkerLabel(const std::string& a_labelName, bool a_isHeadline, bool* setting);
		void		ShowMenu(const char* a_menuName);
		void		HideMenu(const char* a_menuName);
        void		GetIcon(const ButtonPtr& a_button, RE::GFxValue& a_icon);
		void		GetIcon(const Button* a_button, RE::GFxValue& a_icon);
		void		GetIcon(const TextBoxPtr& a_button, RE::GFxValue& a_icon);
		void		GetMenu(const MenuPtr& a_menu, RE::GFxValue& a_menuOut);
		void		SetText(const TextBoxPtr& a_textBox, const std::string& a_text);
		void		SetText(const TextBoxPtr& a_textBox, float a_range);
		void		SetText(const char* a_textBoxName, const std::string& a_text);
		void		SetText(const char* a_textBoxName, float a_range);
		void		ScrollMenu(float a_distance, MenuPtr& a_menu);
		void		SetNewRange(float& a_range, bool a_increase);
		void		UpdateMarkersSelectionButtons();
		void		DisableD3DWarnings();

		bool		ButtonActionScriptMethod(const ButtonPtr& a_button, const char* a_methodName, RE::GFxValue a_args = nullptr, uint32_t a_numArgs = 0);
		bool		ButtonActionScriptMethod(const Button* a_button, const char* a_methodName, RE::GFxValue a_args = nullptr, uint32_t a_numArgs = 0);
        
		MenuPtr		RegisterMenu(const char* a_menuName, uint32_t a_layer, MenuPtr a_parentMenu = nullptr );
		ButtonPtr	RegisterButton(const char* a_buttonName, BUTTON_TYPE a_buttonType, MenuPtr a_parentMenu, ButtonActionFunction a_buttonAction);
		TextBoxPtr	RegisterTextBox(const char* a_textBoxName, MenuPtr a_parentMenu);

		MenuItems*	GetMenuItems();


		ButtonActionFunction DayNightAction;
		ButtonActionFunction CloseMenuAction;
		ButtonActionFunction CellBorderAction;
		ButtonActionFunction CellWallsAction;
		ButtonActionFunction CellQuadsAction;
		ButtonActionFunction NavMeshAction;
		ButtonActionFunction NavMeshPlusAction;
		ButtonActionFunction NavMeshMinusAction;
		ButtonActionFunction NavMeshModeAction;
		ButtonActionFunction NavMeshTrianglesAction;
		ButtonActionFunction NavMeshCoverAction;
		ButtonActionFunction OcclusionAction;
		ButtonActionFunction OcclusionPlusAction;
		ButtonActionFunction OcclusionMinusAction;
		ButtonActionFunction CoordinatesAction;
		ButtonActionFunction MarkersAction;
		ButtonActionFunction MarkersPlusAction;
		ButtonActionFunction MarkersMinusAction;
		ButtonActionFunction SelectMarkersAction;
		ButtonActionFunction LoadPreset1MarkersAction;
		ButtonActionFunction LoadPreset2MarkersAction;
		ButtonActionFunction LoadPreset3MarkersAction;
		ButtonActionFunction LoadPresetAction;
		ButtonActionFunction CancelLoadPresetAction;
		ButtonActionFunction SavePreset1MarkersAction;
		ButtonActionFunction SavePreset2MarkersAction;
		ButtonActionFunction SavePreset3MarkersAction;
		ButtonActionFunction SavePresetAction;
		ButtonActionFunction CancelSavePresetAction;
		ButtonActionFunction CollisionAction;
		ButtonActionFunction CollisionPlusAction;
		ButtonActionFunction CollisionMinusAction;
		ButtonActionFunction CollisionDisplayAction;
		ButtonActionFunction ClearSelectedRefsAction;
		ButtonActionFunction CollisionRenderAction;
		ButtonActionFunction CollisionShowNPCsAction;




		RE::GPtr<RE::GFxMovieView> movie;

		DebugMenuUI();

		static void Register();
		static void OpenMenu();
		static void CloseMenu();

		RE::UI_MESSAGE_RESULTS ProcessMessage(RE::UIMessage& a_message);
		
		static RE::stl::owner<RE::IMenu*> Creator() { return new DebugMenuUI(); }


	private:
		uint32_t currentMarkerPresetIndex = 1;


		void GetIcon_(const char* a_iconName, const char* a_parentMenuName, bool a_isInMask, RE::GFxValue& a_icon);
		void GetRoot(RE::GFxValue& root);




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

				logger::debug("{}"sv, buf.data());
			}
		};


};

