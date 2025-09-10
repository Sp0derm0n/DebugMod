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
	if (!success) logger::debug("LoadMovieEx unsuccessful");

	menuFlags.set(
		RE::UI_MENU_FLAGS::kAlwaysOpen,
		RE::UI_MENU_FLAGS::kRequiresUpdate,
		RE::UI_MENU_FLAGS::kAllowSaving,
		RE::UI_MENU_FLAGS::kUsesCursor);

	menu->inputContext = Context::kNone;
	if (uiMovie) movie = menu->uiMovie;
	else logger::debug("uiMovie not obtained");

	Init();
}

DebugMenu::Button::Button(float a_xMin, float a_xMax, float a_yMin, float a_yMax, std::string a_name,
                                  BUTTON_TYPE a_buttonType, MenuPtr a_parentMenu, bool a_isInMask,
                                  ButtonActionFunction a_buttonAction) :
	xMin(a_xMin),
	xMax(a_xMax),
	yMin(a_yMin),
	yMax(a_yMax),
	name(a_name),
	type(a_buttonType),
    parentMenu(a_parentMenu),
	isInMask(a_isInMask),
	ButtonAction_(a_buttonAction)
{}

void DebugMenu::Button::AddChild(ButtonPtr& a_child)
{
	if (MCM::settings::logUI) logger::debug("   Adding child: {} to parent: {}", a_child->name, this->name);

	children.push_back(a_child);
	a_child->parent = this;
}

void DebugMenu::Button::SetChildFlag(ChildFlags a_flag)
{
	if (HasChildFlag(a_flag)) return;

	uint32_t value = static_cast<uint32_t>(childFlags);
	uint32_t flag  = static_cast<uint32_t>(a_flag);
	value += flag;
	childFlags = static_cast<ChildFlags>(value);
}

bool DebugMenu::Button::HasChildFlag(ChildFlags a_flag)
{
	uint32_t value = static_cast<uint32_t>(childFlags);
	uint32_t flag  = static_cast<uint32_t>(a_flag);
	return value & flag;
}

void DebugMenu::Button::ButtonAction()
{
	if (ButtonAction_)
	{
		ButtonAction_(this);
	}
}

DebugMenu::ButtonPtr DebugMenu::MenuItems::GetButtonByName(const std::string& a_name)
{
	for (auto& button : buttons)
	{
		if (button->name == a_name)
		{
			return button;
		}
	}
	if (MCM::settings::logUI) logger::debug("Could not find button \"{}\"", a_name);
	return nullptr;
}

DebugMenu::TextBoxPtr DebugMenu::MenuItems::GetTextBoxByName(const std::string& a_name)
{
	for (auto& textBox : textBoxes)
	{
		if (textBox->name == a_name)
		{
			return textBox;
		}
	}
	if (MCM::settings::logUI) logger::debug("Could not find textbox \"{}\"", a_name);
	return nullptr;
}

DebugMenu::MenuPtr DebugMenu::MenuItems::GetMenuByName(const std::string& a_name)
{
	for (auto& menu : menus)
	{
		if (menu->name == a_name)
		{
			return menu;
		}
	}
	if (MCM::settings::logUI) logger::debug("Could not find textbox \"{}\"", a_name);
	return nullptr;
}


// To add a new item, make sure to give it a script to register it to a class.
// Remember to also give the symbol a linkage name.
// Then, give the symbol an instance name when added to the MenuContainer symbol.
void DebugMenu::Init()
{
	if (logUI) logger::debug("Registering UI elements");

	InitButtonActionFunctions();

	//////////////////////////////////////////////////////////////////////////////////////////////////
	// Register menus ////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////

	//										   name, layer
	auto mainMenu				= RegisterMenu("mainMenu", 0);
	auto markersSelectionMenu	= RegisterMenu("markersSelectionMenu", 1);
	auto savePresetsMenu = RegisterMenu("SavePresetsMenu", 2);
	auto loadPresetsMenu = RegisterMenu("LoadPresetsMenu", 3, mainMenu);

	HideMenu(markersSelectionMenu->GetName());
	HideMenu(savePresetsMenu->GetName());
	HideMenu(loadPresetsMenu->GetName());

	if (logUI) logger::debug("Registered menus");

	//////////////////////////////////////////////////////////////////////////////////////////////////
	// Register text boxes ///////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////

	auto navMeshRangeText	= RegisterTextBox("navMeshRangeText",	mainMenu);
	auto occlusionRangeText	= RegisterTextBox("occlusionRangeText", mainMenu);
	auto markersRangeText	= RegisterTextBox("markersRangeText",	mainMenu);

	if (logUI) logger::debug("Registered text boxes");
	
	//////////////////////////////////////////////////////////////////////////////////////////////////
	// Register buttons //////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////


	// Frame buttons first
	auto dayNightMode	= RegisterButton("dayNightIcon",		BUTTON_TYPE::kCyclic, mainMenu, DayNightAction);


	// Then container buttons
	auto cellBorder		= RegisterButton("cellBorderIcon",		BUTTON_TYPE::kToggle, mainMenu, CellBorderAction);
	auto cellWalls		= RegisterButton("wallsIcon",			BUTTON_TYPE::kToggle, mainMenu, CellWallsAction);
	auto cellQuads		= RegisterButton("quadsIcon",			BUTTON_TYPE::kToggle, mainMenu, CellQuadsAction);
	cellBorder->AddChild(cellWalls);
	cellBorder->AddChild(cellQuads);


	auto navMesh		= RegisterButton("navMeshIcon",			BUTTON_TYPE::kToggle, mainMenu, NavMeshAction);
	auto navMeshPlus	= RegisterButton("navMeshPlusIcon",		BUTTON_TYPE::kClick,  mainMenu, NavMeshPlusAction);
	auto navMeshMinus	= RegisterButton("navMeshMinusIcon",	BUTTON_TYPE::kClick,  mainMenu, NavMeshMinusAction);
	auto navMeshMode	= RegisterButton("navMeshModeIcon",		BUTTON_TYPE::kToggle, mainMenu, NavMeshModeAction);
	auto triangles		= RegisterButton("trianglesIcon",		BUTTON_TYPE::kToggle, mainMenu, NavMeshTrianglesAction);
	auto cover			= RegisterButton("coverIcon",			BUTTON_TYPE::kToggle, mainMenu, NavMeshCoverAction);
	navMesh->AddChild(triangles);
	navMesh->AddChild(cover);
	
	auto occlusion		= RegisterButton("occlusionIcon",		BUTTON_TYPE::kToggle, mainMenu, OcclusionAction);
	auto occlusionPlus	= RegisterButton("occlusionPlusIcon",	BUTTON_TYPE::kClick,  mainMenu, OcclusionPlusAction);
	auto occlusionMinus	= RegisterButton("occlusionMinusIcon",	BUTTON_TYPE::kClick,  mainMenu, OcclusionMinusAction);
	auto coordinates	= RegisterButton("coordinatesIcon",		BUTTON_TYPE::kToggle, mainMenu, CoordinatesAction);
	auto markers		= RegisterButton("markersIcon",			BUTTON_TYPE::kToggle, mainMenu, MarkersAction);
	auto markersPlus	= RegisterButton("markersPlusIcon",		BUTTON_TYPE::kClick,  mainMenu, MarkersPlusAction);
	auto markersMinus	= RegisterButton("markersMinusIcon",	BUTTON_TYPE::kClick,  mainMenu, MarkersMinusAction);
	auto selectMarkers	= RegisterButton("selectMarkersIcon",	BUTTON_TYPE::kClick,  mainMenu, SelectMarkersAction);
	auto loadPreset1	= RegisterButton("preset1Icon",			BUTTON_TYPE::kClick,  mainMenu, LoadPreset1MarkersAction);
	auto loadPreset2	= RegisterButton("preset2Icon",			BUTTON_TYPE::kClick,  mainMenu, LoadPreset2MarkersAction);
	auto loadPreset3	= RegisterButton("preset3Icon",			BUTTON_TYPE::kClick,  mainMenu, LoadPreset3MarkersAction);
	
	// Load Presets Menu frame Items

	auto loadMarkerYes	= RegisterButton("yesIcon",				BUTTON_TYPE::kClick,  loadPresetsMenu, LoadPresetAction);
	auto loadMarkerNo	= RegisterButton("noIcon",				BUTTON_TYPE::kClick,  loadPresetsMenu, CancelLoadPresetAction);

	// Markers Selection Menu frame items

	auto savePreset1	= RegisterButton("preset1Icon",			BUTTON_TYPE::kClick, markersSelectionMenu, SavePreset1MarkersAction);
	auto savePreset2	= RegisterButton("preset2Icon",			BUTTON_TYPE::kClick, markersSelectionMenu, SavePreset2MarkersAction);
	auto savePreset3	= RegisterButton("preset3Icon",			BUTTON_TYPE::kClick, markersSelectionMenu, SavePreset3MarkersAction);


	// Save Presets Menu frame items

	auto saveMarkerYes	= RegisterButton("yesIcon",				BUTTON_TYPE::kClick, savePresetsMenu, SavePresetAction);
	auto SaveMarkerNo	= RegisterButton("noIcon",				BUTTON_TYPE::kClick, savePresetsMenu, CancelSavePresetAction);



	if (logUI) logger::debug("\nRegistered buttons");

	//std::vector<std::pair<std::string, std::vector<std::string>>> selectableMarkers; // [(Headline1, [sub1, sub2, sub3, ... ]), (Headline2, [sub1, sub2, sub3, ... ]), ... ]

	InitSelectableMarkerButtons();
	OrderMenuItems();

	if (logUI) logger::debug("...Finished UI Initialization");

}

void DebugMenu::OrderMenuItems()
{
	if (logUI) logger::debug("Ordering menu items...");
	auto compareMenus = [](const MenuPtr& a_menu1, const MenuPtr& a_menu2) { return a_menu1->layer > a_menu2->layer; };

	std::sort(menuItems.menus.begin(), menuItems.menus.end(), compareMenus);

	if (logUI)
	{
		logger::debug(" menus ordered:");
		for (const auto& menu : menuItems.menus)
		{
			logger::debug("  {:>2}: {}", menu->layer, menu->GetName());
		}
		logger::debug("...Finished ordiering menu items");
	}
}

void DebugMenu::InitSelectableMarkerButtons()
{
	if (logUI) logger::debug("Creating 'select markers' buttons...");

	// The names are used as the linkage names
	AddMarkerLabel("Furniture",			true,	&MCM::settings::showFurnitureMarkers);
	AddMarkerLabel( "Sit",				false,	&MCM::settings::showSitMarkers);
	AddMarkerLabel( "Lean",				false,	&MCM::settings::showLeanMarkers);
	AddMarkerLabel( "Sleep",			false,	&MCM::settings::showSleepMarkers);

	AddMarkerLabel("Misc",				true,	&MCM::settings::showMiscMarkers);
	AddMarkerLabel( "Light bulbs",		false,	&MCM::settings::showLightMarkers);
	AddMarkerLabel( "Idle markers",		false,  &MCM::settings::showIdleMarkers);
	AddMarkerLabel( "Sound markers",	false,	&MCM::settings::showSoundMarkers);
	AddMarkerLabel( "Dragon markers",	false,	&MCM::settings::showDragonMarkers);
	AddMarkerLabel( "Cloud markers",	false,	&MCM::settings::showCloudMarkers);
	AddMarkerLabel( "Critter markers",	false,	&MCM::settings::showCritterMarkers);
	AddMarkerLabel( "Flora markers",	false,	&MCM::settings::showFloraMarkers);
	AddMarkerLabel( "Hazard markers",	false,	&MCM::settings::showHazardMarkers);
	//AddMarkerLabel( "Texture Set",		false,	&MCM::settings::showTextureSetMarkers);

	AddMarkerLabel("Movable statics",	true,	&MCM::settings::showMovableStaticMarkers);
	AddMarkerLabel( "Mist",				false,	&MCM::settings::showMistMarkers);
	AddMarkerLabel( "Light beams",		false,	&MCM::settings::showLightBeamMarkers);
	AddMarkerLabel( "Other MSTT",		false,	&MCM::settings::showOtherMSTTMarkers);

	AddMarkerLabel("Statics",			true,	&MCM::settings::showStaticMarkers);
	AddMarkerLabel( "X marker",			false,	&MCM::settings::showXMarkers);
	AddMarkerLabel( "Heading",			false,	&MCM::settings::showHeadingMarkers);
	AddMarkerLabel( "Door teleport",	false,	&MCM::settings::showDoorTeleportMarkers);
	AddMarkerLabel( "Other statics",	false,  &MCM::settings::showOtherStaticMarkers);

	AddMarkerLabel("Activators",		true,	&MCM::settings::showActivatorMarkers);
	AddMarkerLabel( "Impact markers",	false,	&MCM::settings::showImpactMarkers);
	AddMarkerLabel( "Other activators",	false,	&MCM::settings::showOtherActivatorMarkers);

	AddMarkerLabel("Civil War",			true,	&MCM::settings::showCWMarkers);
	AddMarkerLabel( "Attacker",			false,	&MCM::settings::showCWAttackerMarkers);
	AddMarkerLabel( "Defender",			false,	&MCM::settings::showCWDefenderMarkers);
	AddMarkerLabel( "Other CW",			false,	&MCM::settings::showOtherCWMarkers);




	ButtonPtr headlineButtonPtr;

	// UI Elements
	bool res = false;

	MenuPtr markersSelectionMenu = menuItems.GetMenuByName("markersSelectionMenu");

	RE::GFxValue root;
	GetRoot(root);

	RE::GFxValue menu;
	root.GetMember(markersSelectionMenu->GetName(), &menu);

	RE::GFxValue maskItems;
	menu.GetMember("maskItems", &maskItems);

	RE::GFxValue headlineButton;
	RE::GFxValue headlineTextBox;
	RE::GFxValue notHeadlineButton;
	RE::GFxValue notHeadlineTextBox;

	maskItems.GetMember("headlineButton", &headlineButton);
	maskItems.GetMember("headlineTextBox", &headlineTextBox);
	maskItems.GetMember("notHeadlineButton", &notHeadlineButton);
	maskItems.GetMember("notHeadlineTextBox", &notHeadlineTextBox);

	RE::GFxValue::DisplayInfo headlineButtonInfo;
	RE::GFxValue::DisplayInfo headlineTextBoxInfo;
	RE::GFxValue::DisplayInfo notHeadlineButtonInfo;
	RE::GFxValue::DisplayInfo notHeadlineTextBoxInfo;

	headlineButton.GetDisplayInfo(&headlineButtonInfo);
	headlineTextBox.GetDisplayInfo(&headlineTextBoxInfo);
	notHeadlineButton.GetDisplayInfo(&notHeadlineButtonInfo);
	notHeadlineTextBox.GetDisplayInfo(&notHeadlineTextBoxInfo);

	float headlineButtonX = static_cast<float>(headlineButtonInfo.GetX());
	float headlineButtonY = static_cast<float>(headlineButtonInfo.GetY());
	float notHeadlineButtonX = static_cast<float>(notHeadlineButtonInfo.GetX());
	float notHeadlineButtonY = static_cast<float>(notHeadlineButtonInfo.GetY());

	float headlineTextBoxX = static_cast<float>(headlineTextBoxInfo.GetX());
	float headlineTextBoxY = static_cast<float>(headlineTextBoxInfo.GetY());
	float notHeadlineTextBoxX = static_cast<float>(notHeadlineTextBoxInfo.GetX());
	float notHeadlineTextBoxY = static_cast<float>(notHeadlineTextBoxInfo.GetY());

	float indentSize = (notHeadlineTextBoxX - headlineTextBoxX) * MCM::settings::uiScale;
	float verticalSpacing = (notHeadlineTextBoxY - headlineTextBoxY) * MCM::settings::uiScale;
	float headlineOffset = 1.5 * verticalSpacing;

	float buttonY = headlineButtonY;
	float textBoxY = headlineTextBoxY;

	if (logUI) logger::debug(" Setup complete");

	for (int i = 0; i < selectableMarkers.size(); i++)
	{
		auto& markerLabel = selectableMarkers[i];

		// Create the physical button

		if (logUI) logger::debug(" Creating physical button '{}'", markerLabel.name);


		// Copy original button
		RE::GFxValue newButton;
		RE::GFxValue newTextBox;

		// get these from flash
		std::string buttonLinkageName = "ButtonIconWide";
		std::string headlineTextBoxLinkageName = "TextBoxSkyrimBooks";
		std::string notHeadlineTextBoxLinkageName = "TextBoxFutura";

		std::string newButtonName = markerLabel.name + "Button";
		std::string newTextBoxName = markerLabel.name + "TextBox";

		RE::GFxValue newButtonParameters[2]{ buttonLinkageName.c_str(), newButtonName.c_str() };
		RE::GFxValue newTextBoxParameters[2]{ headlineTextBoxLinkageName.c_str(), newTextBoxName.c_str() };


		float buttonX = 0.0f;
		float textBoxX = 0.0f;

		if (markerLabel.isHeadline)
		{
			buttonX = headlineButtonX;
			textBoxX = headlineTextBoxX;

			if (i > 0)
			{
				buttonY += headlineOffset;
				textBoxY += headlineOffset;
			}

		}
		else
		{
			newTextBoxParameters[0] = notHeadlineTextBoxLinkageName.c_str();

			buttonX = notHeadlineButtonX;
			textBoxX = notHeadlineTextBoxX;

			buttonY += verticalSpacing;
			textBoxY += verticalSpacing;
		}

		maskItems.Invoke("CreateItem", nullptr, newButtonParameters, 2);
		maskItems.Invoke("CreateItem", nullptr, newTextBoxParameters, 2);

		maskItems.GetMember(newButtonName.c_str(), &newButton);
		maskItems.GetMember(newTextBoxName.c_str(), &newTextBox);



		RE::GFxValue newButtonPosition[2]{ buttonX, buttonY };
		RE::GFxValue newTextBoxPosition[2]{ textBoxX, textBoxY };

		newButton.Invoke("SetPosition", nullptr, newButtonPosition, 2);
		newTextBox.Invoke("SetPosition", nullptr, newTextBoxPosition, 2);



		RE::GFxValue textBoxText{ markerLabel.name };
		newTextBox.Invoke("SetText", nullptr, &textBoxText, 1);

		RE::GFxValue textBoxWidth{ 125 };
		newTextBox.Invoke("SetWidth", nullptr, &textBoxWidth, 1);

		RE::GFxValue textBoxAlign{ "left" };
		newTextBox.Invoke("SetTextAlignment", nullptr, &textBoxAlign, 1);

		RE::GFxValue fontSize{ 16 * MCM::settings::uiScale };
		newTextBox.Invoke("SetFontSize", nullptr, &fontSize, 1);

		RE::GFxValue scale{ MCM::settings::uiScale };
		newButton.Invoke("SetScale", nullptr, &scale, 1);

		uint32_t childTextcolor_ = 0xA1A1A1;

		// initialize non headline text boxes as grey if they are disabled or if their headline is disabled
		if (!markerLabel.isHeadline)
		{
			//if (headlineButtonPtr)
			//	logger::info("headline {} active? {}", headlineButtonPtr->name, headlineButtonPtr->isActive);
			if (!headlineButtonPtr || !headlineButtonPtr->isActive || !*markerLabel.setting)
			{
				RE::GFxValue childTextColor{ childTextcolor_ };
				newTextBox.Invoke("SetTextColor", nullptr, &childTextColor, 1);
			}
		}

		headlineButton.Invoke("Hide", nullptr, nullptr, 0);
		headlineTextBox.Invoke("Hide", nullptr, nullptr, 0);
		notHeadlineButton.Invoke("Hide", nullptr, nullptr, 0);
		notHeadlineTextBox.Invoke("Hide", nullptr, nullptr, 0);

		// Create the logical button
		if (logUI) logger::debug(" Creating logical button '{}'", markerLabel.name);



		RegisterTextBox(newTextBoxName.c_str(), markersSelectionMenu);

		if (markerLabel.isHeadline)
		{
			ButtonActionFunction buttonFunction = [=](Button* a_button)
			{
				*markerLabel.setting = a_button->isActive;

				if (!a_button->isActive)
					MCM::settings::updateVisibleMarkers = true; // hide all markers when disabling a button, such that they arent drawn during the next cycle

				for (const auto& child : a_button->children)
				{
					std::string markerLabelName = child->name.substr(0, child->name.size() - 6); // remove "Button" from the name, as it is <marker label name> + "Button"
					std::string childTextBoxName = markerLabelName + "TextBox";

					auto childTextBox = menuItems.GetTextBoxByName(childTextBoxName.c_str());

					RE::GFxValue childText;
					GetIcon(childTextBox, childText);

					RE::GFxValue childTextColor{ childTextcolor_ };
					if (a_button->isActive && child->isActive) childTextColor = 0xFFFFFF;

					childText.Invoke("SetTextColor", nullptr, &childTextColor, 1);

				}
			};

			auto newButtonPtr = RegisterButton(newButtonName.c_str(), BUTTON_TYPE::kToggle, markersSelectionMenu, buttonFunction);
			markerLabel.button = newButtonPtr;
			headlineButtonPtr = newButtonPtr;
		}
		else
		{
			ButtonActionFunction buttonFunction = [=](Button* a_button)
			{
				if (!a_button->isActive)
					MCM::settings::updateVisibleMarkers = true;

				*markerLabel.setting = a_button->isActive;

				std::string markerLabelName = a_button->name.substr(0, a_button->name.size() - 6); // remove "Button" from the name, as it is <marker label name> + "Button"
				std::string textBoxName = markerLabelName + "TextBox";

				auto textBox = menuItems.GetTextBoxByName(textBoxName.c_str());

				RE::GFxValue childText;
				GetIcon(textBox, childText);

				RE::GFxValue textColor{ childTextcolor_ };
				if (a_button->isActive) textColor = 0xFFFFFF;

				childText.Invoke("SetTextColor", nullptr, &textColor, 1);
			};

			auto newButtonPtr = RegisterButton(newButtonName.c_str(), BUTTON_TYPE::kToggle, markersSelectionMenu, buttonFunction);
			markerLabel.button = newButtonPtr;
			newButtonPtr->SetChildFlag(ChildFlags::kInteractable);
			headlineButtonPtr->AddChild(newButtonPtr);
		}
		markerLabel.button->isActive = *markerLabel.setting;
	}



	// update the height of the maskItems of the menu
	RE::GFxValue::DisplayInfo maskItemsInfo;
	maskItems.GetDisplayInfo(&maskItemsInfo);

	RE::GFxValue itemsHeight;
	maskItems.Invoke("GetHeight", &itemsHeight, nullptr, 0);
	markersSelectionMenu->maskItemsHeight = static_cast<float>(itemsHeight.GetNumber());

	if (logUI) logger::debug("...Finished creating 'select markers' buttons");
}

void DebugMenu::AddMarkerLabel(const std::string& a_labelName, bool a_isHeadline, bool* setting)
{
	selectableMarkers.push_back(MarkerLabel(a_labelName, a_isHeadline, setting));
}

void DebugMenu::UpdateMarkersSelectionButtons()
{
	// Sets the button states to the correct value
	// Sets the color of the non-headline buttons correctly. The headline buttons must be called after the non-headline buttons have their isActive status set, therefore reverse loop
	for (int i = selectableMarkers.size() - 1; i > -1; i--)
	{
		const auto& markerLabel = selectableMarkers[i];
		markerLabel.button->isActive = *markerLabel.setting;
		markerLabel.button->ButtonAction();

	}
}

// How to make a new menu:
// -If it has a mask, call it menuMask
// all items in the mask should be in a symbol called maskitems
// all other interactable items should be in frameItems
DebugMenu::MenuPtr DebugMenu::RegisterMenu(const char* a_menuName, uint32_t a_layer, MenuPtr a_parentMenu)
{
	if (logUI) logger::debug("   Registering menu: {}...", a_menuName);

	RE::GFxValue root;
	GetRoot(root);

	// Get x and y of menu
	RE::GFxValue menu;
	root.GetMember(a_menuName, &menu);

	RE::GFxValue::DisplayInfo menuInfo;
	menu.GetDisplayInfo(&menuInfo);

	float menuX = static_cast<float>(menuInfo.GetX());
	float menuY = static_cast<float>(menuInfo.GetY());

	RE::GFxValue menuWidth;
	RE::GFxValue menuHeight;

	menu.Invoke("GetWidth", &menuWidth, nullptr, 0);
	menu.Invoke("GetHeight", &menuHeight, nullptr, 0);

	float width = static_cast<float>(menuWidth.GetNumber());
	float height = static_cast<float>(menuHeight.GetNumber());

	float xMin = menuX - width / 2;
	float yMin = menuY - height / 2;

	float xMax = xMin + width;
	float yMax = yMin + height;

	// Get the bounds of the mask

	float maskXMin = 0.0f;
	float maskXMax = 0.0f;
	float maskYMin = 0.0f;
	float maskYMax = 0.0f;
	float maskHeight = 0.0f;

	float maskItemsDefaultY = 0.0f;
	float maskItemsCurrentY = 0.0f;
	float maskItemsYLocalOffset = 0.0f;
	float maskItemsHeight = 0.0f;

	RE::GFxValue mask;
	bool hasMask = menu.GetMember("menuMask", &mask);

	if (logUI) logger::debug("   Menu has mask? {}", hasMask);

	if (hasMask)
	{
		RE::GFxValue::DisplayInfo maskInfo;
		mask.GetDisplayInfo(&maskInfo);

		float maskX = static_cast<float>(maskInfo.GetX());
		float maskY = static_cast<float>(maskInfo.GetY());

		RE::GFxValue containerWidth;
		RE::GFxValue containerHeight;

		mask.Invoke("GetWidth", &containerWidth, nullptr, 0);
		mask.Invoke("GetHeight", &containerHeight, nullptr, 0);

		float maskWidth = static_cast<float>(containerWidth.GetNumber());
		maskHeight = static_cast<float>(containerHeight.GetNumber());

		maskXMin = menuX + maskX - maskWidth/2;
		maskYMin = menuY + maskY - maskHeight/2;

		maskXMax = maskXMin + maskWidth;
		maskYMax = maskYMin + maskHeight;
	}
	// Get coordinates of all the icons in the mask container, and their combined height - to use with the scroll ability
	bool hasMaskItems = menu.HasMember("maskItems");

	if (logUI) logger::debug("   Menu has mask items? {}", hasMaskItems);

	if (hasMaskItems)
	{
		RE::GFxValue maskItems;
		menu.GetMember("maskItems", &maskItems);

		RE::GFxValue::DisplayInfo maskItemsInfo;
		maskItems.GetDisplayInfo(&maskItemsInfo);

		RE::GFxValue itemsHeight;
		maskItems.Invoke("GetHeight", &itemsHeight, nullptr, 0);
		maskItemsHeight = static_cast<float>(itemsHeight.GetNumber());

		maskItemsDefaultY = static_cast<float>(maskItemsInfo.GetY()); // local y value of menuItems
		maskItemsCurrentY = maskItemsDefaultY;

		maskItemsYLocalOffset = menuY - maskItemsHeight / 2;
	}

	if (logUI) logger::debug("   Menu has frame items? {}", menu.HasMember("frameItems"));


	//logger::info("{} {} {} {} {} {} {} {}", menuX, menuY, hasMask, maskXMin, maskXMax, maskYMin, maskYMax, a_menuName);
	if (logUI) logger::debug("  ...Registered menu: {}", a_menuName);

	auto newMenu = std::make_shared<Menu>(a_layer, menuX, menuY, xMin, xMax, yMin, yMax, hasMask, maskXMin, maskXMax, maskYMin, maskYMax, maskHeight, maskItemsDefaultY, maskItemsCurrentY, maskItemsYLocalOffset, maskItemsHeight, a_menuName);
	menuItems.menus.push_back(newMenu);

	if (a_parentMenu)
	{
		a_parentMenu->menus.push_back(newMenu);
	}
    return newMenu;
}

DebugMenu::ButtonPtr DebugMenu::RegisterButton(const char* a_buttonName, BUTTON_TYPE a_buttonType, MenuPtr a_parentMenu, ButtonActionFunction a_buttonAction)
{
	RE::GFxValue root;
	GetRoot(root);

	// Get UI elements
	RE::GFxValue menu;

	root.GetMember(a_parentMenu->GetName(), &menu);

	// get info on UI elements
	RE::GFxValue::DisplayInfo menuInfo;

	menu.GetDisplayInfo(&menuInfo);

	float container_x = static_cast<float>(menuInfo.GetX());
	float container_y = static_cast<float>(menuInfo.GetY());

	// Figure out if button is in frame or in scrollable mask

	bool hasMaskItems = menu.HasMember("maskItems");
	bool hasFrameItems = menu.HasMember("frameItems");
	bool isInMask = false;
	bool doesItemExist = false;

	if (hasMaskItems)
	{
		RE::GFxValue items;
		menu.GetMember("maskItems", &items);
		isInMask = items.HasMember(a_buttonName);
		doesItemExist = isInMask;
	}
	if (hasFrameItems)
	{
		RE::GFxValue items;
		menu.GetMember("frameItems", &items);
		doesItemExist = doesItemExist || items.HasMember(a_buttonName);
	}
	
	if (!doesItemExist)
	{
		if (MCM::settings::logUI) logger::debug("Item '{}' could not be found in menu '{}'; menu has maskItems? {}; menu has frameItems? {}", a_buttonName, a_parentMenu->name, hasMaskItems, hasFrameItems);
		return nullptr;
	}

	// get UI element and the items-container it is in
	RE::GFxValue itemsContainer;

	if (isInMask) 
		menu.GetMember("maskItems", &itemsContainer);
	else
		menu.GetMember("frameItems", &itemsContainer);

	RE::GFxValue item;
	itemsContainer.GetMember(a_buttonName, &item);

	// get info on UI elements
	RE::GFxValue::DisplayInfo itemsContainerInfo;
	itemsContainer.GetDisplayInfo(&itemsContainerInfo);

	float itemsContainer_x = static_cast<float>(itemsContainerInfo.GetX());
	float itemsContainer_y = static_cast<float>(itemsContainerInfo.GetY());

	RE::GFxValue::DisplayInfo itemInfo;
	item.GetDisplayInfo(&itemInfo);

	RE::GFxValue itemWidth;
	RE::GFxValue itemHeight;

	item.Invoke("GetWidth", &itemWidth, nullptr, 0);
	item.Invoke("GetHeight", &itemHeight, nullptr, 0);

	float width = static_cast<float>(itemWidth.GetNumber());
	float height = static_cast<float>(itemHeight.GetNumber());

	// symbols should be centered in their own frame
	float item_xmin = static_cast<float>(itemInfo.GetX()) + itemsContainer_x + container_x - width/2;
	float item_ymin = static_cast<float>(itemInfo.GetY()) + itemsContainer_y + container_y - height/2;

	float item_xmax = item_xmin + width;
	float item_ymax = item_ymin + height;

	bool isActive = false;
	BUTTON_STATE buttonState = isActive ?  BUTTON_STATE::kOFF :  BUTTON_STATE::kON;

	if (logUI) logger::debug("  Registered button: {}", a_buttonName);

	auto newButton = std::make_shared<Button>(item_xmin, item_xmax, item_ymin, item_ymax, a_buttonName, a_buttonType, a_parentMenu, isInMask, a_buttonAction);
	menuItems.buttons.push_back(newButton);

	if (a_parentMenu)
	{
		a_parentMenu->buttons.push_back(newButton);
	}

	return newButton;
}

DebugMenu::TextBoxPtr DebugMenu::RegisterTextBox(const char* a_textBoxName, MenuPtr a_parentMenu)
{
	RE::GFxValue root;
	GetRoot(root);

	// Get UI elements
	RE::GFxValue menu;

	root.GetMember(a_parentMenu->GetName(), &menu);

	// Figure out if button is in frame or in scrollable mask
	RE::GFxValue items;
	menu.GetMember("maskItems", &items);
	bool isInMask = items.HasMember(a_textBoxName);

	if (logUI) logger::debug("  Registered text box: {}", a_textBoxName);

	auto newTextBox = std::make_shared<TextBox>(a_textBoxName, isInMask, a_parentMenu);
	menuItems.textBoxes.push_back(newTextBox);

	if (a_parentMenu)
	{
		a_parentMenu->textBoxes.push_back(newTextBox);
	}

	return newTextBox;
}


void DebugMenu::Register()
{
	auto ui = RE::UI::GetSingleton();
	if (ui) 
	{
		ui->Register(MENU_NAME, Creator);
		logger::debug("Registered menu '{}'", MENU_NAME);
	}
}

RE::UI_MESSAGE_RESULTS DebugMenu::ProcessMessage(RE::UIMessage& a_message)
{
	if (a_message.type == RE::UI_MESSAGE_TYPE::kShow)
	{
		UIHandler::GetSingleton()->isMenuOpen = true;	
		if (auto controlMap = RE::ControlMap::GetSingleton())
		{
			controlMap->ToggleControls(RE::UserEvents::USER_EVENT_FLAG::kWheelZoom, false);
		}
	}

	else if (a_message.type == RE::UI_MESSAGE_TYPE::kHide)
	{
		UIHandler::GetSingleton()->isMenuOpen = false;	
		if (auto controlMap = RE::ControlMap::GetSingleton())
		{
			controlMap->ToggleControls(RE::UserEvents::USER_EVENT_FLAG::kWheelZoom, true);
			MCM::DebugMenuPresets::SaveMarkerSettings(0);
		}
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

void DebugMenu::ShowMenu(const char* a_menuName)
{
	if (logUI) logger::debug("show: {}", a_menuName);
	RE::GFxValue root;
	GetRoot(root);

	// Get UI elements
	RE::GFxValue menu;

	root.GetMember(a_menuName, &menu);

	menu.Invoke("Show", nullptr, nullptr, 0);

	auto menu_ = menuItems.GetMenuByName(a_menuName);
	if (menu_) menu_->isActive = true;

}

void DebugMenu::HideMenu(const char* a_menuName)
{
	if (logUI) logger::debug("hide: {}", a_menuName);
	RE::GFxValue root;
	GetRoot(root);

	// Get UI elements
	RE::GFxValue menu;

	root.GetMember(a_menuName, &menu);

	menu.Invoke("Hide", nullptr, nullptr, 0);

	auto menu_ = menuItems.GetMenuByName(a_menuName);
	if (menu_) menu_->isActive = false;
}

DebugMenu::MenuItems* DebugMenu::GetMenuItems()
{
	return &menuItems;
}

void DebugMenu::GetIcon_(const char* a_iconName, const char* a_parentMenuName, bool a_isInMask, RE::GFxValue& a_icon)
{
	if (logUI) logger::debug("  Getting Icon: {}; Belonging to: {} (in mask: {})", a_iconName, a_parentMenuName, a_isInMask);

	RE::GFxValue root;
	GetRoot(root);

	RE::GFxValue menu;
	root.GetMember(a_parentMenuName, &menu);

	RE::GFxValue icons;
	if (a_isInMask)
		menu.GetMember("maskItems", &icons);
	else
		menu.GetMember("frameItems", &icons);

	icons.GetMember(a_iconName, &a_icon);
}

void DebugMenu::GetIcon(const ButtonPtr& a_button, RE::GFxValue& a_icon)
{
	GetIcon_(a_button->GetName(), a_button->parentMenu->GetName(), a_button->isInMask, a_icon);
}

void DebugMenu::GetIcon(const Button* a_button, RE::GFxValue& a_icon)
{
	GetIcon_(a_button->GetName(), a_button->parentMenu->GetName(), a_button->isInMask, a_icon);
}

void DebugMenu::GetIcon(const TextBoxPtr& a_textBox, RE::GFxValue& a_icon)
{
	GetIcon_(a_textBox->GetName(), a_textBox->parentMenu->GetName(), a_textBox->isInMask, a_icon);
}

void DebugMenu::GetMenu(const MenuPtr& a_menu, RE::GFxValue& a_menuOut)
{
	RE::GFxValue root;
	GetRoot(root);

	root.GetMember(a_menu->GetName(), &a_menuOut);
}

void DebugMenu::SetText(const TextBoxPtr& a_textBox, const std::string& a_text)
{
	RE::GFxValue textBox;
	GetIcon(a_textBox, textBox);

	RE::GFxValue message{ a_text };
	textBox.Invoke("SetText", nullptr, &message, 1);

	if (logUI) logger::debug("Setting Text(\"{}\") in: {}", a_text, a_textBox->name);
}

void DebugMenu::SetText(const TextBoxPtr& a_textBox, float a_range)
{
	// round range
	uint32_t rangeInt = static_cast<int>(a_range);
	const std::string text = std::to_string(rangeInt);

	SetText(a_textBox, text);
}

void DebugMenu::SetText(const char* a_textBoxName, const std::string& a_text)
{
	auto textBox = menuItems.GetTextBoxByName(a_textBoxName);
	SetText(textBox, a_text);
}
void DebugMenu::SetText(const char* a_textBoxName, float a_range)
{
	auto textBox = menuItems.GetTextBoxByName(a_textBoxName);
	SetText(textBox, a_range);
}

bool DebugMenu::ButtonActionScriptMethod(const Button* a_button, const char* a_methodName)
{
	if (!a_button) return false;

	RE::GFxValue buttonIcon;
	GetIcon(a_button, buttonIcon);

	RE::GFxValue::DisplayInfo info;
	buttonIcon.GetDisplayInfo(&info);
	auto result = buttonIcon.Invoke(a_methodName, nullptr, nullptr, 0);
	return result;
	//logger::info("Invoking method {} for button {}. Result: {}", a_methodName, a_buttonName, result);

}

bool DebugMenu::ButtonActionScriptMethod(const ButtonPtr& a_button, const char* a_methodName)
{
	if (!a_button) return false;
	return ButtonActionScriptMethod(a_button.get(), a_methodName);
}


void DebugMenu::GetRoot(RE::GFxValue& root)
{
	if (movie)
	{
		movie->GetVariable(&root, "_root");
	}
	else logger::debug("Failed to get root");
}

void DebugMenu::ScrollMenu(float a_distance, MenuPtr& a_menu)
{
	RE::GFxValue root;
	GetRoot(root);

	RE::GFxValue menu;
	root.GetMember(a_menu->GetName(), &menu);

	RE::GFxValue maskItems;
	auto hasMaskItems = menu.GetMember("maskItems", &maskItems); // only mask items are scrollable

	if (!hasMaskItems) return;

	RE::GFxValue::DisplayInfo itemsInfo;
	maskItems.GetDisplayInfo(&itemsInfo);

	float y = a_menu->maskItemsCurrentY + a_distance;

	if (y > a_menu->maskItemsDefaultY) y = a_menu->maskItemsDefaultY;

	float visibleSizeOfIcons = (y + a_menu->maskItemsYLocalOffset + a_menu->maskItemsHeight) - a_menu->maskYMin;
	float minimumVisibleSize = a_menu->maskHeight*0.8f;
	if (visibleSizeOfIcons < minimumVisibleSize)
	{
		y = minimumVisibleSize - a_menu->maskItemsYLocalOffset - a_menu->maskItemsHeight + a_menu->maskYMin;
	}

	RE::GFxValue newY{ y };
	bool res = maskItems.Invoke("SetY", nullptr, &newY, 1);


	float yDifference = y - a_menu->maskItemsCurrentY; // currentY is technially previous y at this point

	a_menu->maskItemsCurrentY = y;

	for (auto& button : a_menu->buttons)
	{
		if (!button->isInMask) continue;
		button->yMin += yDifference;
		button->yMax += yDifference;
	}
	// redo this
	for (auto& subMenu : a_menu->menus)
	{
		subMenu->y += yDifference;
		subMenu->yMin += yDifference;
		subMenu->yMax += yDifference;
		subMenu->maskYMin += yDifference;
		subMenu->maskYMax += yDifference;

		RE::GFxValue subMenu_;
		GetMenu(subMenu, subMenu_);
		RE::GFxValue newSubMenuY{ subMenu->y };
		auto res2 = subMenu_.Invoke("SetY", nullptr, &newSubMenuY, 1);

		for (auto& button : subMenu->buttons)
		{
			button->yMin += yDifference;
			button->yMax += yDifference;
		}
	}
}

void DebugMenu::SetNewRange(float& a_range, bool a_increase)
{
	{
		if (a_increase)
		{
			float maxRange = MCM::settings::maxRange;
			float step = MCM::settings::rangeStep;
			a_range = std::min(a_range + step, maxRange);
		}
		else
		{
			float minRange = MCM::settings::minRange;
			float step = MCM::settings::rangeStep;
			a_range = std::max(a_range - step, minRange);
		}
	}
}

void DebugMenu::InitButtonActionFunctions()
{
	if (logUI) logger::debug("  Initializing button action functions...");

	DayNightAction = [&](Button* a_button)
	{
		if (MCM::settings::dayNightIndex < 2) MCM::settings::dayNightIndex++;
		else MCM::settings::dayNightIndex = 0;		

		ButtonActionScriptMethod(a_button, "CycleIcon");
	};

	CellBorderAction = [&](Button* a_button)
	{
		MCM::settings::showCellBorders = a_button->isActive;
	};

	CellWallsAction = [&](Button* a_button)
	{
		MCM::settings::showCellWalls = a_button->isActive;
	};

	CellQuadsAction = [&](Button* a_button)
	{
		MCM::settings::showCellQuads = a_button->isActive;
	};

	NavMeshAction = [&](Button* a_button)
	{
		MCM::settings::showNavmesh = a_button->isActive;
	};

	NavMeshPlusAction = [&](Button* a_button)
	{
		SetNewRange(MCM::settings::navmeshRange, true);
		SetText("navMeshRangeText", MCM::settings::navmeshRange);
	};

	NavMeshMinusAction = [&](Button* a_button)
	{
		SetNewRange(MCM::settings::navmeshRange, false);
		SetText("navMeshRangeText", MCM::settings::navmeshRange);
	};

	NavMeshModeAction = [&](Button* a_button)
	{
		MCM::settings::useRuntimeNavmesh = a_button->isActive;
	};

	NavMeshTrianglesAction = [&](Button* a_button)
	{
		MCM::settings::showNavmeshTriangles = a_button->isActive;
	};

	NavMeshCoverAction = [&](Button* a_button)
	{
		MCM::settings::showNavmeshCover = a_button->isActive;
	};	


	OcclusionAction = [&](Button* a_button)
	{
		MCM::settings::showOcclusion = a_button->isActive;
	};

	OcclusionPlusAction = [&](Button* a_button)
	{
		SetNewRange(MCM::settings::occlusionRange, true);
		SetText("occlusionRangeText", MCM::settings::occlusionRange);
	};

	OcclusionMinusAction = [&](Button* a_button)
	{
		SetNewRange(MCM::settings::occlusionRange, false);
		SetText("occlusionRangeText", MCM::settings::occlusionRange);
	};

	CoordinatesAction = [&](Button* a_button)
	{
		MCM::settings::showCoordinates = a_button->isActive;
	};

	MarkersAction = [&](Button* a_button)
	{
		MCM::settings::showMarkers = a_button->isActive;
	};

	MarkersPlusAction = [&](Button* a_button)
	{
		SetNewRange(MCM::settings::markersRange, true);
		SetText("markersRangeText", MCM::settings::markersRange);
	};

	MarkersMinusAction = [&](Button* a_button)
	{
		SetNewRange(MCM::settings::markersRange, false);
		SetText("markersRangeText", MCM::settings::markersRange);
	};

	SelectMarkersAction = [&](Button* a_button)
	{
		auto menu = menuItems.GetMenuByName("markersSelectionMenu");
		if (!menu) return;
		if (menu->isActive) 
		{
			HideMenu(menu->GetName());
			MCM::DebugMenuPresets::SaveMarkerSettings(0);
		}
		else ShowMenu(menu->GetName());
	};

	LoadPreset1MarkersAction = [&](Button* a_button)
	{
		currentMarkerPresetIndex = 1;
		ShowMenu("LoadPresetsMenu");
	};

	LoadPreset2MarkersAction = [&](Button* a_button)
	{
		currentMarkerPresetIndex = 2;
		ShowMenu("LoadPresetsMenu");
	};

	LoadPreset3MarkersAction = [&](Button* a_button)
	{
		currentMarkerPresetIndex = 3;
		ShowMenu("LoadPresetsMenu");
	};

	LoadPresetAction = [&](Button* a_button)
	{
		MCM::DebugMenuPresets::LoadMarkerSettings(currentMarkerPresetIndex);
		UpdateMarkersSelectionButtons();
		HideMenu("LoadPresetsMenu");
	};
	CancelLoadPresetAction = [&](Button* a_button)
	{
		HideMenu("LoadPresetsMenu");
	};

	SavePreset1MarkersAction = [&](Button* a_button)
	{
		currentMarkerPresetIndex = 1;
		ShowMenu("SavePresetsMenu");
	};

	SavePreset2MarkersAction = [&](Button* a_button)
	{
		currentMarkerPresetIndex = 2;
		ShowMenu("SavePresetsMenu");
	};

	SavePreset3MarkersAction = [&](Button* a_button)
	{
		currentMarkerPresetIndex = 3;
		ShowMenu("SavePresetsMenu");
	};

	SavePresetAction = [&](Button* a_button)
	{
		MCM::DebugMenuPresets::SaveMarkerSettings(currentMarkerPresetIndex);
		HideMenu("SavePresetsMenu");
	};
	CancelSavePresetAction = [&](Button* a_button)
	{
		HideMenu("SavePresetsMenu");
	};


	if (logUI) logger::debug("  ...Finished button action function initialization");


};




