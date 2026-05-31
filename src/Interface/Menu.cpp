#include "Menu.h"
#include "InputHandler.h"

ScaleformUI::Menu::Menu(const char* a_menuName)
{
	#ifdef LOG_UI
		logger::debug("Creating menu: {}", a_menuName);
	#endif
	
	menuName = a_menuName;
	//defaultTextFormat.SetNull();
}

void ScaleformUI::Menu::Build(MenuCallback a_attachNewCanvasElements)
{
	OpenIMenu();

	auto uiTask = [=]()
	{
		auto start = std::chrono::system_clock::now();

		logger::debug("Building menu '{}'", menuName);


		if (auto ui = RE::UI::GetSingleton())
		{
			iMenu = ui->GetMenu(menuName);

			if (!iMenu)
			{
				logger::debug("CRITICAL UI ERROR: Failed to get menu: {}", menuName);
				return;
			}

			usesCursor = iMenu->UsesCursor();

			// Flag must be set to scale correctly
			iMenu->menuFlags.set(RE::UI_MENU_FLAGS::kAlwaysOpen);

			if (auto& movie = iMenu->uiMovie)
			{
			#ifdef LOG_UI
				auto frame_ = movie->GetVisibleFrameRect();
				logger::debug("UI: Default canvas size: L-R: [{}, {}], T-B: [{}, {}]; {}x{}", frame_.left, frame_.right, frame_.top, frame_.bottom, frame_.right - frame_.left, frame_.bottom - frame_.top);
			#endif

				ConstructDefaultTextFormat();

				


				movie->SetViewAlignment(RE::GFxMovieView::AlignType::kTopLeft);
				movie->SetViewScaleMode(RE::GFxMovieView::ScaleModeType::kNoScale);
				auto frame = movie->GetVisibleFrameRect();

				RE::GViewport viewPort;
				movie->GetViewport(&viewPort);
				xResolution = frame.right - frame.left;
				yResolution = frame.bottom - frame.top;

			#ifdef LOG_UI
				logger::debug("UI: Resized canvas size: L-R: [{}, {}], T-B: [{}, {}], {}x{}", frame.left, frame.right, frame.top, frame.bottom, xResolution, yResolution);
			#endif
			}
			else
			{
				logger::debug("UI ERROR: Failed to get movie of menu '{}'", menuName);
			}

			// Attaches elements from the swf library to the canvas. 
			a_attachNewCanvasElements(this);

			#ifdef PRINT_MENU_HEIRARCHY
				PrintHierarchy();
			#endif

			#ifdef DISPLAY_UI_ON_MAINMENU
				// Set high depth priority so it will be visible on the main screen
				iMenu->depthPriority += 10;
			#endif	

			// Add menu to inputhandler
			InputHandler::GetSingleton()->AddMenu(this);

			auto end = std::chrono::system_clock::now();
			int delta = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
			logger::debug("Built menu '{}' UI in {} µs", menuName, delta);

			CloseIMenu();
		}

	};
	SKSE::GetTaskInterface()->AddUITask(uiTask);
}

void ScaleformUI::Menu::ConstructDefaultTextFormat()
{
	if (iMenu && iMenu->uiMovie)
	{
		iMenu->uiMovie->CreateObject(&defaultTextFormat, "TextFormat");

		RE::GFxValue leftMargin{ 0 };
		RE::GFxValue size{ 12 };
		RE::GFxValue bold{ false };
		RE::GFxValue underline{ false };
		RE::GFxValue bullet; bullet.SetNull();
		RE::GFxValue align{ "center" };
		RE::GFxValue leading{ 2.0f };
		RE::GFxValue url; url.SetNull();
		RE::GFxValue blockIndent; blockIndent.SetNull();
		RE::GFxValue target; target.SetNull();
		RE::GFxValue font{ "Arial" };
		RE::GFxValue kerning{ 1 };
		RE::GFxValue color{ 0xFFFFFF };
		RE::GFxValue tabStops; tabStops.SetNull();
		RE::GFxValue letterSpacing{ 0 };
		RE::GFxValue rightMargin{ 0 };
		RE::GFxValue indent{ 0 };
		RE::GFxValue italic{ false };

		defaultTextFormat.SetMember("leftMargin", leftMargin);
		defaultTextFormat.SetMember("size", size);
		defaultTextFormat.SetMember("bold", bold);
		defaultTextFormat.SetMember("underline", underline);
		defaultTextFormat.SetMember("bullet", bullet);
		defaultTextFormat.SetMember("align", align);
		defaultTextFormat.SetMember("leading", leading);
		defaultTextFormat.SetMember("url", url);
		defaultTextFormat.SetMember("blockIndent", blockIndent);
		defaultTextFormat.SetMember("target", target);
		defaultTextFormat.SetMember("font", font);
		defaultTextFormat.SetMember("kerning", kerning);
		defaultTextFormat.SetMember("color", color);
		defaultTextFormat.SetMember("tabStops", tabStops);
		defaultTextFormat.SetMember("letterSpacing", letterSpacing);
		defaultTextFormat.SetMember("rightMargin", rightMargin);
		defaultTextFormat.SetMember("indent", indent);
		defaultTextFormat.SetMember("italic", italic);
	}
}

void ScaleformUI::Menu::SetDefaultFont(const char* a_font)
{
	RE::GFxValue value{ a_font };
	defaultTextFormat.SetMember("font", value);
}

void ScaleformUI::Menu::SetDefaultFontSize(uint32_t a_size)
{
	RE::GFxValue value{ a_size };
	defaultTextFormat.SetMember("size", value);
}

void ScaleformUI::Menu::SetDefaultFontColor(uint32_t a_defaultColor)
{
	RE::GFxValue value{ a_defaultColor };
	defaultTextFormat.SetMember("color", value);
}

void ScaleformUI::Menu::SetDefaultBold(bool a_enabled)
{
	RE::GFxValue value{ a_enabled };
	defaultTextFormat.SetMember("bold", value);
}

void ScaleformUI::Menu::SetDefaultItalic(bool a_enabled)
{
	RE::GFxValue value{ a_enabled };
	defaultTextFormat.SetMember("italic", value);
}

void ScaleformUI::Menu::SetDefaultUnderline(bool a_enabled)
{
	RE::GFxValue value{ a_enabled };
	defaultTextFormat.SetMember("underline", value);
}

void ScaleformUI::Menu::SetDefaultTextAlign(const char* a_align)
{
	RE::GFxValue value{ a_align };
	defaultTextFormat.SetMember("align", value);
}

void ScaleformUI::Menu::SetDefaultIndent(float a_indent)
{
	RE::GFxValue value{ a_indent };
	defaultTextFormat.SetMember("indent", value);
}

void ScaleformUI::Menu::SetDefaultBlockIndent(float a_indent)
{
	RE::GFxValue value{ a_indent };
	defaultTextFormat.SetMember("blockIndent", value);
}

void ScaleformUI::Menu::SetDefaultBullet(bool a_enabled)
{
	RE::GFxValue value{ a_enabled };
	defaultTextFormat.SetMember("bullet", value);
}

void ScaleformUI::Menu::SetDefaultKerning(bool a_enabled)
{
	RE::GFxValue value{ a_enabled };
	defaultTextFormat.SetMember("kerning", value);
}

void ScaleformUI::Menu::SetDefaultLeftMargin(float a_margin)
{
	RE::GFxValue value{ a_margin };
	defaultTextFormat.SetMember("leftMargin", value);
}

void ScaleformUI::Menu::SetDefaultRightMargin(float a_margin)
{
	RE::GFxValue value{ a_margin };
	defaultTextFormat.SetMember("rightMargin", value);
}

void ScaleformUI::Menu::SetDefaultLetterSpacing(float a_spacing)
{
	RE::GFxValue value{ a_spacing };
	defaultTextFormat.SetMember("letterSpacing", value);
}

void ScaleformUI::Menu::SetDefaultLineSpacing(float a_spacing)
{
	RE::GFxValue value{ a_spacing };
	defaultTextFormat.SetMember("leading", value);
}

void ScaleformUI::Menu::SetDefaultAutoSize(bool a_enabled)
{
	textFieldDefaultAutoSize = a_enabled;
}

void ScaleformUI::Menu::SetDefaultWordWrap(bool a_enabled)
{
	textFieldDefaultWordWrap = a_enabled;
}

bool ScaleformUI::Menu::HasAnimationPlaying()
{
	bool isAnyAnimationPlaying = false;
	TraverseUIElements([&](Element* a_element)
	{
		isAnyAnimationPlaying |= a_element->HasActiveAnimation();
	});
	return isAnyAnimationPlaying;
}

void ScaleformUI::Menu::Update(float a_delta)
{
	UpdateAnimation(a_delta);
	
	if (queueClose)
	{
		if (HasAnimationPlaying())
		{
			DisableCursorIfNecessary();
		}
		else
		{
			queueClose = false;
			CloseIMenu();
		}
	}
	else if (!IsReady())
	{
		bool hasOpenenAnimationPlaying = false;
		TraverseUIElements([&](Element* a_element)
		{
			if (!a_element->isShowAnimationDone)
			{
				hasOpenenAnimationPlaying = true;
				return;
			}
		});
		if (!hasOpenenAnimationPlaying) isReady = true;
	}
}

void ScaleformUI::Menu::UpdateAnimation(float a_delta)
{
	TraverseUIElements([&](Element* a_element)
	{
		a_element->UpdateAnimation(a_delta);
	});
}

bool ScaleformUI::Menu::IsReady()
{
	if (isReady) return true;
}

bool ScaleformUI::Menu::IsOpen() const
{
	return iMenu ? iMenu->OnStack() : false;
}

bool ScaleformUI::Menu::IsClosed() const
{
	return !IsOpen();
}

void ScaleformUI::Menu::ToggleMenu()
{
	if (IsClosed() || IsCurrentlyClosing()) Open();
	else Close();
}

void ScaleformUI::Menu::Open()
{
	if (IsOpen()) // No need to open if already open
	{
		return; 
	}
	bool openedSuccessfully = true;
	if (openMenuFunction) openedSuccessfully = openMenuFunction();

	if (!openedSuccessfully) return;

	if (usesCursor) iMenu->menuFlags.set(RE::UI_MENU_FLAGS::kUsesCursor);

	if (IsClosed()) OpenIMenu();

	if (usesCursor) 
	{
		if (!IsCurrentlyClosing())
		{
			// If cursor is already open, no need to open it again
			if (RE::UI::GetSingleton()->IsMenuOpen(RE::CursorMenu::MENU_NAME))
			{
				if (auto UIMessageQueue = RE::UIMessageQueue::GetSingleton())
				{
					UIMessageQueue->AddMessage(RE::CursorMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr);
					RE::ControlMap::GetSingleton()->ToggleControls(RE::UserEvents::USER_EVENT_FLAG::kLooking, false, false);
				}
			}
		}
	}

	TraverseUIElements([](Element* a_element)
	{
		if (a_element->IsButton())
		{
			auto button = a_element->AsButton();
			if (button->IsToggleButton()) button->SetVisibleCorrectToggleImage();
			else if (button->IsCyclicButton()) button->SetVisibleCorrectCycleImage();
		}
		if (a_element->shouldBeVisible) a_element->Show();
		if (a_element->sendCheckedEventWhenMenuOpens) a_element->OnMouseStateChanged(IElement::MOUSE_STATE::kCHECKED, a_element->IsChecked());
	});
}

void ScaleformUI::Menu::Close()
{
	queueClose = true;
	isReady = false;

	TraverseUIElements([](Element* a_element)
	{
		a_element->HideWithoutTurningInvisible();
	});

}

bool ScaleformUI::Menu::IsCurrentlyClosing()
{
	return (IsOpen() && queueClose);
}

void ScaleformUI::Menu::OpenIMenu()
{
	if (auto UIMessageQueue = RE::UIMessageQueue::GetSingleton())
		UIMessageQueue->AddMessage(menuName, RE::UI_MESSAGE_TYPE::kShow, nullptr);
}

void ScaleformUI::Menu::CloseIMenu()
{
	if (auto UIMessageQueue = RE::UIMessageQueue::GetSingleton())
		UIMessageQueue->AddMessage(menuName, RE::UI_MESSAGE_TYPE::kHide, nullptr);
}

bool ScaleformUI::Menu::DoesOtherMenuUseCursor()
{
	for (const auto& [menuName_, menu_] : RE::UI::GetSingleton()->menuMap)
	{
		if (menu_.menu)
		{
			if (menu_.menu->UsesCursor() && RE::UI::GetSingleton()->IsMenuOpen(menuName_))
			{
				return true;
			}
		}
	}
	return false;
}

void ScaleformUI::Menu::DisableCursorIfNecessary()
{
	if (!usesCursor || IsClosed()) return;

	iMenu->menuFlags.reset(RE::UI_MENU_FLAGS::kUsesCursor);
	if (!DoesOtherMenuUseCursor())
	{
		if (auto UIMessageQueue = RE::UIMessageQueue::GetSingleton())
		{
			UIMessageQueue->AddMessage(RE::CursorMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
			RE::ControlMap::GetSingleton()->ToggleControls(RE::UserEvents::USER_EVENT_FLAG::kLooking, true, false);

		}
	}
}

ScaleformUI::IElement* ScaleformUI::Menu::GetElementByName(std::string a_instanceName)
{
	IElement* res = nullptr;
	TraverseUIElements([&](Element* a_element)
	{
		if (a_element->instanceName == a_instanceName) 
		{
			res = a_element;
			return;
		}
	});
	return res;
}

ScaleformUI::ElementPTR ScaleformUI::Menu::CreateUIElement(std::string a_instanceName, ELEMENT_TYPE a_type)
{
	ElementPTR element = nullptr;

	switch (a_type)
	{
		case ELEMENT_TYPE::kGROUP:
		{
			element = std::make_unique<Group>(a_instanceName);
			break;
		}
		case ELEMENT_TYPE::kBUTTON:
		{
			element = std::make_unique<Button>(a_instanceName);
			break;
		}
		case ELEMENT_TYPE::kTEXTFIELD:
		{
			element = std::make_unique<Textfield>(a_instanceName);
			break;
		}
		case ELEMENT_TYPE::kNONE:
		default:
		{
			element = std::make_unique<Element>(a_instanceName);
			break;
		}

	}
	if (!element) logger::debug("CRITICAL UI ERROR: Failed to create UI element '{}'", a_instanceName);

	element->AsUIElement()->menu = this;

	return std::move(element);
}

void ScaleformUI::Menu::RegisterUIElement(ElementPTR& a_element)
{
	#ifdef LOG_UI
		UIIndent++;
		logger::debug("{}UI: Registering element '{}'", GetUIIndent(), a_element->AsUIElement()->GetInstanceName());
		UIIndent--;
	#endif

	//a_element->menu = this;

	//elements.emplace_back(a_element);
}

void ScaleformUI::Menu::RegisterUIElement(std::string a_instanceName, ELEMENT_TYPE a_type)
{
	auto newElement = CreateUIElement(a_instanceName, a_type);
	RegisterUIElement(newElement);
}

ScaleformUI::IElement* ScaleformUI::Menu::AttachUIElement(std::string a_linkageName, ELEMENT_TYPE a_type)
{
	return AttachUIElement(a_linkageName, a_linkageName, a_type, nullptr);
}

ScaleformUI::IElement* ScaleformUI::Menu::AttachUIElement(std::string a_linkageName, std::string a_instanceName, ELEMENT_TYPE a_type)
{
	return AttachUIElement(a_linkageName, a_instanceName, a_type, nullptr);
}

ScaleformUI::IElement* ScaleformUI::Menu::AttachUIElement(std::string a_linkageName, std::string a_instanceName, ELEMENT_TYPE a_type, Element* a_parentElement)
{
	#ifdef LOG_UI
		UIIndent++;
		auto parentName = a_parentElement ? a_parentElement->GetInstanceName() : "_root";
		logger::debug("{}UI: Attaching element '{}' of MovieClip '{}' to element '{}'", GetUIIndent(), a_instanceName, a_linkageName, parentName);
	#endif

	ElementPTR newElementPTR = CreateUIElement(a_instanceName, a_type);
	Element* newElement = newElementPTR->AsUIElement();

	if (a_parentElement)
	{
		a_parentElement->AddChild(std::move(newElementPTR));
	}
	else
	{
		rootElements.emplace_back(std::move(newElementPTR));
	}


	auto parentGFx = newElement->GetParentGFx();

	RE::GFxValue nextHighestDepth;
	parentGFx.Invoke("getNextHighestDepth", &nextHighestDepth);

	// AttachMovie is the frame time eater
	RE::GFxValue newGFx;
	switch (a_type)
	{
		case ELEMENT_TYPE::kGROUP:
		{
			parentGFx.CreateEmptyMovieClip(&newGFx, a_instanceName.c_str(), nextHighestDepth.GetNumber());
			newElement->SetGFx(newGFx);
			break;
		}
		case ELEMENT_TYPE::kTEXTFIELD:
		{
			// instanceName, depth, x, y, width, height
			RE::GFxValue textFieldArgs[6]{ a_instanceName.c_str(), nextHighestDepth, 0.0f, 0.0f, 10.0f, 10.0f };
			parentGFx.Invoke("createTextField", &newGFx, textFieldArgs, 6);

			newGFx.SetMember("autoSize", { textFieldDefaultAutoSize });
			newGFx.SetMember("wordWrap", { textFieldDefaultWordWrap });

			// Textfield must have text, or its textformat will be null, even if sat
			newGFx.SetMember("text", { "TextField" });

			// Do not use setNewTextFormat
			newGFx.Invoke("setTextFormat", nullptr, &defaultTextFormat, 1);

			newElement->SetGFx(newGFx);

			break;
		}
		default:
		{
			if (a_linkageName.empty())
			{
				parentGFx.CreateEmptyMovieClip(&newGFx, a_instanceName.c_str(), nextHighestDepth.GetNumber());
			}
			else
			{
				parentGFx.AttachMovie(&newGFx, a_linkageName.c_str(), a_instanceName.c_str(), nextHighestDepth.GetNumber());
			}
			newElement->SetGFx(newGFx);
		}
	}

	newElement->Init();


	#ifdef LOG_UI
		UIIndent--;
	#endif

	return newElement;
}

ScaleformUI::IElement* ScaleformUI::Menu::CreateGroup(std::string a_instanceName)
{
	return CreateGroup(a_instanceName, nullptr);
}

ScaleformUI::IElement* ScaleformUI::Menu::CreateGroup(std::string a_instanceName, Element* a_parentElement)
{
	return AttachUIElement("", a_instanceName, ELEMENT_TYPE::kGROUP, a_parentElement);
}

ScaleformUI::IElement* ScaleformUI::Menu::CreateSquareElement(std::string a_instanceName, uint32_t a_color, ELEMENT_TYPE a_type)
{
	return CreateSquareElement(a_instanceName, a_color, a_type, nullptr);
}

ScaleformUI::IElement* ScaleformUI::Menu::CreateSquareElement(std::string a_instanceName, uint32_t a_color, ELEMENT_TYPE a_type, Element* a_parentElement)
{
	auto* square = CreateEmptyUIElement(a_instanceName, a_type, a_parentElement);
	
	auto* squareLayout = square->CreateEmptyUIElement(IElement::LAYOUT_NAME);
	
	auto squareGFx = squareLayout->GetGFx();

	float xmin = 0.0f;
	float ymin = 0.0f;
	float xmax = 100.0f;
	float ymax = 100.0f;

	// thickness, color, alpha
	RE::GFxValue argsLineStyle[3]{ 0, a_color, 100 };
	squareGFx.Invoke("lineStyle", nullptr, argsLineStyle, 3);

	RE::GFxValue argsFill[2]{ a_color, 100 };
	squareGFx.Invoke("beginFill", nullptr, argsFill, 2);

	RE::GFxValue argsCorner00[2]{ xmin, ymin };
	squareGFx.Invoke("moveTo", nullptr, argsCorner00, 2);

	RE::GFxValue argsCorner10[2]{ xmax, ymin };
	squareGFx.Invoke("lineTo", nullptr, argsCorner10, 2);

	RE::GFxValue argsCorner11[2]{ xmax, ymax };
	squareGFx.Invoke("lineTo", nullptr, argsCorner11, 2);

	RE::GFxValue argsCorner01[2]{ xmin, ymax };
	squareGFx.Invoke("lineTo", nullptr, argsCorner01, 2);

	squareGFx.Invoke("lineTo", nullptr, argsCorner00, 2);

	squareGFx.Invoke("endFill", nullptr, nullptr, 0);

	square->AsUIElement()->Init();
	if (a_parentElement) a_parentElement->UpdateBounds();

	return square;
}

ScaleformUI::IElement* ScaleformUI::Menu::CreateEmptyUIElement(std::string a_instanceName, ELEMENT_TYPE a_type)
{
	return CreateEmptyUIElement(a_instanceName, a_type, nullptr);
}

ScaleformUI::IElement* ScaleformUI::Menu::CreateEmptyUIElement(std::string a_instanceName, ELEMENT_TYPE a_type, Element* a_parentElement)
{
	return AttachUIElement("", a_instanceName, a_type, a_parentElement);
}

ScaleformUI::IElement* ScaleformUI::Menu::CreateTextfield(std::string a_instanceName)
{
	return CreateTextfield(a_instanceName, nullptr);
}

ScaleformUI::IElement* ScaleformUI::Menu::CreateTextfield(std::string a_instanceName, Element* a_parentElement)
{
	return AttachUIElement("", a_instanceName, ELEMENT_TYPE::kTEXTFIELD, a_parentElement);
}

void ScaleformUI::Menu::TraverseUIElements(std::function<void(Element*)> a_callback)
{
	for (auto& element : rootElements)
	{
		if (!element) continue;

		a_callback(element->AsUIElement());
		element->AsUIElement()->TraverseChildren(a_callback);
	}
}


void ScaleformUI::Menu::GetRoot(RE::GFxValue& a_root)
{
	bool success = false;
	if (iMenu && iMenu->uiMovie)
	{
		success = iMenu->uiMovie->GetVariable(&a_root, "_root");
	}
	if (!success) logger::debug("CRITICAL UI ERROR: Failed to get root of menu: {}", menuName);
}

void ScaleformUI::Menu::CreateHierarchy()
{
	return;
	//RE::GFxValue _root;
	//GetRoot(_root);
	//
	//const auto findChildren = [&](const auto& self, RE::GFxValue& a_parent, ElementPTR a_parentElement) -> void
	//{
	//	for (auto& element : elements)
	//	{
	//		if (a_parent.HasMember(element->GetInstanceName()))
	//		{
	//			RE::GFxValue child;
	//			a_parent.GetMember(element->GetInstanceName(), &child);
	//			self(self, child, element);
	//		}
	//	}
	//};

	//findChildren(findChildren, _root, nullptr);
}

void ScaleformUI::Menu::PrintHierarchy()
{
	logger::debug("Hierachy of menu: {}", menuName);
	logger::debug(" root");


	const auto printElement = [&](const auto& self, Element* a_element, int a_indent) -> void
	{
		std::string indent = ""s;
		for (int i = 0; i < a_indent - 1; i++) indent += "| ";
		indent += "|-";
		logger::debug(" {}{}", indent, a_element->GetInstanceName());

		for (const auto& child : a_element->children)
		{
			self(self, child->AsUIElement(), a_indent+1);
		}
	};

	for (auto& element : rootElements)
	{
		printElement(printElement, element->AsUIElement(), 1);
	}

	//RE::GFxValue _root;
	//GetRoot(_root);

	//const auto printChildren = [&](const auto& self, RE::GFxValue& a_parent, ElementPTR a_parentElement, int a_indent) -> void
	//{
	//	for (auto& element : elements)
	//	{
	//		if (a_parent.HasMember(element->GetInstanceName()))
	//		{
	//			std::string indent = ""s;
	//			for (int i = 0; i < a_indent - 1; i++) indent += "| ";
	//			indent += "|-";
	//			logger::debug(" {}{}", indent, element->GetInstanceName());

	//			RE::GFxValue child;
	//			a_parent.GetMember(element->GetInstanceName(), &child);
	//			self(self, child, element, a_indent + 1);
	//		}
	//	}
	//};
	//printChildren(printChildren, _root, nullptr, 1);
}

void ScaleformUI::Menu::GetDebugElement(RE::GFxValue& a_elementOut)
{
	auto& movie = iMenu->uiMovie;
	RE::GFxValue root;
	GetRoot(root);

	if (DebugElementInstanceName.empty())
	{
		root.CreateEmptyMovieClip(&a_elementOut, "DebugElement", -1);
		DebugElementInstanceName = "DebugElement"s;
	}
	else
	{
		root.GetMember(DebugElementInstanceName.c_str(), &a_elementOut);
	}
}


void ScaleformUI::Menu::DrawLine(RE::NiPoint2 a_start, RE::NiPoint2 a_end, float a_thickness, uint32_t a_color, uint32_t a_alpha)
{
	if (!iMenu || !iMenu->uiMovie)
	{
		logger::debug("UI DEBUG ERROR: Failed to draw line because iMenu or uiMovie is null");
		return;
	}

	RE::GFxValue debugElement;
	GetDebugElement(debugElement);
	
	RE::GFxValue argsLineStyle[3]{ a_thickness, a_color, a_alpha };
	debugElement.Invoke("lineStyle", nullptr, argsLineStyle, 3);

	RE::GFxValue argsStartPos[2]{ a_start.x, a_start.y };
	debugElement.Invoke("moveTo", nullptr, argsStartPos, 2);

	RE::GFxValue argsEndPos[2]{ a_end.x, a_end.y };
	debugElement.Invoke("lineTo", nullptr, argsEndPos, 2);

	debugElement.Invoke("endFill", nullptr, nullptr, 0);
}

void ScaleformUI::Menu::DrawGrid(uint32_t a_horizontalGridLines, uint32_t a_verticalGridLines, float a_thickness, uint32_t a_color, uint32_t a_alpha)
{
	float xSpacing = xResolution / (a_verticalGridLines + 1);
	float ySpacing = yResolution / (a_horizontalGridLines + 1);

	float xmin = 0;
	float xmax = xResolution;

	float ymin = 0;
	float ymax = yResolution;


	for (uint32_t i = 0; i < a_verticalGridLines; i++)
	{
		float x = xSpacing*(i+1);
		RE::NiPoint2 pt1{ x, ymin };
		RE::NiPoint2 pt2{ x, ymax };

		DrawLine(pt1, pt2, a_thickness, a_color, a_alpha);

	}

	for (uint32_t i = 0; i < a_horizontalGridLines; i++)
	{
		float y = ySpacing * (i + 1);
		RE::NiPoint2 pt1{ xmin, y };
		RE::NiPoint2 pt2{ xmax, y };

		DrawLine(pt1, pt2, a_thickness, a_color, a_alpha);

	}
}
//void ScaleformUI::Menu::DrawSquare(SquareBBox a_bounds, float a_thickness, uint32_t a_color, uint32_t a_alpha)
//{
//	DrawSquare(a_bounds.xmin, a_bounds.ymin, a_bounds.xmax, a_bounds.ymax, a_thickness, a_color, a_alpha);
//}


void ScaleformUI::Menu::DrawSquare(float a_xmin, float a_ymin, float a_xmax, float a_ymax, float a_thickness, uint32_t a_color, uint32_t a_alpha)
{
	RE::NiPoint2 topleft{ a_xmin, a_ymin };
	RE::NiPoint2 topRight{ a_xmax, a_ymin };
	RE::NiPoint2 bottomRight{ a_xmax, a_ymax };
	RE::NiPoint2 bottomleft{ a_xmin, a_ymax };

	DrawLine(topleft, topRight, a_thickness, a_color, a_alpha);
	DrawLine(topRight, bottomRight, a_thickness, a_color, a_alpha);
	DrawLine(bottomRight, bottomleft, a_thickness, a_color, a_alpha);
	DrawLine(bottomleft, topleft, a_thickness, a_color, a_alpha);
}

void ScaleformUI::Menu::DrawObjectBounds(uint32_t a_color)
{
	drawObjects.draw = true;
	drawObjects.color = a_color;
}

void ScaleformUI::Menu::DrawGroupBounds(uint32_t a_color)
{
	drawGroups.draw = true;
	drawGroups.color = a_color;
}

void ScaleformUI::Menu::DrawInteractableBounds(uint32_t a_color)
{
	drawInteractables.draw = true;
	drawInteractables.color = a_color;
}

void ScaleformUI::Menu::DrawDebugInfo()
{
	TraverseUIElements([&](Element* a_element)
	{
		bool shouldDraw = false;
		BBox::AABB bounds;
		uint32_t color = 0x0;

		if (a_element->IsInteractable() && drawInteractables)
		{
			bounds = a_element->GetInteractableWorldAABB();
			color = drawInteractables.color;
			shouldDraw = true;
		}
		else if (a_element->IsGroup() && drawGroups)
		{
			bounds = a_element->GetWorldBounds()->GetAABB();
			color = drawGroups.color;
			shouldDraw = true;

		}
		else if (drawObjects && !a_element->IsGroup())
		{
			bounds = a_element->GetWorldBounds()->GetAABB();
			color = drawObjects.color;
			shouldDraw = true;
		}
		if (shouldDraw)
		{
			auto aabb = bounds;
			DrawSquare(aabb.xmin, aabb.ymin, aabb.xmax, aabb.ymax, 2.0f, color, 80);

			//logger::debug("{}: xmin, xmax, ymin, ymax: [{:.0f}:{:.0f}], [{:.0f}:{:.0f}]", a_element->GetInstanceName(), aabb.xmin, aabb.xmax, aabb.ymin, aabb.ymax
		}
	});
}

bool ScaleformUI::Menu::HasDebugDraws() const
{
	return drawObjects || drawGroups || drawInteractables;
}

void ScaleformUI::Menu::ClearDebugDraws()
{
	if (!iMenu || !iMenu->uiMovie)
	{
		logger::debug("UI DEBUG ERROR: Failed to clear drawn lines because iMenu or uiMovie is null or there is no DebugElement");
		return;
	}

	RE::GFxValue debugElement;
	GetDebugElement(debugElement);
	debugElement.Invoke("clear", nullptr, nullptr, 0);
}

void ScaleformUI::Menu::PrintFontList()
{
	if (auto movie = iMenu->uiMovie)
	{
		RE::GFxValue fontList;
		movie->Invoke("TextField.getFontList", &fontList, nullptr, 0);
		logger::debug("All availabel fonts:");

		RE::GFxValue font;

		for (int i = 0; i < fontList.GetArraySize(); i++)
		{
			fontList.GetElement(i, &font);
			logger::debug(" {}", font.GetString()); 
		}
	}
}



