#include "GroupElement.h"
#include "Menu.h"


ScaleformUI::Group::Group(std::string& a_name) : Element(a_name)
{
	elementType = ELEMENT_TYPE::kGROUP;
}

float ScaleformUI::Group::GetWidth() const
{
	return GetLocalXMax() - GetLocalXMin();
}

float ScaleformUI::Group::GetHeight() const
{
	return GetLocalYMax() - GetLocalYMin();
}

float ScaleformUI::Group::GetLocalXMin() const
{
	if (children.empty()) return x;
	float smallestChildXMin = 123456790.0f;

	for (const auto& childBase : children)
	{
		auto child = childBase->AsUIElement();
		if (child->GetLocalXMin() < smallestChildXMin) smallestChildXMin = child->GetLocalXMin();
	}

	return x + smallestChildXMin;

	//if (children.empty()) return 0.0f;
	//float localXMin = 123456789.0f;

	//TraverseChildren([&](Element* a_child)
	//{
	//	
	//	// child positions ignore the parent's scale, so we must multiply them to 
	//	// get child positions in parent's coordinates
	//	float tempXMin = a_child->GetLocalXMin() * a_child->parent->GetXScale();
	//	if (tempXMin < localXMin) localXMin = tempXMin;
	//});
	//return localXMin + x;
}

float ScaleformUI::Group::GetLocalYMin() const
{
	if (children.empty()) return y;
	float smallestChildYMin = 123456790.0f;

	for (const auto& childBase : children)
	{
		auto child = childBase->AsUIElement();
		if (child->GetLocalYMin() < smallestChildYMin) smallestChildYMin = child->GetLocalYMin();
	}

	return y + smallestChildYMin;

	//if (children.empty()) return 0.0f;
	//float localYMin = 123456789.0f;

	//TraverseChildren([&](Element* a_child)
	//{
	//	float tempYMin = a_child->GetLocalYMin() * a_child->parent->GetYScale();

	//	if (tempYMin < localYMin) 
	//	{
	//		localYMin = tempYMin;
	//	}
	//});

	//return localYMin + y;
}

float ScaleformUI::Group::GetLocalXMax() const
{
	if (children.empty()) return x;
	float biggestChildXMax = -123456790.0f;

	for (const auto& childBase : children)
	{
		auto child = childBase->AsUIElement();
		if (child->GetLocalXMax() > biggestChildXMax) biggestChildXMax = child->GetLocalXMax();
	}

	return x + biggestChildXMax;

	//if (children.empty()) return 0.0f;
	//float localXMax = -123456789.0f;;

	//TraverseChildren([&](Element* a_child)
	//{
	//	// child positions ignore the parent's scale, so we must multiply them to 
	//	// get child positions in parent's coordinates
	//	float tempXMax = a_child->GetLocalXMax() * a_child->parent->GetXScale();

	//	if (tempXMax > localXMax) localXMax = tempXMax;

	//});

	//return localXMax + x;
}

float ScaleformUI::Group::GetLocalYMax() const
{
	if (children.empty()) return y;
	float biggestChildYMax = -123456790.0f;

	for (const auto& childBase : children)
	{
		auto child = childBase->AsUIElement();
		if (child->GetLocalYMax() > biggestChildYMax) biggestChildYMax = child->GetLocalYMax();
	}

	return y + biggestChildYMax;
	//if (children.empty()) return 0.0f;
	//float localYMax = -123456789.0f;

	//TraverseChildren([&](Element* a_child)
	//{
	//	// child positions ignore the parent's scale, so we must multiply them to 
	//	// get child positions in parent's coordinates
	//	float tempYMax = a_child->GetLocalYMax() * a_child->parent->GetYScale();

	//	if (tempYMax > localYMax) localYMax = tempYMax;

	//});

	//return localYMax + y;
}

void ScaleformUI::Group::MoveTo00()
{
	SetPos(0.0f, 0.0f);
	SetPos(- GetLocalXMin(), - GetLocalYMin());
}

float ScaleformUI::Group::GetBiggestChildWidth()
{
	float maxWidth = 0.0f;
	for (const auto& child : children)
	{
		if (child->GetWidth() > maxWidth) maxWidth = child->GetWidth();
	}
	return maxWidth;
}

float ScaleformUI::Group::GetBiggestChildHeight()
{
	float maxHeight = 0.0f;
	TraverseChildren([&](Element* a_child)
	{
		if (a_child->GetHeight() > maxHeight) maxHeight = a_child->GetHeight();
	});
	return maxHeight;
}

#pragma warning(disable : 26813)
void ScaleformUI::Group::AlignChildrenHorizontally(Size a_spacing, Alignment a_align)
{
	uint16_t childIndex = 0;
	uint16_t numChildren = children.size();
	if (numChildren == 0) return;

	float spacing = SizeToDistanceX(a_spacing);

	float biggestHeight = GetBiggestChildHeight();

	float previousXMax = 0.0f;

	for (auto& childBase : children)
	{
		auto* child = childBase->AsUIElement();
		if (child->IsGroup()) child->MoveTo00();

		float newX = previousXMax;
		float newY = 0;
		if (childIndex > 0) newX += spacing;
		if (a_align == Alignment::kCenterV) newY = biggestHeight / 2 - child->GetHeight() / 2;
		else if (a_align == Alignment::kBottom) newY = biggestHeight - child->GetHeight();

		if (child->IsGroup()) child->Move(newX, newY);
		else child->SetPos(newX, newY);
		childIndex++;
		previousXMax = child->GetLocalXMax();
	}

	UpdateAlignment();
}

void ScaleformUI::Group::AlignChildrenVertically(Size a_spacing, Alignment a_align)
{
	uint16_t childIndex = 0;
	uint16_t numChildren = children.size();
	if (numChildren == 0) return;

	float spacing = SizeToDistanceY(a_spacing);

	float biggestWidth = GetBiggestChildWidth();

	float previousYMax = 0.0f;

	for (auto& childBase : children)
	{
		auto* child = childBase->AsUIElement();
		float newX = child->GetX();
		float newY = previousYMax;

		//if (child->IsGroup()) 
		//{
		//	child->MoveTo00();
		//}

		if (childIndex > 0) newY += spacing;
		if (a_align == Alignment::kLeft) newX = 0.0f; 
		if (a_align == Alignment::kCenterH) newX = (biggestWidth - child->GetWidth())/2; 
		else if (a_align == Alignment::kRight) newX = biggestWidth - child->GetWidth(); 

		if (child->IsGroup()) 
		{
			newX += child->GetX() - child->GetLocalXMin();
			newY += child->GetY() - child->GetLocalYMin();
		}

		//if (child->IsGroup()) child->Move(newX, newY);
		//else 
		child->SetPos(newX, newY);

		childIndex++;
		previousYMax = child->GetLocalYMax();
	}

	UpdateAlignment();
}

bool ScaleformUI::Group::IsVerticallyScrollable()
{
	if (!scrollableArea)
	{
		#ifdef LOG_UI
			UIIndent++;
			if (isVerticallyScrollable) logger::debug("{}UI: group '{}' is vertically scrollable, but no scrollable area is defined!", GetUIIndent(), GetInstanceName());
			UIIndent--;
		#endif

		return false;
	}

	return isVerticallyScrollable;
}

bool ScaleformUI::Group::IsHorizontallyScrollable()
{
	if (!scrollableArea)
	{
		#ifdef LOG_UI
			UIIndent++;
			if (isHorizontallyScrollable) logger::debug("{}UI: group '{}' is horizontally scrollable, but no scrollable area is defined!", GetUIIndent(), GetInstanceName());
			UIIndent--;
		#endif

		return false;
	}

	return isHorizontallyScrollable;
}

void ScaleformUI::Group::ScrollVertically(Size a_distance)
{
	if (!scrollableArea)
	{
		#ifdef LOG_UI
			UIIndent++;
			logger::debug("{}UI ERROR: Falied to scroll group '{}': No scrollable area defined!", GetUIIndent(), GetInstanceName());
			UIIndent--;
		#endif

		return;
	}


	float distance = SizeToDistanceY(a_distance);

	float currentYMin = GetLocalYMin();
	float currentYMax = GetLocalYMax();
	float areaYMin = GlobalToLocalY(scrollableArea->AsUIElement()->GetGlobalYMin());
	float areaYMax = GlobalToLocalY(scrollableArea->AsUIElement()->GetGlobalYMax());

	if (currentYMax - currentYMin < areaYMax - areaYMin) return;


	float bottomStopValue = areaYMax - (areaYMax - areaYMin) * scrollBottomStopRatio;
	float topStopValue = areaYMin + (areaYMax - areaYMin) * scrollTopStopRatio;

	if (currentYMin + distance > topStopValue) distance = topStopValue - currentYMin;
	else if (currentYMax + distance < bottomStopValue) distance = bottomStopValue - currentYMax;

	Move(0.0f, distance);

	// min_y: currentYMin + distance = topStopValue; 
	//			let y' = y + distance => y' = topStopValue 
	//								  => min_y = topStopValue
	// max y: currentYMax + distance = bottomStopValue;
	//			let y' = y+distance => y' + height = bottomStopValue 
	//								=> max_y = bottomStopValue - height


	float tempHeight = currentYMax - currentYMin;
	float distanceToTop = topStopValue - (currentYMin+distance);
	float scrollAreaSize = topStopValue - (bottomStopValue - tempHeight);
	float scrollRatio = distanceToTop/scrollAreaSize;
	OnScroll(scrollRatio);
}

void ScaleformUI::Group::ScrollHorizontally(Size a_distance)
{

}

void ScaleformUI::Group::ResetScroll()
{
	float currentYMin = GetLocalYMin();
	float areaYMin = GlobalToLocalY(scrollableArea->AsUIElement()->GetGlobalYMin());
	float areaYMax = GlobalToLocalY(scrollableArea->AsUIElement()->GetGlobalYMax());
	float topStopValue = areaYMin + (areaYMax - areaYMin) * scrollTopStopRatio;

	float distance = topStopValue - currentYMin;
	Move(0.0f, distance);
	OnScroll(0.0f);
}


void ScaleformUI::Group::OnScroll(float a_scrollRatio)
{
	if (onScrollCallback) onScrollCallback(a_scrollRatio);
}

