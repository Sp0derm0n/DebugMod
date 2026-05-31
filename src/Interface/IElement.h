#pragma once


//#define LOG_UI
//#define PRINT_MENU_HEIRARCHY
//#define DISPLAY_UI_ON_MAINMENU // Sets depth priority high enough to display on top the main menu, can cause cursor issues

#include "BoundingBox.h"
#include "AnimationHandler.h"

namespace ScaleformUI
{
	class IElement;
	class Element;
	class Group;
	class Button;
	class Textfield;

	using ElementPTR = std::unique_ptr<IElement>;
	using UIFunction = std::function<void(Element*)>;
	
	using AnimationFunction = std::function<void(AnimationHandler*)>;

	class IElement
	{
		public:
			constexpr static const char* LAYOUT_NAME = "layout";
			constexpr static const char* TOGGLE_ON_NAME = "onIcon";
			constexpr static const char* TOGGLE_OFF_NAME = "offIcon";

			enum class ELEMENT_TYPE : uint16_t
			{
				kNONE = 0,
				kGROUP = 1,
				kBUTTON = 2,
				kTEXTFIELD = 3
			};

			enum class MOUSE_STATE
			{
				kNONE = 0,
				kHOVER = 1 << 0,
				kPRESSED = 1 << 1,
				kCHECKED = 1 << 2,
			};

			struct Direction
			{
				uint16_t value;
				constexpr Direction(uint16_t a_value) : value(a_value) {}
				operator Alignment() const;
				constexpr operator int() const { return value; }
				const bool operator==(Direction a_other) const { return value == a_other.value; }

				static const Direction left, right, top, bottom;
			};

			// MOUSE_STATE is the mouse state that has changed; bool is if it is sat or reset
			using UIOnMouseStateFunction = std::function<void(Element*, IElement::MOUSE_STATE, bool)>;

			virtual ~IElement() = default;

			// Create new UI elements as children
			virtual IElement* AttachUIElement(std::string a_linkageName, ELEMENT_TYPE a_type = ELEMENT_TYPE::kNONE) = 0;
			virtual IElement* AttachUIElement(std::string a_linkageName, std::string a_instanceName, ELEMENT_TYPE a_type = ELEMENT_TYPE::kNONE) = 0;
			virtual IElement* CreateGroup(std::string a_instanceName) = 0;
			virtual IElement* CreateTextField(std::string a_instanceName) = 0;
			virtual IElement* CreateEmptyUIElement(std::string a_instanceName, ELEMENT_TYPE a_type = ELEMENT_TYPE::kNONE) = 0;
			virtual IElement* CreateSquareBBox() = 0;
			virtual IElement* CreateSquareElement(std::string a_instanceName, uint32_t a_color, ELEMENT_TYPE a_type = ELEMENT_TYPE::kNONE) = 0;

			virtual std::vector<ElementPTR>& GetChildren() = 0;

			virtual void Show() = 0;
			virtual void Hide() = 0;
			virtual void ToggleShowHide() = 0;
			virtual void SetIgnoreMenuLock(bool a_ignore) = 0;

			virtual void SetVisible() = 0;
			virtual void SetInvisible() = 0;
			virtual bool IsVisible() const = 0;

			virtual void SetMask(IElement& a_maskElement) = 0;
			virtual void MoveToFront() = 0; // Move element to the current highest layer

			virtual void SetOnMouseStateChange(UIOnMouseStateFunction a_callback) = 0;
			virtual void SetOnChecked(std::function<void(Element*, bool)> a_callback) = 0; // bool = isChecked
			virtual void SetOnChecked(std::function<void(bool)> a_callback) = 0; // bool = isChecked
			virtual void SetWhileHoverCallback(std::function<void(float)> a_callback) = 0; // float = hover duration with no mouse movement
			virtual	void SendCheckedEventWhenMenuOpens(bool a_yes) = 0;

			// Animations
			virtual void SetHoverAnimation(AnimationFunction a_fn) = 0;
			virtual void SetPressedAnimation(AnimationFunction a_fn) = 0;
			virtual void SetResetAnimation(AnimationFunction a_fn) = 0;
			virtual void SetAnimationResetDuration(float a_duration) = 0;
			virtual void SetShowAnimation(AnimationFunction a_fn) = 0;
			virtual void SetHideAnimation(AnimationFunction a_fn) = 0;

			// Translation
			virtual void SetX(Size a_x) = 0;
			virtual void SetY(Size a_y) = 0;
			virtual void SetX(float a_x) = 0;
			virtual void SetY(float a_y) = 0;
			virtual void SetPos(Size a_x, Size a_y) = 0;
			virtual void SetPos(float a_x, Size a_y) = 0;
			virtual void SetPos(Size a_x, float a_y) = 0;
			virtual void SetPos(float a_x, float a_y) = 0;
			virtual void Move(Size a_x, Size a_y) = 0;
			virtual void Move(Size a_x, float a_y) = 0;
			virtual void Move(float a_x, Size a_y) = 0;
			virtual void Move(float a_x, float a_y) = 0;

			virtual float GetX() const = 0;
			virtual float GetY() const = 0;
			

			// Scaling
			virtual void SetXScale(Size a_x) = 0;
			virtual void SetYScale(Size a_y) = 0;
			virtual void SetXScale(float a_x) = 0;
			virtual void SetYScale(float a_y) = 0;
			virtual void SetScale(Size a_x, Size a_y) = 0;
			virtual void SetScale(float a_x, Size a_y) = 0;
			virtual void SetScale(Size a_x, float a_y) = 0;
			virtual void SetScale(float a_x, float a_y) = 0;
			virtual void SetScale(Size a_scale) = 0;
			virtual void SetScale(float a_scale) = 0;
			virtual void LockAspect() = 0;
			virtual void UnlockAspect() = 0;
			virtual void SetWidth(Size a_width) = 0;
			virtual void SetHeight(Size a_height) = 0;
			virtual void SetWidth(float a_width) = 0;
			virtual void SetHeight(float a_height) = 0;

			virtual float GetYScale() const = 0;
			virtual float GetXScale() const = 0;
			virtual float GetWidth() const = 0;
			virtual float GetHeight() const = 0;

			//Alignment
			virtual void AlignInsideParent(Alignment a_align) = 0;
			virtual void AlignInsideParent(Alignment a_align1, Alignment a_align2) = 0;
			virtual void AlignOutsideParent(Alignment a_align) = 0;
			virtual void AlignOutsideParent(Alignment a_align1, Alignment a_align2) = 0;
			virtual void AlignInsideOther(IElement* a_element, Alignment a_align) = 0;
			virtual void AlignInsideOther(IElement* a_element, Alignment a_align1, Alignment a_align2) = 0;
			virtual void AlignOutsideOther(IElement* a_element, Alignment a_align) = 0;
			virtual void AlignOutsideOther(IElement* a_element, Alignment a_align1, Alignment a_align2) = 0;
			virtual void RemoveAlignment() = 0;
			virtual void SetPadding(Direction a_direction, Size a_amount) = 0;;
			virtual void SetPadding(Direction a_direction, float a_amount) = 0;

			// Misc
			virtual void SetAlpha(uint32_t a_alpha) = 0;
			virtual void SetLayoutAlpha(uint32_t a_alpha) = 0;
			virtual bool IsHovering() const = 0;
			virtual bool IsPressed() const = 0;
			virtual bool IsChecked() const = 0;
			virtual void BlockChildInteractions() = 0;
			virtual void AllowChildInteractions() = 0;
			virtual RE::GFxValue GetGFx() = 0;


			// Button methods
			virtual void SetToggleValuePtr(bool* a_toggleValue) = 0;
			virtual void SetCyclicValuePtr(uint32_t* a_cyclicValue) = 0;
			virtual void SetCorrectToggleValue() = 0; // Set toggle value to match the buttons Checked status
			virtual void CycleValue() = 0; // Cycle value saved in Cyclic button
			virtual void PrintOrderOfCyclicImages() = 0;

			// Group methods
			virtual void AlignChildrenHorizontally(Size a_spacing, Alignment a_align = Alignment::kTop) = 0;
			virtual void AlignChildrenVertically(Size a_spacing, Alignment a_align = Alignment::kLeft) = 0;
			virtual void SetVerticalScrollable(bool a_enable) = 0;
			virtual void SetHorizontalScrollable(bool a_enable) = 0;
			virtual void SetScrollableArea(IElement* a_element) = 0; // Do not set group as a scrollable area: group bounds are determined by its children
			virtual void SetScrollableTopRatio(float a_ratio) = 0;
			virtual void SetScrollableBottomRatio(float a_ratio) = 0;
			virtual void SetOnScrollCallback(std::function<void(float)> a_callback) = 0; // The float argument is the scroll ratio (from 0 to 1)

			// Text field methods
			virtual void SetText(const char* a_text) = 0;
			virtual void SetText(std::string a_text) = 0;
			virtual void SetFont(const char* a_font) = 0; // use PrintFontList for list of font names
			virtual void SetFontSize(uint32_t a_size) = 0;
			virtual void SetFontColor(uint32_t a_defaultColor) = 0;
			virtual void SetBold(bool a_enabled) = 0;
			virtual void SetItalic(bool a_enabled) = 0;
			virtual void SetUnderline(bool a_enabled) = 0;
			virtual void SetTextAlign(const char* a_align) = 0; // "left", "center", "right", "justify"
			virtual void SetIndent(float a_indent) = 0; // indentation of first line
			virtual void SetBlockIndent(float a_indent) = 0; // indentation of entire block of text
			virtual void SetBullet(bool a_enabled) = 0; // bulleted list
			virtual void SetKerning(bool a_enabled) = 0; // Google Kerning
			virtual void SetLeftMargin(float a_margin) = 0;
			virtual void SetRightMargin(float a_margin) = 0;
			virtual void SetLetterSpacing(float a_spacing) = 0; // unknown unit
			virtual void SetLineSpacing(float a_spacing) = 0; // In pixels	
			virtual void SetAutoSize(bool a_enabled) = 0;
			virtual void SetWordWrap(bool a_enabled) = 0;
			virtual void SetTextfieldWidth(Size a_width) = 0;
			virtual void SetTextfieldHeight(Size a_height) = 0;
			virtual void SetTextfieldWidth(float a_width) = 0;
			virtual void SetTextfieldHeight(float a_height) = 0;
			
			virtual Element* AsUIElement() = 0;

	};
}