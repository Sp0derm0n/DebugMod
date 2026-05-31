#include "Element.h"
#include "Menu.h"

#ifdef LOG_UI
	int ScaleformUI::UIIndent = 0;
	std::string UIIndentStr = ""s;
	std::string& ScaleformUI::GetUIIndent() 
	{ 
		UIIndentStr = std::string(ScaleformUI::UIIndent, ' '); 
		return UIIndentStr;
	}
#endif

bool ScaleformUI::Element::HasMenu()
{
	if (!menu)
	{
		logger::debug("UI ERROR: element '{}' has no menu!", GetInstanceName());
		return false;
	}
	return true;
}

void ScaleformUI::Element::RetrievePositionalInfo()
{
	if (IsLayout() || IsBBox()) parent->RetrievePositionalInfo();

	auto thisGFx = GetGFx();

	bool hasLayout = thisGFx.HasMember("layout");

	RE::GFxValue displayHolder = thisGFx;

	if (hasLayout)
	{
		thisGFx.GetMember("layout", &displayHolder);
	}

	RE::GFxValue::DisplayInfo displayInfo;
	thisGFx.GetDisplayInfo(&displayInfo);

	x = static_cast<float>(displayInfo.GetX());
	y = static_cast<float>(displayInfo.GetY());
	xScale = static_cast<float>(displayInfo.GetXScale()) / 100;
	yScale = static_cast<float>(displayInfo.GetYScale()) / 100;

	RE::GFxValue _width, _height;
	displayHolder.GetMember("_width", &_width);
	displayHolder.GetMember("_height", &_height);

	baseWidth = static_cast<float>(_width.GetNumber());
	baseHeight = static_cast<float>(_height.GetNumber());

	//logger::debug("{} w,h: {}, {}", GetInstanceName(), width, height);
}

void ScaleformUI::Element::Show()
{
	SetVisible();
	PlayShowAnimation();
	if (showAnimation)
	{
		//isShowAnimationDone = false; // Blocks child interactions;
		BlockChildInteractions();
		animationHandler.SetPostAnimationQueueFinishCallback([](Element* a_this)
		{
			//a_this->isShowAnimationDone = true;
			a_this->AllowChildInteractions();
		}, 
		true);
	}
}

void ScaleformUI::Element::Hide()
{
	HideWithoutTurningInvisible();
	if (hideAnimation)
	{
		animationHandler.SetPostAnimationQueueFinishCallback([](Element* a_this)
		{
			a_this->SetInvisible();
		});
	}
	else
	{
		SetInvisible();
	}
}

void ScaleformUI::Element::HideWithoutTurningInvisible()
{
	PlayHideAnimation();
	visible = false; // Must be here such that if toggle is hit while the menu is hiding, it will open immediately
}

void ScaleformUI::Element::ToggleShowHide()
{
	if (visible) Hide();
	else Show();
}

void ScaleformUI::Element::PlayShowAnimation()
{
	if (showAnimation)
	{
		animationHandler.EmptyAnimationQueue();
		showAnimation(&animationHandler);
	}
}

void ScaleformUI::Element::PlayHideAnimation()
{
	if (hideAnimation)
	{
		animationHandler.EmptyAnimationQueue();
		hideAnimation(&animationHandler);
	}
}

void ScaleformUI::Element::Init()
{

	if (!HasMenu()) return;

	#ifdef LOG_UI
		auto thisGFx = GetGFx();
		UIIndent++;
		if (!thisGFx.HasMember(LAYOUT_NAME)) logger::debug("{}Element {} has no member named '{}'", GetUIIndent(), GetInstanceName(), LAYOUT_NAME);
	#endif

	if (!bounds) bounds = std::make_unique<SquareBBox>();

	

	if (IsLayout())
	{
		parent->Init();
	}
	else if (!IsGroup())
	{ 
		RetrievePositionalInfo();
		UpdateBounds();
	}
	

	#ifdef LOG_UI
		UIIndent--;
	#endif
}

ScaleformUI::BBox::AABB ScaleformUI::Element::GetInteractableWorldAABB() const
{
	return GetInteractableWorldBounds()->GetAABB();
}

void ScaleformUI::Element::TraverseParents(std::function<void(Element*)> a_callback)
{
	for (auto parentElement = parent; parentElement; parentElement = parentElement->parent)
	{
		a_callback(parentElement);
	}
}

void ScaleformUI::Element::TraverseParents(const std::function<void(Element*)> a_callback) const
{
	for (auto parentElement = parent; parentElement; parentElement = parentElement->parent)
	{
		a_callback(parentElement);
	}
}

void ScaleformUI::Element::TraverseParentsReverse(std::function<void(Element*)> a_callback)
{
	std::vector<Element*> parents;
	for (auto parentElement = parent; parentElement; parentElement = parentElement->parent)
	{
		parents.push_back(parentElement);
	}
	for (size_t i = parents.size(); i--;)
	{
		a_callback(parents[i]);
	}
}

void ScaleformUI::Element::TraverseParentsReverse(const std::function<void(Element*)> a_callback) const
{
	std::vector<Element*> parents;
	for (auto parentElement = parent; parentElement; parentElement = parentElement->parent)
	{
		parents.push_back(parentElement);
	}
	for (size_t i = parents.size(); i--;)
	{
		a_callback(parents[i]);
	}
}

void ScaleformUI::Element::TraverseChildren(std::function<void(Element*)> a_callback)
{
	if (children.empty()) return;

	for (auto& child : children)
	{
		if (child->AsUIElement()->IsLayout() || child->AsUIElement()->IsBBox()) continue;
		a_callback(child->AsUIElement());
		child->AsUIElement()->TraverseChildren(a_callback);
	}
}

void ScaleformUI::Element::TraverseChildren(const std::function<void(Element*)> a_callback) const
{
	if (children.empty()) return;

	for (const auto& child : children)
	{
		if (child->AsUIElement()->IsLayout() || child->AsUIElement()->IsBBox()) continue;
		a_callback(child->AsUIElement());
		child->AsUIElement()->TraverseChildren(a_callback);
	}
}

void ScaleformUI::Element::BlockChildInteractions() 
{ 
	allowChildInteractions = false; 
	ResetHover();
	ResetClick();
}

#pragma warning(disable : 26813)
void ScaleformUI::Element::OnMouseStateChanged(MOUSE_STATE a_state, bool a_sat)
{
	if (a_state == MOUSE_STATE::kCHECKED)
	{
		if (IsButton())
		{
			if (AsButton()->IsToggleButton()) AsButton()->SetCorrectToggleValue();
			else if (AsButton()->IsCyclicButton()) AsButton()->CycleValue();
		}
	}

	if (onMouseStateChangeCallback) onMouseStateChangeCallback(this, a_state, a_sat);


	bool playPressedAnimation = a_state == MOUSE_STATE::kPRESSED && a_sat;
	bool playHoverAnimation = (a_state == MOUSE_STATE::kHOVER && a_sat) || 
							  (a_state == MOUSE_STATE::kPRESSED && !a_sat &&
							   mouseState.hasFlag(MOUSE_STATE::kHOVER));
	if (playPressedAnimation)
	{
		if (pressedAnimation)
		{
			animationHandler.EmptyAnimationQueue();
			pressedAnimation(&animationHandler);
		}
		//if (a_sat) RE::PlaySound("UIFavorite");

	}
	else if (playHoverAnimation)
	{
		if (hoverAnimation)
		{
			animationHandler.EmptyAnimationQueue();
			hoverAnimation(&animationHandler);
		}
		//if (a_sat) RE::PlaySound("UIFavorite");

	}
	else if (!mouseState.hasFlag(MOUSE_STATE::kPRESSED) && !mouseState.hasFlag(MOUSE_STATE::kHOVER))
	{
		if (animationHandler.IsAnimated())
		{
			PlayResetAnimation();
		}
		//if (a_sat) RE::PlaySound("UIFavorite");

	}
}

void ScaleformUI::Element::SetOnChecked(std::function<void(Element*, bool)> a_callback)
{
	SetOnMouseStateChange([=](Element* a_element, MOUSE_STATE a_state, bool a_sat)
	{
		switch (a_state)
		{
			case MOUSE_STATE::kCHECKED:
			{
				a_callback(a_element, a_sat);
			}
		}
	});
}

void ScaleformUI::Element::PlayResetAnimation()
{
	animationHandler.EmptyAnimationQueue();
	if (resetAnimation) resetAnimation(&animationHandler);
	else animationHandler.PlayResetAnimation(animationResetDuration);
}

void ScaleformUI::Element::UpdateAnimation(float a_deltaTime)
{
	animationHandler.Update(a_deltaTime);
}

void ScaleformUI::Element::OnHover(float a_duration)
{
	if (!mouseState.hasFlag(MOUSE_STATE::kHOVER))
	{
		mouseState.set(MOUSE_STATE::kHOVER);
		OnMouseStateChanged(MOUSE_STATE::kHOVER, true);
	}

	if (whileHoverCallback) whileHoverCallback(a_duration);
}

void ScaleformUI::Element::OnClick(float a_duration, bool a_isButtonDown)
{
	if (!mouseState.hasFlag(MOUSE_STATE::kPRESSED) && a_duration == 0.0f)
	{
		mouseState.set(MOUSE_STATE::kPRESSED);
		OnMouseStateChanged(MOUSE_STATE::kPRESSED, true);
	}

	// handle click release
	if (mouseState.hasFlag(MOUSE_STATE::kPRESSED) && !a_isButtonDown && mouseState.hasFlag(MOUSE_STATE::kHOVER))
	{
		ToggleChecked();
	}
}

void ScaleformUI::Element::ToggleChecked()
{
	if (!mouseState.hasFlag(MOUSE_STATE::kCHECKED))
	{
		mouseState.set(MOUSE_STATE::kCHECKED);
		OnMouseStateChanged(MOUSE_STATE::kCHECKED, true);
	}
	else
	{
		mouseState.reset(MOUSE_STATE::kCHECKED);
		OnMouseStateChanged(MOUSE_STATE::kCHECKED, false);
	}
}

void ScaleformUI::Element::ResetHover()
{
	if (mouseState.hasFlag(MOUSE_STATE::kHOVER))
	{
		mouseState.reset(MOUSE_STATE::kHOVER);
		OnMouseStateChanged(MOUSE_STATE::kHOVER, false);

		if (whileHoverCallback) whileHoverCallback(-1.0f);

	}
}

void ScaleformUI::Element::ResetClick()
{
	if (mouseState.hasFlag(MOUSE_STATE::kPRESSED))
	{
		mouseState.reset(MOUSE_STATE::kPRESSED);
		OnMouseStateChanged(MOUSE_STATE::kPRESSED, false);
	}
}

bool ScaleformUI::Element::IsInteractable() const
{
	if (!interactable || !IsVisible()) return false;

	bool isChildInteractionsAllowed = true;
	bool shouldMenuLockBeIgnored = false;
	TraverseParents([&](Element* a_parent)
	{
		if (!a_parent->allowChildInteractions)
		{
			isChildInteractionsAllowed = false;
			return; // if child interacitons are not allowed, return, since element is not interactable
		}
		if (a_parent->ignoreMenuLock)
		{
			shouldMenuLockBeIgnored = true;
			// if parents ignore menu lock, don't return, because maybe parents further up don't allow child interactions
		}
	});

	if (!isChildInteractionsAllowed) return false;
	if (menu->IsMenuLocked())
	{
		if (shouldMenuLockBeIgnored) return true;
		else return false;
	}


	return true;
}

bool ScaleformUI::Element::IsVisible() const
{
	if (!visible || !isShowAnimationDone) 
	{
		return false;
	}
	if (parent) return parent->IsVisible();
	
	return true;
}

void ScaleformUI::Element::AddChild(ElementPTR a_uiElement)
{
	if (!a_uiElement)
	{
		logger::debug("UI ERROR: Failed to add child to '{}", instanceName);
		return;
	}
	a_uiElement->AsUIElement()->parent = this;
	children.emplace_back(std::move(a_uiElement));
}

ScaleformUI::IElement* ScaleformUI::Element::AttachUIElement(std::string a_linkageName, std::string a_instanceName, ELEMENT_TYPE a_type)
{
	return menu->AttachUIElement(a_linkageName, a_instanceName, a_type, this);
}

ScaleformUI::IElement* ScaleformUI::Element::CreateGroup(std::string a_instanceName)
{
	return menu->CreateGroup(a_instanceName, this);
}

ScaleformUI::IElement* ScaleformUI::Element::CreateTextField(std::string a_instanceName)
{
	return menu->CreateTextfield(a_instanceName, this);
}

ScaleformUI::IElement* ScaleformUI::Element::CreateEmptyUIElement(std::string a_instanceName, ELEMENT_TYPE a_type)
{
	return menu->CreateEmptyUIElement(a_instanceName, a_type, this);
}

ScaleformUI::IElement* ScaleformUI::Element::CreateSquareBBox()
{
	auto elementGFx = GetGFx();

	if (elementGFx.HasMember(BBOX_NAME))
	{
		for (auto& child : children)
		{
			if (std::strcmp(child->AsUIElement()->GetInstanceName(), BBOX_NAME) == 0)
			{
				return child.get();
			}
		}
		logger::debug("UI ERROR: Element '{}' has bbox member, but it couldn't be found!", GetInstanceName());
	}
	auto bboxElement = CreateEmptyUIElement(BBOX_NAME);

	auto bboxGFx = bboxElement->GetGFx();

	float xmin = 0;
	float xmax = GetWidth()/xScale;
	float ymin = 0;
	float ymax = GetHeight()/yScale;

	if (GetWidth() == 0.0f) xmax = 10.0f;
	if (GetHeight() == 0.0f) ymax = 10.0f;

	// thickness, color, alpha
	RE::GFxValue argsLineStyle[3]{ 1, 0xFF0000, 100 };
	bboxGFx.Invoke("lineStyle", nullptr, argsLineStyle, 3);

	RE::GFxValue argsCorner00[2]{ xmin, ymin };
	bboxGFx.Invoke("moveTo", nullptr, argsCorner00, 2);

	RE::GFxValue argsCorner10[2]{ xmax, ymin };
	bboxGFx.Invoke("lineTo", nullptr, argsCorner10, 2);

	RE::GFxValue argsCorner11[2]{ xmax, ymax };
	bboxGFx.Invoke("lineTo", nullptr, argsCorner11, 2);

	RE::GFxValue argsCorner01[2]{ xmin, ymax };
	bboxGFx.Invoke("lineTo", nullptr, argsCorner01, 2);

	bboxGFx.Invoke("lineTo", nullptr, argsCorner00, 2);

	bboxGFx.Invoke("endFill", nullptr, nullptr, 0);

	bboxElement->SetInvisible();

	bbox = std::make_unique<SquareBBox>();

	bboxElement->AsUIElement()->Init();
	UpdateBounds();



	return bboxElement;
}

ScaleformUI::IElement* ScaleformUI::Element::CreateSquareElement(std::string a_instanceName, uint32_t a_color, ELEMENT_TYPE a_type)
{
	return menu->CreateSquareElement(a_instanceName, a_color, a_type, this);
	//auto* square = CreateEmptyUIElement(a_instanceName, a_type);
	//auto squareGFx = square->GetGFx();

	//float xmin = 0.0f;
	//float ymin = 0.0f;
	//float xmax = 100.0f;
	//float ymax = 100.0f;

	//// thickness, color, alpha
	//RE::GFxValue argsLineStyle[3]{ 0, 0xFFFFFF, 100 };
	//squareGFx.Invoke("lineStyle", nullptr, argsLineStyle, 3);

	//RE::GFxValue argsFill[2]{ 0xFFFFFF, 100 };
	//squareGFx.Invoke("beginFill", nullptr, argsFill, 2);

	//RE::GFxValue argsCorner00[2]{ xmin, ymin };
	//squareGFx.Invoke("moveTo", nullptr, argsCorner00, 2);

	//RE::GFxValue argsCorner10[2]{ xmax, ymin };
	//squareGFx.Invoke("lineTo", nullptr, argsCorner10, 2);

	//RE::GFxValue argsCorner11[2]{ xmax, ymax };
	//squareGFx.Invoke("lineTo", nullptr, argsCorner11, 2);

	//RE::GFxValue argsCorner01[2]{ xmin, ymax };
	//squareGFx.Invoke("lineTo", nullptr, argsCorner01, 2);

	//squareGFx.Invoke("lineTo", nullptr, argsCorner00, 2);

	//squareGFx.Invoke("endFill", nullptr, nullptr, 0);

	//square->AsUIElement()->Init();
	//UpdateBounds();

	//return square;
}

void ScaleformUI::Element::SetGFx(RE::GFxValue& a_GFxValue) 
{ 
	hasGFx = true; 
	gfx = a_GFxValue; 
}

RE::GFxValue ScaleformUI::Element::GetGFx()
{
	if (gfx.IsNull()) logger::debug("UI ERROR: Element '{}' has no GFx", GetInstanceName());
	return gfx;
}

RE::GFxValue ScaleformUI::Element::GetParentGFx()
{
	#ifdef LOG_UI
		UIIndent++;
	#endif

	if (parent) 
	{
		return parent->GetGFx();
	}
	else if (HasMenu())
	{
		RE::GFxValue rootGFx;
		menu->GetRoot(rootGFx);

		return rootGFx;
	}
	else
	{
		#ifdef LOG_UI
			logger::debug("{}UI ERROR: element '{}' has no parent and belongs to no menu; ParentGFx unavailable", GetUIIndent(), GetInstanceName());
		#endif
	}

	#ifdef LOG_UI
		UIIndent--;
	#endif

	RE::GFxValue nullGFx; nullGFx.SetNull();
	return nullGFx;
}

void ScaleformUI::Element::SetMask(IElement& a_maskElement)
{
	#ifdef LOG_UI
		UIIndent++;
		logger::debug("{}Setting element '{}' as mask for element '{}'", GetUIIndent(), a_maskElement.AsUIElement()->GetInstanceName(), GetInstanceName());
	#endif

	//Attachment newAttachment;
	//newAttachment.attachedElement = a_maskElement;
	//newAttachment.SetFlag(Attachment::flag::kInteractable);
	//newAttachment.SetFlag(Attachment::flag::kMask);

	maskElement = a_maskElement.AsUIElement();

	auto maskGFx = maskElement->GetGFx();

	auto thisGFx = GetGFx();

	RE::GFxValue layout;
	bool hasLayout = thisGFx.GetMember("layout", &layout);

	//#ifdef LOG_UI
	//	if (!success) logger::debug("UI ERROR: Failed to get layout from mask element '{}'", a_maskElement->GetInstanceName());
	//#endif
	bool success = false;
	if (hasLayout)
	{
		success = layout.Invoke("setMask", nullptr, &maskGFx, 1);
	}
	else
	{
		success = thisGFx.Invoke("setMask", nullptr, &maskGFx, 1);
	}

	#ifdef LOG_UI
		if (!success) logger::debug("{}UI ERROR: Failed to set '{}' as mask for element '{}'. Possibly because '{}' has no layout", GetUIIndent(), a_maskElement.AsUIElement()->GetInstanceName(), GetInstanceName(), GetInstanceName());
		UIIndent--;
	#endif
}

ScaleformUI::Element* ScaleformUI::Element::GetMask()
{
	if (maskElement) return maskElement;

	TraverseParents([&](Element* a_element)
	{
		// break, because we want to get the closest most mask
		if (a_element->maskElement) 
		{
			maskElement = a_element->maskElement;
			return;
		}

	});
	return maskElement;
}

void ScaleformUI::Element::MoveToFront()
{
	auto elementGFx = GetGFx();

	RE::GFxValue root;
	menu->GetRoot(root);

	RE::GFxValue nextHighestDepth;
	root.Invoke("getNextHighestDepth", &nextHighestDepth);

	elementGFx.Invoke("swapDepths", nullptr, &nextHighestDepth, 1);
}

void ScaleformUI::Element::SetVisibleStatus(bool a_enabled)
{
	SetVisibleStatusImpl(a_enabled);
	shouldBeVisible = a_enabled;
}

void ScaleformUI::Element::SetVisibleStatusImpl(bool a_enabled)
{
	#ifdef LOG_UI
		UIIndent++;
	#endif

	visible = a_enabled;


	auto elementGFx = GetGFx();

	RE::GFxValue::DisplayInfo displayInfo;
	elementGFx.GetDisplayInfo(&displayInfo);

	displayInfo.SetVisible(a_enabled);
	bool success = elementGFx.SetDisplayInfo(displayInfo);

	visible = a_enabled;

	#ifdef LOG_UI
		if (!success) logger::debug("{}UI ERROR: Failed to set visible status of element '{}'", GetUIIndent(), GetInstanceName());
		UIIndent--;
	#endif
}

float ScaleformUI::Element::GetParentWidth()
{
	if (!parent) return menu->ScreenW();

	float width_ = parent->GetWidth() / parent->GetXScale();
	if (parent->IsGroup()) width_ = parent->GetParentWidth();

	return width_;
}

float ScaleformUI::Element::GetParentHeight()
{
	if (!parent) return menu->ScreenH();

	return parent->IsGroup() ? parent->GetParentHeight() : parent->GetHeight() / parent->GetYScale();
	
}

float ScaleformUI::Element::LocalToGlobalX(float a_localX) const
{
	float globalX = a_localX;
	TraverseParents([&](const Element* a_parent)
	{
		globalX = globalX*a_parent->xScale + a_parent->x;
	});
	return globalX;
}

float ScaleformUI::Element::LocalToGlobalY(float a_localY)const
{
	float globalY = a_localY;
	TraverseParents([&](Element* a_parent)
	{
		globalY = globalY*a_parent->yScale + a_parent->y;
	});
	return globalY;
}

float ScaleformUI::Element::GlobalToLocalX(float a_globalX)const
{
	float localX = a_globalX;
	TraverseParentsReverse([&](Element* a_parent)
	{
		localX = (localX - a_parent->x)/a_parent->xScale;
	});
	return localX;
}

float ScaleformUI::Element::GlobalToLocalY(float a_globalY)const
{
	float localY = a_globalY;
	TraverseParentsReverse([&](Element* a_parent)
	{
		localY = (localY - a_parent->y) / a_parent->yScale;
	});
	return localY;
}

float ScaleformUI::Element::LocalToGlobalXDistance(float a_localX) const
{
	return LocalToGlobalX(a_localX) - LocalToGlobalX(0.0f);
}

float ScaleformUI::Element::LocalToGlobalYDistance(float a_localY) const
{
	return LocalToGlobalY(a_localY) - LocalToGlobalY(0.0f);
}

float ScaleformUI::Element::GlobalToLocalXDistance(float a_globalX) const
{
	return GlobalToLocalX(a_globalX) - GlobalToLocalX(0.0f);
}

float ScaleformUI::Element::GlobalToLocalYDistance(float a_globalY) const
{
	return GlobalToLocalY(a_globalY) - GlobalToLocalY(0.0f);
}

float ScaleformUI::Element::LocalToGlobalXScale(float a_localX)const
{
	float globalXScale = a_localX;
	TraverseParents([&](Element* a_parent)
	{
		globalXScale *= a_parent->xScale;
	});
	return globalXScale;
}

float ScaleformUI::Element::LocalToGlobalYScale(float a_localY)const
{
	float globalYScale = a_localY;
	TraverseParents([&](Element* a_parent)
	{
		globalYScale *= a_parent->yScale;
	});
	return globalYScale;
}

float ScaleformUI::Element::GlobalToLocalXScale(float a_globalX)const
{
	float localXScale = a_globalX;
	TraverseParents([&](Element* a_parent)
	{
		localXScale /= a_parent->xScale;
	});
	return localXScale;
}

float ScaleformUI::Element::GlobalToLocalYScale(float a_globalY)const
{
	float localYScale = a_globalY;
	TraverseParents([&](Element* a_parent)
	{
		localYScale /= a_parent->yScale;
	});
	return localYScale;
}

void ScaleformUI::Element::UpdateBounds()
{
	float xmin = GetGlobalXMin();
	float xmax = GetGlobalXMax();
	float ymin = GetGlobalYMin();
	float ymax = GetGlobalYMax();

	bounds->UpdateBounds(xmin, ymin, xmax, ymax);

	if (IsBBox() && parent->bbox)
	{
		parent->bbox->UpdateBounds(xmin, ymin, xmax, ymax);
	}

	// Cannot use TraverseChildren, since it excludes bboxes
	for (auto& child : children)
	{
		if (!child->AsUIElement()->IsLayout())
		{
			child->AsUIElement()->UpdateBounds();
		}
	}

}

void ScaleformUI::Element::OnTranslate(std::string arguments)
{
	UpdateBounds();
	if (arguments == "x"s)
	{
		RemoveXAlignment();
	}
	else if (arguments == "y"s)
	{
		RemoveYAlignment();
	}
}

void ScaleformUI::Element::OnSetScale()
{
	UpdateAlignment(); // Bounds updated in here
}

float ScaleformUI::Element::SizeToPosition(Size a_size, bool a_isX)
{
	float finalPosition = 0.0f;
	for (Size* size = &a_size; size; size = size->next.get())
	{
		float position = size->size;
		if (size->IsLocal())
		{
			std::vector<Element*> parents;
			for(auto p = parent; p && p->IsGroup(); p = p->parent)
			{
				parents.push_back(p);
			}

			for (size_t i = parents.size(); i--;)
			{
				auto p = parents[i];
				if (a_isX)	(position = position - p->x) / p->xScale;
				else		(position = position - p->y) / p->yScale;
			}
		}
		if (size->IsGlobal())
		{
			if (a_isX)	position = GlobalToLocalX(size->size);
			else		position = GlobalToLocalY(size->size);
		}
		else if (size->IsPW())
		{
			if (!parent)	size->vw();
			else			position = size->size * GetParentWidth();
		}
		else if (size->IsPH())
		{
			if (!parent)	size->vh();
			else			position = size->size * GetParentHeight();
		}
		else if (size->IsVW())
		{
			if (a_isX)	position = GlobalToLocalX(size->size * menu->ScreenW());
			else		position = GlobalToLocalY(size->size * menu->ScreenW());
		}
		else if (size->IsVH())
		{
			if (a_isX)	position = GlobalToLocalX(size->size * menu->ScreenH());
			else		position = GlobalToLocalY(size->size * menu->ScreenH());
		}
		finalPosition += position;
	}
	return finalPosition;
}

float ScaleformUI::Element::SizeToPositionX(Size a_size)
{
	return SizeToPosition(a_size, true);
}

float ScaleformUI::Element::SizeToPositionY(Size a_size)
{
	return SizeToPosition(a_size, false);
}

float ScaleformUI::Element::SizeToDistanceX(Size a_size)
{
	Size zero = a_size;
	zero.size = 0.0f;
	zero.next = nullptr;
	return SizeToPosition(a_size, true) - SizeToPosition(zero, true);
}

float ScaleformUI::Element::SizeToDistanceY(Size a_size)
{
	Size zero = a_size;
	zero.size = 0.0f;
	zero.next = nullptr;
	return SizeToPosition(a_size, false) - SizeToPosition(zero, false);
}

float ScaleformUI::Element::SizeToScale(Size a_size, bool a_isXScale)
{
	float finalScale = 1.0f;
	for (Size* size = &a_size; size; size = size->next.get())
	{
		float scale = size->size;
		if (size->IsGlobal())
		{
			scale = GlobalToLocalXScale(size->size);
		}
		else if (size->IsPW())
		{
			if (!parent) size->vw();

			scale = size->size * GetParentWidth();

			if (a_isXScale) scale /= GetWidth();
			else scale /= GetHeight();
		}
		else if (size->IsPH())
		{
			if (!parent) size->vh();

			scale = size->size * GetParentHeight();

			if (a_isXScale) scale /= GetWidth();
			else scale /= GetHeight();
		}
		else if (size->IsVW())
		{
			if (a_isXScale) scale = GlobalToLocalXScale(size->size * menu->ScreenW() / GetWidth());
			else scale = GlobalToLocalXScale(size->size * menu->ScreenW() / GetHeight());
		}
		else if (size->IsVH())
		{
			if (a_isXScale) scale = GlobalToLocalXScale(size->size * menu->ScreenH() / GetWidth());
			else scale = GlobalToLocalXScale(size->size * menu->ScreenH() / GetHeight());
		}
		finalScale *= scale;
	}
	return finalScale;
}

void ScaleformUI::Element::Move(Size a_x, Size a_y)
{ 
	if (a_x.size != 0.0f) 
	{
		float xDelta = SizeToDistanceX(a_x);
		SetXImpl(x + xDelta);
		OnTranslate("x"s);
	}
	if (a_y.size != 0.0f) 
	{
		float yDelta = SizeToDistanceY(a_y);
		SetYImpl(y + yDelta);
		OnTranslate("y"s);
	}
}

void ScaleformUI::Element::SetX(Size a_x)
{
	SetXImpl(SizeToPosition(a_x, true));
	OnTranslate("x"s);
}

void ScaleformUI::Element::SetY(Size a_y)
{
	SetYImpl(SizeToPosition(a_y, false));
	OnTranslate("y"s);
}

void ScaleformUI::Element::SetXImpl(float a_x, bool a_updateInternally)
{
	#ifdef LOG_UI
		UIIndent++;
	#endif

	if (a_updateInternally)
		x = a_x;

	auto elementGFx = GetGFx();

	RE::GFxValue::DisplayInfo displayInfo;
	elementGFx.GetDisplayInfo(&displayInfo);

	displayInfo.SetX(a_x);
	bool success = elementGFx.SetDisplayInfo(displayInfo);

	#ifdef LOG_UI
		if (!success) logger::debug("{}UI ERROR: Failed to set x of element '{}'", GetUIIndent(), GetInstanceName());
		UIIndent--;
	#endif
}

void ScaleformUI::Element::SetYImpl(float a_y, bool a_updateInternally)
{
	#ifdef LOG_UI
		UIIndent++;
	#endif

	if (a_updateInternally)
		y = a_y;

	auto elementGFx = GetGFx();

	RE::GFxValue::DisplayInfo displayInfo;
	elementGFx.GetDisplayInfo(&displayInfo);

	displayInfo.SetY(a_y);
	bool success = elementGFx.SetDisplayInfo(displayInfo);


	//auto success = elementGFx.Invoke("moveTo", nullptr, posGFx, 2);

	#ifdef LOG_UI
		if (!success) logger::debug("{}UI ERROR: Failed to set y of element '{}'", GetUIIndent(), GetInstanceName());
		UIIndent--;
	#endif

}

void ScaleformUI::Element::SetXScale(Size a_x)
{
	float newScale = SizeToScale(a_x, true);
	SetXScaleImpl(newScale);
	if (lockAspectRatio) SetYScaleImpl(newScale);
	OnSetScale();
	
}

void ScaleformUI::Element::SetYScale(Size a_y)
{
	float newScale = SizeToScale(a_y, false);
	SetYScaleImpl(newScale);
	if (lockAspectRatio) SetXScaleImpl(newScale);
	OnSetScale();
}

void ScaleformUI::Element::SetWidth(Size a_width)
{
	float newWidth = SizeToDistanceX(a_width);
	float widthRatio = newWidth / baseWidth;
	SetXScaleImpl(widthRatio);
	if (lockAspectRatio) SetYScaleImpl(widthRatio);
	OnSetScale();
}

void ScaleformUI::Element::SetHeight(Size a_height)
{
	float newHeight = SizeToDistanceY(a_height);
	float heightRatio = newHeight / baseHeight;
	SetYScaleImpl(heightRatio);
	if (lockAspectRatio) SetXScaleImpl(heightRatio);
	OnSetScale();

}

void ScaleformUI::Element::SetXScaleImpl(float a_x, bool a_updateInternally)
{
	#ifdef LOG_UI
		UIIndent++;
	#endif

	if (a_updateInternally)
	{
		xScale = a_x;
	}

	auto elementGFx = GetGFx();

	RE::GFxValue::DisplayInfo displayInfo;
	elementGFx.GetDisplayInfo(&displayInfo);

	displayInfo.SetXScale(a_x * 100);
	bool success = elementGFx.SetDisplayInfo(displayInfo);

	#ifdef LOG_UI
		if (!success) logger::debug("{}UI ERROR: Failed to set x scale of element '{}'", GetUIIndent(),  GetInstanceName());
		UIIndent--;
	#endif
}

void ScaleformUI::Element::SetYScaleImpl(float a_y, bool a_updateInternally)
{
	#ifdef LOG_UI
		UIIndent++;
	#endif

	if (a_updateInternally)
	{
		yScale = a_y;
	}

	auto elementGFx = GetGFx();

	RE::GFxValue::DisplayInfo displayInfo;
	elementGFx.GetDisplayInfo(&displayInfo);

	displayInfo.SetYScale(a_y*100);
	bool success = elementGFx.SetDisplayInfo(displayInfo);

	//auto success = elementGFx.Invoke("SetYScale", nullptr, &yScaleGFx, 1);

	#ifdef LOG_UI
		if (!success) logger::debug("{}UI ERROR: Failed to set y scale of element '{}'", GetUIIndent(), GetInstanceName());
		UIIndent--;
	#endif
}

void ScaleformUI::Element::SetAlpha(uint32_t a_alpha)
{
	SetAlphaImpl(a_alpha);
}

void ScaleformUI::Element::SetAlphaImpl(uint32_t a_alpha, bool a_updateInternally)
{
	if (a_updateInternally) alpha = a_alpha;

	auto elementGFx = GetGFx();

	RE::GFxValue::DisplayInfo displayInfo;
	elementGFx.GetDisplayInfo(&displayInfo);

	displayInfo.SetAlpha(static_cast<double>(a_alpha));
	elementGFx.SetDisplayInfo(displayInfo);
}

void ScaleformUI::Element::SetLayoutAlpha(uint32_t a_alpha)
{
	auto elementGFx = GetGFx();
	if (!elementGFx.HasMember(LAYOUT_NAME)) return;

	RE::GFxValue layoutGFx;
	elementGFx.GetMember(LAYOUT_NAME, &layoutGFx);

	RE::GFxValue::DisplayInfo displayInfo;
	layoutGFx.GetDisplayInfo(&displayInfo);

	displayInfo.SetAlpha(static_cast<double>(a_alpha));
	layoutGFx.SetDisplayInfo(displayInfo);
}


void ScaleformUI::Element::SetPadding(Direction a_direction, Size a_amount)
{
	switch ((Alignment)a_direction)
	{
		case Alignment::kLeft:
		{
			padding.right = 0.0f;
			padding.left = SizeToDistanceX(a_amount);
			if (!HasAlignment(Alignment::kLeft) && !HasAlignment(Alignment::kCenterH) && !HasAlignment(Alignment::kRight))
			{
				logger::debug("UI WARNING: Element '{}' has horizontal padding but no horizontal alignment; Padding has no effect", GetInstanceName());
			}
			break;
		}
		case Alignment::kRight:
		{
			padding.left = 0.0f;
			padding.right = SizeToDistanceX(a_amount);
			if (!HasAlignment(Alignment::kLeft) && !HasAlignment(Alignment::kCenterH) && !HasAlignment(Alignment::kRight))
			{
				logger::debug("UI WARNING: Element '{}' has horizontal padding but no horizontal alignment; Padding has no effect", GetInstanceName());
			}
			break;
		}
		case Alignment::kTop:
		{
			padding.bottom = 0.0f;
			padding.top = SizeToDistanceY(a_amount);
			if (!HasAlignment(Alignment::kTop) && !HasAlignment(Alignment::kCenterV) && !HasAlignment(Alignment::kBottom))
			{
				logger::debug("UI WARNING: Element '{}' has vertical padding but no vertical alignment; Padding has no effect", GetInstanceName());
			}
			break;
		}
		case Alignment::kBottom:
		{
			padding.top = 0.0f;
			padding.bottom = SizeToDistanceY(a_amount);
			if (!HasAlignment(Alignment::kTop) && !HasAlignment(Alignment::kCenterV) && !HasAlignment(Alignment::kBottom))
			{
				logger::debug("UI WARNING: Element '{}' has vertical padding but no vertical alignment; Padding has no effect", GetInstanceName());
			}
			break;
		}
	}
	UpdateAlignment();
}

void ScaleformUI::Element::RemoveXAlignment()
{
	ResetAlignmentFlag(Alignment::kLeft);
	ResetAlignmentFlag(Alignment::kCenterH);
	ResetAlignmentFlag(Alignment::kRight);
}

void ScaleformUI::Element::RemoveYAlignment()
{
	ResetAlignmentFlag(Alignment::kTop);
	ResetAlignmentFlag(Alignment::kCenterV);
	ResetAlignmentFlag(Alignment::kBottom);
}

void ScaleformUI::Element::SetAlignment(Alignment a_align)
{
	auto setFlag = [&](Alignment a_aling)
	{
		alignmentFlags = static_cast<Alignment>(static_cast<uint16_t>(alignmentFlags) | static_cast<uint16_t>(a_align));
	};

	switch (a_align)
	{
		case Alignment::kNone:
		{
			alignmentFlags = Alignment::kNone;
			break;
		}
		case Alignment::kTop:
		{
			ResetAlignmentFlag(Alignment::kBottom);
			ResetAlignmentFlag(Alignment::kCenterV);
			setFlag(Alignment::kTop);
			break;
		}
		case Alignment::kCenterV:
		{
			ResetAlignmentFlag(Alignment::kTop);
			ResetAlignmentFlag(Alignment::kBottom);
			setFlag(Alignment::kCenterV);
			break;
		}
		case Alignment::kBottom:
		{
			ResetAlignmentFlag(Alignment::kTop);
			ResetAlignmentFlag(Alignment::kCenterV);
			setFlag(Alignment::kBottom);
			break;
		}
		case Alignment::kLeft:
		{
			ResetAlignmentFlag(Alignment::kRight);
			ResetAlignmentFlag(Alignment::kCenterH);
			setFlag(Alignment::kLeft);
			break;
		}
		case Alignment::kCenterH:
		{
			ResetAlignmentFlag(Alignment::kLeft);
			ResetAlignmentFlag(Alignment::kRight);
			setFlag(Alignment::kCenterH);
			break;
		}
		case Alignment::kRight:
		{
			ResetAlignmentFlag(Alignment::kLeft);
			ResetAlignmentFlag(Alignment::kCenterH);
			setFlag(Alignment::kRight);
			break;
		}
		case Alignment::kInside:
		{
			ResetAlignmentFlag(Alignment::kOutside);
			setFlag(Alignment::kInside);
			break;
		}
		case Alignment::kOutside:
		{
			ResetAlignmentFlag(Alignment::kInside);
			setFlag(Alignment::kOutside);
			break;
		}
	}
}

void ScaleformUI::Element::AlignInParent(Alignment a_align1, Alignment a_align2, Alignment a_align3)
{
	AlignToOther(nullptr, a_align1, a_align2, a_align3);
}

void ScaleformUI::Element::AlignInParent(Alignment a_align1, Alignment a_align2)
{
	AlignToOther(nullptr, a_align1, a_align2);
}

void ScaleformUI::Element::AlignInParent(Alignment a_align)
{
	AlignToOther(nullptr, a_align);
}

void ScaleformUI::Element::AlignToOther(Element* a_other, Alignment a_align)
{
	SetElementAlignedTo(a_other);
	SetAlignment(a_align);
	UpdateAlignment();
}

void ScaleformUI::Element::AlignToOther(Element* a_other, Alignment a_align1, Alignment a_align2)
{
	SetElementAlignedTo(a_other);
	SetAlignment(a_align1);
	SetAlignment(a_align2);
	UpdateAlignment();
}

void ScaleformUI::Element::AlignToOther(Element* a_other, Alignment a_align1, Alignment a_align2, Alignment a_align3)
{
	SetElementAlignedTo(a_other);
	SetAlignment(a_align1);
	SetAlignment(a_align2);
	SetAlignment(a_align3);
	UpdateAlignment();
}

void ScaleformUI::Element::SetElementAlignedTo(Element* a_element)
{
	// In case element has other element alignment, remove it when changing element, include changing to parent
	if (a_element && a_element != elementAlignedTo) RemoveAlignment();
	if (!a_element && elementAlignedTo) RemoveAlignment();

	elementAlignedTo = a_element;

}

void ScaleformUI::Element::UpdateAlignment()	// Fix this for align inside other

{
	float alignedElementXMin = 0.0f;
	float alignedElementYMin = 0.0f;
	float alignedElementWidth = 0.0f;
	float alignedElementHeight = 0.0f;

	if (elementAlignedTo)
	{
		alignedElementXMin = GlobalToLocalX(elementAlignedTo->GetGlobalXMin());
		alignedElementYMin = GlobalToLocalY(elementAlignedTo->GetGlobalYMin());
		alignedElementWidth = GlobalToLocalXDistance(elementAlignedTo->LocalToGlobalXDistance(elementAlignedTo->GetWidth()));
		alignedElementHeight = GlobalToLocalYDistance(elementAlignedTo->LocalToGlobalYDistance(elementAlignedTo->GetHeight()));
	}
	else
	{
		//if (parent && parent->IsGroup())
		//{
		//	alignedElementXMin = parent->GetLocalXMin();
		//	alignedElementYMin = parent->GetLocalXMax();
		//}
		// parent xMin and yMin are always 0 per definition (its a local frame)
		alignedElementWidth = GetParentWidth();
		alignedElementHeight = GetParentHeight();
	}

	bool outside = HasAlignment(Alignment::kOutside);
	bool isGroup = IsGroup();

	if (HasAlignment(Alignment::kTop))		
	{
		//float alignmentPos = outside ? -GetHeight() : 0.0f; 
		float alignmentPos = alignedElementYMin;
		if (outside) alignmentPos -= GetHeight();
		if (isGroup) alignmentPos -= GetLocalYMin() - y;
	
		float paddingAmount = padding.top - padding.bottom; // only one of them is not 0
		SetYImpl(alignmentPos + paddingAmount);
	}
	if (HasAlignment(Alignment::kCenterV))	
	{
		float alignmentPos = alignedElementYMin + alignedElementHeight / 2 - GetHeight() / 2;
		if (isGroup) alignmentPos -= GetLocalYMin() - y;

		float paddingAmount = padding.top - padding.bottom; // only one of them is not 0
		SetYImpl(alignmentPos + paddingAmount);;
	}
	if (HasAlignment(Alignment::kBottom))	
	{
		float alignmentPos = alignedElementYMin + alignedElementHeight - GetHeight();
		if (outside) alignmentPos += GetHeight();
		if (isGroup) alignmentPos -= GetLocalYMin() - y;

		float paddingAmount = padding.top - padding.bottom; // only one of them is not 0
		SetYImpl(alignmentPos + paddingAmount);
	}
	if (HasAlignment(Alignment::kLeft))		
	{
		float alignmentPos = alignedElementXMin;
		if (outside) alignmentPos -= GetWidth();
		if (isGroup) alignmentPos += x - GetLocalXMin();

		float paddingAmount = padding.left - padding.right; // only one of them is not 0
		SetXImpl(alignmentPos + paddingAmount);
	}
	if (HasAlignment(Alignment::kCenterH))	
	{
		float alignmentPos = alignedElementXMin + alignedElementWidth / 2 - GetWidth() / 2;
		if (isGroup) alignmentPos += x - GetLocalXMin();

		float paddingAmount = padding.left - padding.right; // only one of them is not 0
		SetXImpl(alignmentPos + paddingAmount);
	}
	if (HasAlignment(Alignment::kRight))	
	{
		float alignmentPos = alignedElementXMin + alignedElementWidth - GetWidth();
		if (outside) alignmentPos += GetWidth();
		if (isGroup) alignmentPos += x - GetLocalXMin();
		// x + localXMin is the invariant offset between the position of the group and its left border,
		// since both x and localXMin are given in parent-space coordinates

		float paddingAmount = padding.left - padding.right; // only one of them is not 0
		SetXImpl(alignmentPos + paddingAmount);
	}

	UpdateBounds();
}

//float ScaleformUI::Element::GetLocalXMin() const
//{
//	//float localXMin = x;
//	//for (const auto& child : children)
//	//{}
//
//	if (children.empty()) return x;//0.0f;
//	float localXMin = x;
//
//	TraverseChildren([&](Element* a_child)
//	{
//
//		// child positions ignore the parent's scale, so we must multiply them to 
//		// get child positions in parent's coordinates
//		float tempXMin = a_child->GetLocalXMin() * a_child->parent->GetXScale();
//		if (tempXMin < localXMin) localXMin = tempXMin;
//	});
//	return localXMin;
//}
//
//float ScaleformUI::Element::GetLocalYMin() const
//{
//	if (children.empty()) return y;//0.0f;
//	float localYMin = y;
//
//	TraverseChildren([&](Element* a_child)
//	{
//		float tempYMin = a_child->GetLocalYMin() * a_child->parent->GetYScale();
//
//		if (tempYMin < localYMin)
//		{
//			localYMin = tempYMin;
//		}
//	});
//
//	return localYMin;
//}
//
//float ScaleformUI::Element::GetLocalXMax() const
//{
//	if (children.empty()) return x+width;0.0f;
//	float localXMax = x+width;
//
//	TraverseChildren([&](Element* a_child)
//	{
//		// child positions ignore the parent's scale, so we must multiply them to 
//		// get child positions in parent's coordinates
//		float tempXMax = a_child->GetLocalXMax() * a_child->parent->GetXScale();
//
//		if (tempXMax > localXMax) localXMax = tempXMax;
//
//	});
//
//	return localXMax;
//}
//
//float ScaleformUI::Element::GetLocalYMax() const
//{
//	if (children.empty()) return y+height;//0.0f;
//	float localYMax = y+height;
//
//	TraverseChildren([&](Element* a_child)
//	{
//		// child positions ignore the parent's scale, so we must multiply them to 
//		// get child positions in parent's coordinates
//		float tempYMax = a_child->GetLocalYMax() * a_child->parent->GetYScale();
//
//		if (tempYMax > localYMax) localYMax = tempYMax;
//
//	});
//
//	return localYMax;
//}


