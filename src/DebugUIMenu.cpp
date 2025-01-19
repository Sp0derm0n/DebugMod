#include "DebugUIMenu.h"
#include "UIHandler.h"
#include "MCM.h"

DebugMenu::DebugMenu()
{
	auto menu = static_cast<RE::IMenu*>(this);
	menu->depthPriority = 1; // must be higher than depth of draw menu
	auto scaleformManager = RE::BSScaleformManager::GetSingleton();

	const auto success = scaleformManager->LoadMovieEx(this, MENU_PATH, RE::GFxMovieView::ScaleModeType::kNoBorder, 0.0, [](RE::GFxMovieDef* a_def) -> void {
		a_def->SetState(RE::GFxState::StateType::kLog,
		RE::make_gptr<Logger>().get());
		});
	if (!success) logger::info(" -LoadMovieEx unsuccessful");

	menuFlags.set(
		RE::UI_MENU_FLAGS::kAlwaysOpen,
		RE::UI_MENU_FLAGS::kRequiresUpdate,
		RE::UI_MENU_FLAGS::kAllowSaving,
		RE::UI_MENU_FLAGS::kUsesCursor);

	menu->inputContext = Context::kNone;
	if (uiMovie) movie = menu->uiMovie;
	else logger::info("uiMovie not obtained");

	Init();
}

// To add a new item, make sure to give it a script to register it to a class.
// Remember to also give the symbol a linkage name.
// Then, give the symbol an instance name when added to the MenuContainer symbol.
void DebugMenu::Init()
{
	RegisterButton("dayNightIcon",			BUTTON::kDayNight);
	RegisterButton("cellBorderIcon",		BUTTON::kCellBorder);
	RegisterButton("wallsIcon",				BUTTON::kCellWalls);
	RegisterButton("quadsIcon",				BUTTON::kCellQuads);
	RegisterButton("navMeshIcon",			BUTTON::kNavMesh);
	RegisterButton("navMeshPlusIcon",		BUTTON::kNavMeshPlus);
	RegisterButton("navMeshMinusIcon",		BUTTON::kNavMeshMinus);
	RegisterButton("navMeshModeIcon",		BUTTON::kNavMeshMode);
	RegisterButton("trianglesIcon",			BUTTON::kNavMeshTriangles);
	RegisterButton("coverIcon",				BUTTON::kNavMeshCover);
	RegisterButton("occlusionIcon",			BUTTON::kOcclusion);
	RegisterButton("occlusionPlusIcon",		BUTTON::kOcclusionPlus);
	RegisterButton("occlusionMinusIcon",	BUTTON::kOcclusionMinus);
	RegisterButton("coordinatesIcon",		BUTTON::kCoordinates);
}

void DebugMenu::RegisterButton(const char* a_buttonName, BUTTON a_buttonType)
{
	RE::GFxValue root;
	GetRoot(root);

	// Get UI elements
	RE::GFxValue menuContainer;

	root.GetMember("menuContainer", &menuContainer);

	// get info on UI elements
	RE::GFxValue::DisplayInfo containerInfo;

	menuContainer.GetDisplayInfo(&containerInfo);

	float container_x = static_cast<float>(containerInfo.GetX());
	float container_y = static_cast<float>(containerInfo.GetY());

	// get UI elements
	RE::GFxValue item;
	menuContainer.GetMember(a_buttonName, &item);

	// get info on UI elements
	RE::GFxValue::DisplayInfo itemInfo;
	item.GetDisplayInfo(&itemInfo);

	RE::GFxValue itemWidth;
	RE::GFxValue itemHeight;

	item.Invoke("GetWidth", &itemWidth, nullptr, 0);
	item.Invoke("GetHeight", &itemHeight, nullptr, 0);

	float width = static_cast<float>(itemWidth.GetNumber());
	float height = static_cast<float>(itemHeight.GetNumber());

	// symbols should be centered in their own frame
	float item_xmin = static_cast<float>(itemInfo.GetX()) + container_x - width/2;
	float item_ymin = static_cast<float>(itemInfo.GetY()) + container_y - height/2;

	float item_xmax = item_xmin + width;
	float item_ymax = item_ymin + height;

	bool isActive = false;
	BUTTON_STATE buttonState = isActive ?  BUTTON_STATE::kOFF :  BUTTON_STATE::kON;

	menuItems.buttons.push_back(MenuButton(item_xmin, item_xmax, item_ymin, item_ymax, a_buttonName, a_buttonType, buttonState, isActive));
}


void DebugMenu::Register()
{
	auto ui = RE::UI::GetSingleton();
	if (ui) 
	{
		ui->Register(MENU_NAME, Creator);
		logger::info("Registered menu '{}'", MENU_NAME);
	}


}

RE::UI_MESSAGE_RESULTS DebugMenu::ProcessMessage(RE::UIMessage& a_message)
{
	if (a_message.type == RE::UI_MESSAGE_TYPE::kShow)
	{
		UIHandler::GetSingleton()->isMenuOpen = true;	
	}

	else if (a_message.type == RE::UI_MESSAGE_TYPE::kHide)
	{
		UIHandler::GetSingleton()->isMenuOpen = false;	
	}
			

	return RE::UI_MESSAGE_RESULTS::kHandled;
}

void DebugMenu::OpenMenu()
{
	if (auto UIMessageQueue = RE::UIMessageQueue::GetSingleton())
		UIMessageQueue->AddMessage(MENU_NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr);
}

void DebugMenu::CloseMenu()
{
	if (auto UIMessageQueue = RE::UIMessageQueue::GetSingleton())
		UIMessageQueue->AddMessage(MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
}

void DebugMenu::ShowMenu()
{
	RE::GFxValue root;
	if (movie)
	{
		movie->GetVariable(&root, "_root");
	}
	if (!root.IsObject())
	{
		logger::info("Root not obtained");
		return;
	}
	
	SetText("navMeshRangeText", MCM::settings::navmeshRange);
	SetText("occlusionRangeText", MCM::settings::occlusionRange);	
}

DebugMenu::MenuItems DebugMenu::GetMenuItems()
{
	return menuItems;
}

void DebugMenu::GetIcon(const char* a_iconName, RE::GFxValue& a_icon)
{
	RE::GFxValue root;
	GetRoot(root);
	RE::GFxValue menuContainer;
	root.GetMember("menuContainer", &menuContainer);
	menuContainer.GetMember(a_iconName, &a_icon);
}

void DebugMenu::SetText(const char* a_textBoxName, float a_range)
{
	RE::GFxValue root;
	GetRoot(root);

	RE::GFxValue menuContainer;
	root.GetMember("menuContainer", &menuContainer);

	RE::GFxValue textBox;
	menuContainer.GetMember(a_textBoxName, &textBox);

	// round range
	uint32_t rangeInt = static_cast<int>(a_range);
	const std::string text = std::to_string(rangeInt);

	RE::GFxValue message{ text };
	textBox.Invoke("SetText", nullptr, &message, 1);
}

void DebugMenu::ButtonMethod(const char* a_buttonName, const char* a_methodName)
{
	RE::GFxValue buttonIcon;
	GetIcon(a_buttonName, buttonIcon);
	
	RE::GFxValue::DisplayInfo info;
	buttonIcon.GetDisplayInfo(&info);
	auto result = buttonIcon.Invoke(a_methodName, nullptr, nullptr, 0);
	//logger::info("Invoking method {} for button {}. Result: {}", a_methodName, a_buttonName, result);

}

void DebugMenu::GetRoot(RE::GFxValue& root)
{
	if (movie)
	{
		movie->GetVariable(&root, "_root");
	}
	else logger::info("Failed to get root");
}