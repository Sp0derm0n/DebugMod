#pragma once

#include "GroupElement.h"
#include "Button.h"
#include "Textfield.h"

namespace ScaleformUI
{
	class Menu;
	using MenuCallback = std::function<void(Menu*)>;
	using ELEMENT_TYPE = Element::ELEMENT_TYPE;

	class [[nodiscard]] Menu
	{
		public:
			constexpr static const char* TEXT_FIELD_CLASS_NAME = "TextField";
			constexpr static const char* TEXT_FIELD_INSTANCE_NAME = "textField";

			struct OpenConditions
			{
				std::function<bool()> conditionsFunction = nullptr;
				uint32_t openKeyCode = 0x0;
			};

			RE::GPtr<RE::IMenu> iMenu = nullptr;
			std::vector<ElementPTR> rootElements{};

			Menu(const char* a_menuName);

			void Build(MenuCallback a_attachNewCanvasElements);
			bool IsReady();
			bool IsOpen() const;
			bool IsClosed() const;
			void Update(float a_delta);
			void ToggleMenu();
			void Open();
			void Close();
			void SetMenuOpenKeyCode(uint32_t a_keyCode) { openMenuKeyCode = a_keyCode; }
			void SetOpenMenuFunction(std::function<bool()> a_fn) { openMenuFunction = a_fn; }
			void SetCloseMenuFunction(std::function<void()> a_fn) { closeMenuFunction = a_fn; }
			
			void LockMenu() { isMenuLocked = true; }
			void UnlockMenu() { isMenuLocked = false; }
			bool IsMenuLocked() { return isMenuLocked; }

			const char*	GetMenuName() { return menuName.c_str(); }
			uint32_t	GetMenuOpenKeyCode() { return openMenuKeyCode; }

			// Register existing UI element
			void RegisterUIElement(ElementPTR& a_element);
			void RegisterUIElement(std::string a_instanceName, ELEMENT_TYPE a_type = ELEMENT_TYPE::kNONE);
			
			// Create and attach new UI elements to the canvas
			// A similar method is also reachable from Element objects
			IElement* AttachUIElement(std::string a_linkageName, ELEMENT_TYPE a_type = ELEMENT_TYPE::kNONE);
			IElement* AttachUIElement(std::string a_linkageName, std::string a_instanceName, ELEMENT_TYPE a_type = ELEMENT_TYPE::kNONE);
			IElement* AttachUIElement(std::string a_linkageName, std::string a_instanceName, ELEMENT_TYPE a_type, Element* a_parentElement);
			IElement* CreateGroup(std::string a_instanceName, Element* a_parentElement);
			IElement* CreateGroup(std::string a_instanceName);
			IElement* CreateSquareElement(std::string a_instanceName, uint32_t a_color, ELEMENT_TYPE a_type = ELEMENT_TYPE::kNONE);
			IElement* CreateSquareElement(std::string a_instanceName, uint32_t a_color, ELEMENT_TYPE a_type, Element* a_parentElement);
			IElement* CreateEmptyUIElement(std::string a_instanceName, ELEMENT_TYPE a_type = ELEMENT_TYPE::kNONE);
			IElement* CreateEmptyUIElement(std::string a_instanceName, ELEMENT_TYPE a_type, Element* a_parentElement);

			IElement* CreateTextfield(std::string a_instanceName, Element* a_parentElement);
			IElement* CreateTextfield(std::string a_instanceName);

			IElement* GetElementByName(std::string a_instanceName);

			// Set default format for text fields
			void SetDefaultFont(const char* a_font); // use PrintFontList for list of font names
			void SetDefaultFontSize(uint32_t a_size);
			void SetDefaultFontColor(uint32_t a_defaultColor); 
			void SetDefaultBold(bool a_enabled);
			void SetDefaultItalic(bool a_enabled);
			void SetDefaultUnderline(bool a_enabled);
			void SetDefaultTextAlign(const char* a_align); // "left", "center", "right", "justify"
			void SetDefaultIndent(float a_indent); // indentation of first line
			void SetDefaultBlockIndent(float a_indent); // indentation of entire block of text
			void SetDefaultBullet(bool a_enabled); // bulleted list
			void SetDefaultKerning(bool a_enabled); // Google Kerning
			void SetDefaultLeftMargin(float a_margin);
			void SetDefaultRightMargin(float a_margin);
			void SetDefaultLetterSpacing(float a_spacing); // unknown unit
			void SetDefaultLineSpacing(float a_spacing); // In pixels	
			void SetDefaultAutoSize(bool a_enabled);
			void SetDefaultWordWrap(bool a_enabled);

			// Set logical parent/child relationship for all registered elements
			void CreateHierarchy();
			void GetRoot(RE::GFxValue& a_root);

			float ScreenW() { return xResolution; }
			float ScreenH() { return yResolution; }

			void TraverseUIElements(std::function<void(Element*)> a_callback);
			void SetMenuCloseCallback(std::function<void()> a_callback) { onMenuCloseCallback = a_callback; }
			

		private:
			bool		isReady = false;
			bool		queueClose = false;

			uint32_t	openMenuKeyCode = 0x0;
			// Canvas is automatically rescaled to fit the game's resolution
			// 0,0 is the top left corner
			float		xResolution = 0.0f;
			float		yResolution = 0.0f;
			std::string menuName;
			bool		usesCursor = false;
			bool		isMenuLocked = false;

			// Text field defaults
			RE::GFxValue	defaultTextFormat;
			bool			textFieldDefaultAutoSize = true;
			bool			textFieldDefaultWordWrap = false;

			std::function<void()> onMenuCloseCallback = nullptr;
			std::function<bool()> openMenuFunction = nullptr; // will only open if function returns true
			std::function<void()> closeMenuFunction = nullptr;
			
			ElementPTR	CreateUIElement(std::string a_instanceName, ELEMENT_TYPE a_type);
			void			PrintHierarchy();
			void			OpenIMenu();
			void			CloseIMenu();
			bool			HasAnimationPlaying();
			bool			DoesOtherMenuUseCursor();
			void			DisableCursorIfNecessary();
			void			ConstructDefaultTextFormat();
			void			UpdateAnimation(float a_delta);
			bool			IsCurrentlyClosing();


		public:
			// Debug methods
			
			void DrawLine(RE::NiPoint2 a_start, RE::NiPoint2 a_end, float a_thickness=10.0f, uint32_t a_color = 0xFFFFFF, uint32_t a_alpha=100);
			void DrawGrid(uint32_t a_horizontalGridLines, uint32_t a_verticalGridLines, float a_thickness = 10.0f, uint32_t a_color = 0xFFFFFF, uint32_t a_alpha = 100);
			void DrawSquare(float a_xmin, float a_ymin, float a_xmax, float a_ymax, float a_thickness = 10.0f, uint32_t a_color = 0xFFFFFF, uint32_t a_alpha = 100);
			void DrawObjectBounds(uint32_t a_color = 0xFF0000);
			void DrawGroupBounds(uint32_t a_color = 0x00FF00);
			void DrawInteractableBounds(uint32_t a_color = 0xFF0000);
			void DrawDebugInfo();
			void ClearDebugDraws();
			void PrintFontList();
			bool HasDebugDraws() const;

		private:
			struct DebugDrawParameters
			{
				bool draw = false;
				uint32_t color = 0xFF0000;

				constexpr operator bool() const { return draw; }
			};
			DebugDrawParameters drawObjects{};
			DebugDrawParameters drawGroups{};
			DebugDrawParameters drawInteractables{};

			std::string	DebugElementInstanceName;

			void		GetDebugElement(RE::GFxValue& a_elementOut);

			
	};
}