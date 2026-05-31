#pragma once


// UI Elements must be designed using this structure:
// 
// ElementMovieClip
//   |--layout				(required; this single MovieClip must contain only 1 symbol, and it must be named 'layout'. this symbol must be with its top left corner at the transformation point)
//	     |--bounding box	(optional; must have instance name 'boundingBox' if it exists)
//       |--graphics		(the graphics of the UI element, can be distributed on several layers, doesn't have to contain symbols)
//
// 
// When parenting elements, this is the following structure:
// 
// ParentMovieClip
//   |--layout
//       |--bounding box
//       |--graphics
//   |--ChildMovieClip1
//        |--layout
//            |-bounding box
//            |-graphics
//   |--ChildMovieClip2
//        |--layout
//            |-bounding box
//            |-graphics
//   |
//   .
//   .
//   .
// 
//	By implementing the layout MovieClips, aligning the graphics and bounding box is much easier, since
//  the 'Align' menu in flash can be used
//	Use the included debug draws to draw layout outlines and bbox outlines
//

#include "IElement.h"

namespace ScaleformUI
{

	#ifdef LOG_UI
		extern int UIIndent;
		std::string& GetUIIndent();
	#endif

	class Element;
	class Menu;
	class AnimationHandler;

	class Element : public IElement
	{
		public:
			friend class AnimationHandler;

			ELEMENT_TYPE				elementType = ELEMENT_TYPE::kNONE;
			bool						isActive = false;
			Element*					parent = nullptr;
			std::vector<ElementPTR>	children{};
			Menu*						menu = nullptr;

			Element(std::string& a_name) : instanceName(a_name) { gfx.SetNull(); }

			void						Show() override;
			void						Hide() override;
			void						ToggleShowHide() override;
			void						Init();
			const char*					GetInstanceName() { return instanceName.c_str(); }
			std::vector<ElementPTR>&	GetChildren() override { return children; }
			void						AddChild(ElementPTR a_uiElement);
			void						BlockChildInteractions() override;
			void						AllowChildInteractions() override { allowChildInteractions = true; }
			
			virtual void	SetGFx(RE::GFxValue& a_GFxValue);
			RE::GFxValue	GetGFx() override;
			RE::GFxValue	GetParentGFx();
			bool			HasMenu();
			void			SetMask(IElement& a_maskElement) override;
			Element*		GetMask();
			void			MoveToFront() override;
			bool			IsInteractable() const;
			bool			IsVisible() const override;
			void			SetIgnoreMenuLock(bool a_ignore) override { ignoreMenuLock = a_ignore; }

			IElement* AttachUIElement(std::string a_linkageName, ELEMENT_TYPE a_type = ELEMENT_TYPE::kNONE) override { return AttachUIElement(a_linkageName, a_linkageName, a_type); }
			IElement* AttachUIElement(std::string a_linkageName, std::string a_instanceName, ELEMENT_TYPE a_type = ELEMENT_TYPE::kNONE) override;
			IElement* CreateGroup(std::string a_instanceName) override;
			IElement* CreateTextField(std::string a_instanceName) override;
			IElement* CreateEmptyUIElement(std::string a_instanceName, ELEMENT_TYPE a_type = ELEMENT_TYPE::kNONE) override;
			IElement* CreateSquareBBox() override;
			IElement* CreateSquareElement(std::string a_instanceName, uint32_t a_color, ELEMENT_TYPE a_type = ELEMENT_TYPE::kNONE) override;


			////// Interaction /////////////////////////////////////////////////
			void SetOnMouseStateChange(UIOnMouseStateFunction a_callback) override { onMouseStateChangeCallback = a_callback; }
			void SetOnChecked(std::function<void(Element*, bool)> a_callback) override;
			void SetOnChecked(std::function<void(bool)> a_callback) override { SetOnChecked([=](Element*, bool a_isChecked){ a_callback(a_isChecked); }); }
			void SetWhileHoverCallback(std::function<void(float)> a_callback) override { whileHoverCallback = a_callback; }

			void SetHoverAnimation(AnimationFunction a_fn) override { hoverAnimation = a_fn; }
			void SetPressedAnimation(AnimationFunction a_fn) override { pressedAnimation = a_fn; }
			void SetResetAnimation(AnimationFunction a_fn) override { resetAnimation = a_fn; }
			void SetShowAnimation(AnimationFunction a_fn) override { showAnimation = a_fn; }
			void SetHideAnimation(AnimationFunction a_fn) override { hideAnimation = a_fn; }
			void SetAnimationResetDuration(float a_duration) override { animationResetDuration = a_duration; } // Only relevant if resetanimation is not defined
			bool HasActiveAnimation() { return animationHandler.HasActiveAnimation(); }
			void PlayResetAnimation();

			void OnHover(float a_duration);
			void OnClick(float a_duration, bool a_isButtonDown);
			void ToggleChecked();
			//void SimulateClickAndRelease() { mouseState.set(MOUSE_STATE::kHOVER); OnClick(0.0f, false); }
			void PlayShowAnimation();
			void PlayHideAnimation();
			void ResetHover();
			void ResetClick();
			void OnMouseStateChanged(MOUSE_STATE a_state, bool a_sat);
			void SendCheckedEventWhenMenuOpens(bool a_yes) override { sendCheckedEventWhenMenuOpens = a_yes; }
			bool IsHovering() const override { return mouseState.hasFlag(MOUSE_STATE::kHOVER); }
			bool IsPressed() const override { return mouseState.hasFlag(MOUSE_STATE::kPRESSED); }
			bool IsChecked() const override { return mouseState.hasFlag(MOUSE_STATE::kCHECKED); }

			////// Editing /////////////////////////////////////////////////

			// Translation
			void			SetX(Size a_x) override;
			void			SetY(Size a_y) override;
			void			SetX(float a_x) override { SetX(Size(a_x)); }
			void			SetY(float a_y) override { SetY(Size(a_y)); }
			void			SetPos(Size a_x, Size a_y) override { SetX(a_x); SetY(a_y); }
			void			SetPos(float a_x, Size a_y) override { SetX(a_x); SetY(a_y); }
			void			SetPos(Size a_x, float a_y) override { SetX(a_x); SetY(a_y); }
			void			SetPos(float a_x, float a_y) override { SetX(a_x); SetY(a_y); }
			void			Move(Size a_x, Size a_y) override;
			void			Move(Size a_x, float a_y) override { Move(a_x, Size(a_y)); }
			void			Move(float a_x, Size a_y) override { Move(Size(a_x), a_y); }
			void			Move(float a_x, float a_y) override { Move(Size(a_x), Size(a_y)); }
			virtual void	MoveTo00() { SetPos(0.0f, 0.0f); }
			//virtual	void	SetWidth(Size a_width);
			//virtual	void	SetHeight(Size a_height);


			// Scaling
			void			SetXScale(Size a_x) override;
			void			SetYScale(Size a_y) override;
			void			SetXScale(float a_x) override { SetXScale(Size(a_x)); }
			void			SetYScale(float a_y) override { SetYScale(Size(a_y)); }
			void			SetScale(Size a_x, Size a_y) override { SetXScale(a_x); if (!lockAspectRatio) SetYScale(a_y) /* if lockAspectRatio is true, SetYScale is called by SetXScale*/; }
			void			SetScale(float a_x, Size a_y) override { SetXScale(a_x); if (!lockAspectRatio) SetYScale(a_y); }
			void			SetScale(Size a_x, float a_y) override { SetXScale(a_x); if (!lockAspectRatio) SetYScale(a_y); }
			void			SetScale(float a_x, float a_y) override { SetXScale(a_x); if (!lockAspectRatio) SetYScale(a_y); }
			void			SetScale(Size a_scale) override { SetScale(a_scale, a_scale); }
			void			SetScale(float a_scale) override { SetScale(a_scale, a_scale); }
			void			LockAspect() override { lockAspectRatio = true; }
			void			UnlockAspect() override { lockAspectRatio = false; }
			void			SetWidth(Size a_width) override;
			void			SetHeight(Size a_height) override;
			void			SetWidth(float a_width) override { SetWidth(Size(a_width)); }
			void			SetHeight(float a_height) override { SetHeight(Size(a_height)); }

			// Alignment
			void			AlignInsideParent(Alignment a_align) override { AlignInParent(a_align, Alignment::kInside); }
			void			AlignInsideParent(Alignment a_align1, Alignment a_align2) override { AlignInParent(a_align1, a_align2, Alignment::kInside); }
			void			AlignOutsideParent(Alignment a_align) override { AlignInParent(a_align, Alignment::kOutside); }
			void			AlignOutsideParent(Alignment a_align1, Alignment a_align2) override { AlignInParent(a_align1, a_align2, Alignment::kOutside); }
			void			AlignInsideOther(IElement* a_element, Alignment a_align) override { AlignToOther(a_element->AsUIElement(), a_align, Alignment::kInside); }
			void			AlignInsideOther(IElement* a_element, Alignment a_align1, Alignment a_align2) override { AlignToOther(a_element->AsUIElement(), a_align1, a_align2, Alignment::kInside); }
			void			AlignOutsideOther(IElement* a_element, Alignment a_align) override { AlignToOther(a_element->AsUIElement(), a_align, Alignment::kOutside); }
			void			AlignOutsideOther(IElement* a_element, Alignment a_align1, Alignment a_align2) override { AlignToOther(a_element->AsUIElement(), a_align1, a_align2, Alignment::kOutside); }
			void			RemoveAlignment() override { alignmentFlags = Alignment::kNone; elementAlignedTo = nullptr; }
			void			SetPadding(Direction a_direction, Size a_amount) override;
			void			SetPadding(Direction a_direction, float a_amount) override { SetPadding(a_direction, Size(a_amount)); }

			// Misc
			void			SetAlpha(uint32_t a_alpha) override;
			void			SetLayoutAlpha(uint32_t a_alpha) override;


			// Getters
			BBox*				GetWorldBounds() const { return bounds.get(); } 
			BBox*				GetBBox() const { return bbox.get(); }
			BBox*				GetInteractableWorldBounds() const { return bbox ? GetBBox() : GetWorldBounds(); }
			BBox::AABB			GetInteractableWorldAABB() const;
			float				GetX() const override { return x; }
			float				GetY() const override { return y; }
			float				GetWidth() const override { return baseWidth * xScale; }
			float				GetHeight() const override { return baseHeight * yScale; }
			virtual float		GetXScale() const override { return xScale; }
			virtual float		GetYScale() const override { return yScale; }
			virtual float		GetLocalXMin() const { return x; }
			virtual float		GetLocalYMin() const { return y; }
			virtual float		GetLocalXMax() const { return x+GetWidth(); }
			virtual float		GetLocalYMax() const { return y+GetHeight(); }
			float				GetGlobalXMin() const { return LocalToGlobalX(GetLocalXMin()); }
			float				GetGlobalXMax() const { return LocalToGlobalX(GetLocalXMax()); }
			float				GetGlobalYMin() const { return LocalToGlobalY(GetLocalYMin()); }
			float				GetGlobalYMax() const { return LocalToGlobalY(GetLocalYMax()); }
			UIFlag<MOUSE_STATE>	GetMouseState() const { return mouseState; }
			uint32_t			GetAlpha() const { return alpha; }


			virtual Group*		AsGroup() { return nullptr; }
			virtual Button*		AsButton() { return nullptr; }
			virtual Textfield*	AsTextfield() { return nullptr; }

			virtual bool	IsGroup() { return false; }
			virtual bool	IsButton() { return false; }
			virtual bool	IsTextfield() { return false; }

			bool			IsLayout() { return instanceName == LAYOUT_NAME || instanceName == TOGGLE_ON_NAME || instanceName == TOGGLE_OFF_NAME; }
			bool			IsBBox() { return instanceName == BBOX_NAME; }

			// Button methods
			virtual void SetToggleValuePtr(bool* a_toggleValue) override { return; };
			virtual void SetCyclicValuePtr(uint32_t* a_cyclicValue) override { return; };
			virtual void SetCorrectToggleValue() override { return; }
			virtual void CycleValue() override { return; }
			virtual void PrintOrderOfCyclicImages() override { return; }

			// Group Methods
			virtual void AlignChildrenHorizontally(Size a_spacing, Alignment a_align = Alignment::kTop) override { return; }
			virtual void AlignChildrenVertically(Size a_spacing, Alignment a_align = Alignment::kLeft) override { return; }
			virtual void SetVerticalScrollable(bool a_enabled) override { return; }
			virtual void SetHorizontalScrollable(bool a_enabled) override { return; }
			virtual void SetScrollableArea(IElement* a_element) override { return; };
			virtual void SetScrollableTopRatio(float a_ratio) override { return; }
			virtual void SetScrollableBottomRatio(float a_ratio) override { return; }
			virtual void SetOnScrollCallback(std::function<void(float)> a_callback) override { return; }

			// Text field methods
			virtual void SetText(const char* a_text) override { return; }
			virtual void SetText(std::string a_text) override { return; }
			virtual void SetFont(const char* a_font) override { return; }
			virtual void SetFontSize(uint32_t a_size) override { return; }
			virtual void SetFontColor(uint32_t a_defaultColor) override { return; }
			virtual void SetBold(bool a_enabled) override { return; }
			virtual void SetItalic(bool a_enabled) override { return; }
			virtual void SetUnderline(bool a_enabled) override { return; }
			virtual void SetTextAlign(const char* a_align) override { return; }
			virtual void SetIndent(float a_indent) override { return; }
			virtual void SetBlockIndent(float a_indent) override { return; }
			virtual void SetBullet(bool a_enabled) override { return; }
			virtual void SetKerning(bool a_enabled) override { return; }
			virtual void SetLeftMargin(float a_margin) override { return; }
			virtual void SetRightMargin(float a_margin) override { return; }
			virtual void SetLetterSpacing(float a_spacing) override { return; }
			virtual void SetLineSpacing(float a_spacing) override { return; }
			virtual void SetAutoSize(bool a_enabled) override { return; }
			virtual void SetWordWrap(bool a_enabled) override { return; }
			virtual void SetTextfieldWidth(Size a_width) override { return; }
			virtual void SetTextfieldHeight(Size a_height) override { return; }
			virtual void SetTextfieldWidth(float a_width) override { return; }
			virtual void SetTextfieldHeight(float a_height) override { return; }


			Element* AsUIElement() override { return static_cast<Element*>(this); }

			void SetVisible() { SetVisibleStatus(true); }
			void SetInvisible() { SetVisibleStatus(false); }

			void TraverseParents(std::function<void(Element*)> a_callback);
			void TraverseParents(const std::function<void(Element*)> a_callback) const;
			void TraverseParentsReverse(std::function<void(Element*)> a_callback);
			void TraverseParentsReverse(const std::function<void(Element*)> a_callback) const;
			void TraverseChildren(std::function<void(Element*)> a_callback);
			void TraverseChildren(const std::function<void(Element*)> a_callback) const;

		protected:
			friend class Menu;

			float		x = 0.0f;
			float		y = 0.0f;
			float		xScale = 1.0f;
			float		yScale = 1.0f;
			float		baseWidth = 1.0f;
			float		baseHeight = 1.0f;
			uint32_t	alpha = 100;
			bool		lockAspectRatio = true;
			bool		visible = true;
			bool		shouldBeVisible = true;
			bool		isShowAnimationDone = true;

			constexpr static const char* BBOX_NAME = "BBox";

			BBoxPTR				bbox = nullptr; // bbox is in global coordinates
			bool				interactable = false;
			bool				ignoreMenuLock = false;
			bool				allowChildInteractions = true;
			Element*			maskElement = nullptr;
			UIFlag<MOUSE_STATE>	mouseState{ MOUSE_STATE::kNONE };
			AnimationHandler	animationHandler{ this };
			bool				sendCheckedEventWhenMenuOpens = false;

			std::string					instanceName;
			std::function<void(float)>	whileHoverCallback = nullptr;
			UIOnMouseStateFunction		onMouseStateChangeCallback = nullptr;
			AnimationFunction			hoverAnimation = nullptr;
			AnimationFunction			pressedAnimation = nullptr;
			AnimationFunction			resetAnimation = nullptr;
			AnimationFunction			showAnimation = nullptr;
			AnimationFunction			hideAnimation = nullptr;
			float						animationResetDuration = 0.05f;	

			void	HideWithoutTurningInvisible();

		//// Local/Global transformations ////////////////////////////////////////////////////////////////////////////////////
			float	LocalToGlobalX(float a_localX) const;
			float	LocalToGlobalY(float a_localY) const;
			float	GlobalToLocalX(float a_globalX) const;
			float	GlobalToLocalY(float a_globalY) const;
			float	LocalToGlobalXDistance(float a_localX) const;
			float	LocalToGlobalYDistance(float a_localY) const;
			float	GlobalToLocalXDistance(float a_globalX) const;
			float	GlobalToLocalYDistance(float a_globalY) const;
			float	LocalToGlobalXScale(float a_localX) const;
			float	LocalToGlobalYScale(float a_localY) const;
			float	GlobalToLocalXScale(float a_globalY) const;
			float	GlobalToLocalYScale(float a_globalY) const;

		public: // Temp
			void	RetrievePositionalInfo();
			void	UpdateBounds();
			void	UpdateAlignment();
			float	GetParentWidth();
			float	GetParentHeight();
			void	OnTranslate(std::string arguments);
			void	OnSetScale();
			float	SizeToPosition(Size a_size, bool a_isX);
			float	SizeToPositionX(Size a_size);
			float	SizeToPositionY(Size a_size);
			float	SizeToDistanceX(Size a_size);
			float	SizeToDistanceY(Size a_size);
			float	SizeToScale(Size a_size, bool a_isXScale);

			void	SetVisibleStatus(bool a_enabled);
			void	SetVisibleStatusImpl(bool a_enabled);

			
		private:
			bool hasGFx = false;
			RE::GFxValue gfx;

			void	UpdateAnimation(float a_deltaTime);

			void	SetXImpl(float a_x, bool a_updateInternally = true);
			void	SetYImpl(float a_y, bool a_updateInternally = true);
			void	SetXScaleImpl(float a_x, bool a_updateInternally = true);
			void	SetYScaleImpl(float a_y, bool a_updateInternally = true);
			void	SetAlphaImpl(uint32_t a_alpha, bool a_updateInternally = true);
			

			// Global bounds of MovieClip excluding its children
			BBoxPTR bounds;

		//// Alignment ////////////////////////////////////////////////////////////////////////////////////
			struct Padding
			{
				float left = 0.0f;
				float right = 0.0f;
				float top = 0.0f;
				float bottom = 0.0f;
			};

			Alignment	alignmentFlags = Alignment::kNone;
			Padding		padding{};
			Element*  elementAlignedTo = nullptr; // Uses parent if null

			bool HasAlignment(Alignment a_align) { return static_cast<uint16_t>(alignmentFlags) & static_cast<uint16_t>(a_align); }
			void RemoveXAlignment();
			void RemoveYAlignment();
			void ResetAlignmentFlag(Alignment a_align) { alignmentFlags = static_cast<Alignment>(static_cast<uint16_t>(alignmentFlags) & ~static_cast<uint16_t>(a_align)); }
			void SetAlignment(Alignment a_align);
			void AlignInParent(Alignment a_align);
			void AlignInParent(Alignment a_align1, Alignment a_align2);
			void AlignInParent(Alignment a_align1, Alignment a_align2, Alignment a_align3);
			void AlignToOther(Element* a_other, Alignment a_align);
			void AlignToOther(Element* a_other, Alignment a_align1, Alignment a_align2);
			void AlignToOther(Element* a_other, Alignment a_align1, Alignment a_align2, Alignment a_align3);
			void SetElementAlignedTo(Element* a_element);

			
	};
}