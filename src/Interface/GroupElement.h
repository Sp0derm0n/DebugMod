#pragma once

#include "Element.h"

namespace ScaleformUI
{
	class Group : public Element
	{
		public:
			Group(std::string& a_name);

			Group*		AsGroup() override { return static_cast<Group*>(this); }
			bool		IsGroup() override { return true; }

			float		GetWidth() const override;
			float		GetHeight() const override;
			float		GetLocalXMin() const override;
			float		GetLocalYMin() const override;
			float		GetLocalXMax() const override;
			float		GetLocalYMax() const override;

			void		MoveTo00() override;
			void		AlignChildrenHorizontally(Size a_spacing, Alignment a_align = Alignment::kTop) override;
			void		AlignChildrenVertically(Size a_spacing, Alignment a_align = Alignment::kLeft) override;

			void		SetVerticalScrollable(bool a_enabled) override { isVerticallyScrollable = a_enabled; }
			void		SetHorizontalScrollable(bool a_enabled) override { isHorizontallyScrollable = a_enabled; }
			bool		IsVerticallyScrollable();
			bool		IsHorizontallyScrollable();
			void		SetScrollableArea(IElement* a_element) override { scrollableArea = a_element; };
			BBox*		GetScrollableArea() { return scrollableArea ? scrollableArea->AsUIElement()->GetWorldBounds() : nullptr; }
			void		SetScrollableTopRatio(float a_ratio) { scrollTopStopRatio = a_ratio; }
			void		SetScrollableBottomRatio(float a_ratio) { scrollBottomStopRatio = a_ratio; }
			void		ScrollVertically(Size a_distance);
			void		ScrollHorizontally(Size a_distance);
			void		OnScroll(float a_scrollRatio);
			void		SetOnScrollCallback(std::function<void(float)> a_callback) override { onScrollCallback = a_callback; }
			void		ResetScroll();

		private:
			bool			isVerticallyScrollable = false;
			bool			isHorizontallyScrollable = false;
			IElement*	scrollableArea = nullptr;

			std::function<void(float)> onScrollCallback = nullptr;

			// top: percentage of visible area above the element when it can't be scrolled down any lower
			// bottom: percante of visible area below the element when it can't be scrolled up any higher
			float			scrollTopStopRatio = 0.0f;
			float			scrollBottomStopRatio = 0.2f;

			float	GetBiggestChildWidth();
			float	GetBiggestChildHeight();
	};
}