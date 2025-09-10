#include "UIHandler.h"
#include "DebugUIMenu.h"
#include "DebugHandler.h"
#include "MCM.h"

// called when menu is opened
void UIHandler::Init() 
{
	DebugMenu::OpenMenu();

	keyScrollUp = false;
	keyScrollDown = false;
	mouseScrollUp = false;
	mouseScrollDown = false;

	auto uiTask = [&]() 
	{
		auto g_Debug = DebugHandler::GetSingleton();

		bool firstOpen = !g_Debug->hasDebugMenuBeenOpenedBefore;

		if (firstOpen)
		{
			MCM::DebugMenuMCM::ReadSettings(true);		
			g_Debug->hasDebugMenuBeenOpenedBefore = true; // never sat to false again
		}

		GetDebugMenu();
		canvasWidth = g_DebugMenu->movie->GetVisibleFrameRect().right;
		canvasHeight = g_DebugMenu->movie->GetVisibleFrameRect().bottom;
		//maskMinX = g_DebugMenu->maskMinX;
		//maskMaxX = g_DebugMenu->maskMaxX; 
		//maskMinY = g_DebugMenu->maskMinY;  
		//maskMaxY = g_DebugMenu->maskMaxY; 

		g_Cursor = RE::MenuCursor::GetSingleton();
		cursorWidthInPercentage = canvasWidth/g_Cursor->screenWidthX;
		cursorHeightInPercentage = canvasHeight/g_Cursor->screenWidthY;

		isMenuOpen = true;
	
		//g_DebugMenu->ShowMenu();

		menuItems = g_DebugMenu->GetMenuItems();


		////////// TEXT ////////////////////////////////////////////////////////////////////////////////////////////////////////////

		g_DebugMenu->SetText("navMeshRangeText",	MCM::settings::navmeshRange);
		g_DebugMenu->SetText("occlusionRangeText",	MCM::settings::occlusionRange);
		g_DebugMenu->SetText("markersRangeText",	MCM::settings::markersRange);

		////////// DAY NIGHT MODE /////////////////////////////////////////////////////////////////////////////////////////////

		bool success = true;

		const char* methodName;
		switch (MCM::settings::dayNightIndex)
		{
			case 0:
			{
				methodName = "SetSunIcon";
				break;
			}
			case 1:
			{
				methodName = "SetMoonIcon";
				break;
			}
			case 2:
			{
				methodName = "SetAutoIcon";
				break;
			}
		}
		success *= g_DebugMenu->ButtonActionScriptMethod(menuItems->GetButtonByName("dayNightIcon"), methodName);

		////////// CELL BORDERS /////////////////////////////////////////////////////////////////////////////////////////////

		if (MCM::settings::showCellWalls)
		{
			success *= InitializeToggleButtonOnMenuOpen("wallsIcon");
		}
		if (MCM::settings::showCellQuads)
		{
			success *= InitializeToggleButtonOnMenuOpen("quadsIcon");
		}
		if (MCM::settings::showCellBorders)
		{	
			success *= InitializeToggleButtonOnMenuOpen("cellBorderIcon");
		}

		////////// NAVMESH /////////////////////////////////////////////////////////////////////////////////////////////

		if (MCM::settings::showNavmeshTriangles)
		{
			success *= InitializeToggleButtonOnMenuOpen("trianglesIcon");
		}
		if (MCM::settings::showNavmeshCover)
		{
			success *= InitializeToggleButtonOnMenuOpen("coverIcon");
		}

		if (MCM::settings::showNavmesh)
		{
			success *= InitializeToggleButtonOnMenuOpen("navMeshIcon");
		}

		if (MCM::settings::useRuntimeNavmesh)
		{
			success *= InitializeToggleButtonOnMenuOpen("navMeshModeIcon");
		}

		////////// OCCLUSION /////////////////////////////////////////////////////////////////////////////////////////////


		if (MCM::settings::showOcclusion)
		{
			success *= InitializeToggleButtonOnMenuOpen("occlusionIcon");
		}

		////////// COORDINATES /////////////////////////////////////////////////////////////////////////////////////////////

		if (MCM::settings::showCoordinates)
		{
			success *= InitializeToggleButtonOnMenuOpen("coordinatesIcon");
		}

		////////// MARKERS /////////////////////////////////////////////////////////////////////////////////////////////

		if (MCM::settings::showMarkers)
		{
			success *= InitializeToggleButtonOnMenuOpen("markersIcon");
		}

		if (firstOpen)
		{
			if (success) logger::debug("Correctly initalized all button states");
			else logger::debug("Failed to initialize all button states");
		}

		for (auto& button : menuItems->buttons)
		{
			UpdateChildButtonsState(button);
		}
		
	};
	SKSE::GetTaskInterface()->AddUITask(uiTask);

}
bool UIHandler::InitializeToggleButtonOnMenuOpen(const char* a_buttonName)
{
	const auto& button = menuItems->GetButtonByName(a_buttonName);
	if (!button) return false;

	button->isActive = true;
	return true; //g_DebugMenu->ButtonActionScriptMethod(button, "ON");
}

void UIHandler::Unload()
{
	DebugMenu::CloseMenu();
	g_DebugMenu = nullptr;
	isMenuOpen = false;
}

void UIHandler::Update()
{
	if (!g_Cursor) return;

	float x = g_Cursor->cursorPosX*cursorWidthInPercentage;
	float y = g_Cursor->cursorPosY*cursorHeightInPercentage;

	bool isMenuInteractable = true;

	for (auto& menu : menuItems->menus)
	{
		if (!menu->isActive) continue;

		bool isCursorInMenu = x > menu->xMin && x < menu->xMax && y > menu->yMin && y < menu->yMax;
		bool isCursorInMask = x > menu->maskXMin && x < menu->maskXMax && y > menu->maskYMin && y < menu->maskYMax;

		if (isCursorInMask && (mouseScrollUp || mouseScrollDown))
		{
			float scrollDistance_ = scrollDistance;
			if (mouseScrollDown) scrollDistance_ *= -1.0f;
				
			g_DebugMenu->ScrollMenu(scrollDistance_, menu);
		}

		if (keyScrollUp || keyScrollDown)
		{
			float scrollDistance_ = scrollDistance*10;
			if (keyScrollDown) scrollDistance_ *= -1.0f;

			g_DebugMenu->ScrollMenu(scrollDistance_, menu);

			// Only scroll the top most active menu
			keyScrollDown = false;
			keyScrollUp = false;
		}


		for (auto& button : menu->buttons)
		{
			if (button->IsChildButton())
			{
				if (!button->parent->isActive && !button->HasChildFlag(ChildFlags::kInteractable)) continue;
			}

			bool isCursorOverButton = x > button->xMin && x < button->xMax - 2 && y > button->yMin && y < button->yMax - 2; // - 2 on the max values since the cursor position has a resolution of 1.5 pixels)
			bool isButtonInteractable = isCursorInMask || !button->isInMask; // buttons in the scrollable area should only be interactable when they are in the mask

			if (isMenuInteractable && isCursorOverButton && isButtonInteractable)
			{
				if (mousePressed)
				{
					if (button->state != BUTTON_STATE::kHIT)
					{
						g_DebugMenu->ButtonActionScriptMethod(button, "HIT");
						button->state = BUTTON_STATE::kHIT;
					}
				}
				else if (mouseReleased)
				{
					if (button->isActive)
					{
						// if button is child button and interactable, if its parent is disabled, treat the disable of this button as an enable
						if (button->IsChildButton() && button->HasChildFlag(ChildFlags::kInteractable) && !button->parent->isActive) {}
						else
						{
							g_DebugMenu->ButtonActionScriptMethod(button, "OFF");
							button->state = BUTTON_STATE::kOFF;
							button->isActive = false;
						}
						ProcessButtonClick(button);
					}
					else
					{
						g_DebugMenu->ButtonActionScriptMethod(button, "ON");
						button->state = BUTTON_STATE::kON;
						button->isActive = true;
						ProcessButtonClick(button);
					}
				}
				else if (!button->isActive && button->state != BUTTON_STATE::kHOVER)
				{
					g_DebugMenu->ButtonActionScriptMethod(button, "HOVER");
					button->state = BUTTON_STATE::kHOVER;
				}
			}

			// This also sets buttons to the correct on/off state when the menu is opened as long as the active state of the buttons are set in Init()
			else if (!mousePressed || button->state != BUTTON_STATE::kHIT)
			{
				if (button->isActive && button->state != BUTTON_STATE::kON)
				{
					g_DebugMenu->ButtonActionScriptMethod(button, "ON");
					button->state = BUTTON_STATE::kON;
				}
				else if (!button->isActive && button->state != BUTTON_STATE::kOFF)
				{
					g_DebugMenu->ButtonActionScriptMethod(button, "OFF");
					button->state = BUTTON_STATE::kOFF;
				}
			}
		}
		// some of the code needs only run when the cursor is in one of the menus - other must run every frame
		if (isCursorInMenu && isMenuInteractable) 
			isMenuInteractable = false;

	}
	// set scroll up and down to false such that a scroll is not buffered, since the variables are sat to true whenever the scroll wheel is used
	keyScrollUp = false;
	keyScrollDown = false;
	mouseScrollUp = false;
	mouseScrollDown = false;
	if (mouseReleased) mouseReleased = false; // makes it so mouseReleased is only true during 1 frame
}

void UIHandler::UpdateChildButtonsState(std::shared_ptr<Button>& a_parentButton)
{
	// if the button's children does not have the interactable tag, set them to off when disabling parent button, and on if they are supposed to be on when parent is on
	for (const auto& child : a_parentButton->children)
	{
		if (!child->HasChildFlag(ChildFlags::kInteractable))
		{
			if (a_parentButton->isActive && child->isActive)
			{
				g_DebugMenu->ButtonActionScriptMethod(child.get(), "ON");
			}
			else
			{
				g_DebugMenu->ButtonActionScriptMethod(child.get(), "OFF");
			}
		}
	}
}

void UIHandler::UpdateParentButtonState(std::shared_ptr<Button>& a_childButton)
{
	if (!a_childButton->IsChildButton()) return;
	if (!a_childButton->HasChildFlag(ChildFlags::kInteractable)) return;

	if (a_childButton->isActive)
	{
		a_childButton->parent->isActive = true;
		a_childButton->parent->ButtonAction();
	}
	else if (a_childButton->parent->isActive) // disable the parent if all its children are interactable, and disabled
	{
		bool areAllChildrenInteractable = true;
		bool areAllChildrenInactive = true;
		for (const auto& child : a_childButton->parent->children)
		{
			if (!child->HasChildFlag(ChildFlags::kInteractable))
			{
				areAllChildrenInteractable = false;
			}

			if (child->isActive)
			{
				areAllChildrenInactive = false;
			}
		}
		if (areAllChildrenInteractable && areAllChildrenInactive)
		{
			a_childButton->parent->isActive = false;
			a_childButton->parent->ButtonAction();
		}
	}
	
}


void UIHandler::ProcessButtonClick(std::shared_ptr<Button>& a_button)
{
	if (mouseReleased) mouseReleased = false; // makes it so only one button can be pressed pr frame

	auto g_DebugHandler = DebugHandler::GetSingleton();
	g_DebugHandler->ResetUpdateTimer();

	if (a_button->type == BUTTON_TYPE::kClick)
	{
		a_button->isActive = false; // makes hover allways work, as it only works when button->isActive = false
	}

	a_button->ButtonAction();

	UpdateChildButtonsState(a_button);
	UpdateParentButtonState(a_button);

	return;

	
}

void UIHandler::SetNewRange(float& a_range, bool a_increase)
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

void UIHandler::GetDebugMenu()
{
	auto ui = RE::UI::GetSingleton();
	g_DebugMenu = ui ? ui->GetMenu<DebugMenu>(DebugMenu::MENU_NAME) : nullptr;
}




//switch (a_button->button)
//{
//	case BUTTON::kDayNight :
//	{ 
//		if (MCM::settings::dayNightIndex < 2) MCM::settings::dayNightIndex++;
//		else MCM::settings::dayNightIndex = 0;		
//
//		g_DebugMenu->ButtonActionScriptMethod(a_button->name, "CycleIcon");
//		break;
//	}
//
//	case BUTTON::kCellBorder : 
//	{
//		MCM::settings::showCellBorders = a_button->isActive;
//
//		if (!a_button->isActive)
//		{
//			// show cellWalls and cellQuad buttons as off (no need to change the isActive status, as they won't be shown anyway when cellborders are not active
//			g_DebugMenu->ButtonActionScriptMethod(menuItems->buttons[BUTTON::kCellWalls]->name, "OFF");
//			g_DebugMenu->ButtonActionScriptMethod(menuItems->buttons[BUTTON::kCellQuads]->name, "OFF");
//		}
//		else
//		{
//			if (MCM::settings::showCellWalls) g_DebugMenu->ButtonActionScriptMethod(menuItems->buttons[BUTTON::kCellWalls]->name, "ON");
//			if (MCM::settings::showCellQuads) g_DebugMenu->ButtonActionScriptMethod(menuItems->buttons[BUTTON::kCellQuads]->name, "ON");
//		}
//		break;
//	}
//	case BUTTON::kCellWalls :
//	{
//		MCM::settings::showCellWalls = a_button->isActive;
//		break;
//	}
//	case BUTTON::kCellQuads :
//	{
//		MCM::settings::showCellQuads = a_button->isActive;
//		break;
//	}
//	case BUTTON::kNavMesh : 
//	{
//		MCM::settings::showNavmesh = a_button->isActive;
//
//		if (!a_button->isActive)
//		{
//			// show cellWalls and cellQuad buttons as off (no need to change the isActive status, as they won't be shown anyway when cellborders are not active
//			if (MCM::settings::showNavmeshTriangles) g_DebugMenu->ButtonActionScriptMethod(menuItems->buttons[BUTTON::kNavMeshTriangles]->name, "OFF");
//			if (MCM::settings::showNavmeshCover) g_DebugMenu->ButtonActionScriptMethod(menuItems->buttons[BUTTON::kNavMeshCover]->name, "OFF");
//		}
//		else
//		{
//			if (MCM::settings::showNavmeshTriangles) g_DebugMenu->ButtonActionScriptMethod(menuItems->buttons[BUTTON::kNavMeshTriangles]->name, "ON");
//			if (MCM::settings::showNavmeshCover) g_DebugMenu->ButtonActionScriptMethod(menuItems->buttons[BUTTON::kNavMeshCover]->name, "ON");
//		}
//		break;
//	}
//	case BUTTON::kNavMeshPlus : 
//	{
//		SetNewRange(MCM::settings::navmeshRange, true);
//		g_DebugMenu->SetText("navMeshRangeText", MCM::settings::navmeshRange);
//
//		break;
//	}
//	case BUTTON::kNavMeshMinus : 
//	{
//		SetNewRange(MCM::settings::navmeshRange, false);
//		g_DebugMenu->SetText("navMeshRangeText", MCM::settings::navmeshRange);
//
//		break;
//	}
//	case BUTTON::kNavMeshMode :
//	{
//		MCM::settings::useRuntimeNavmesh = a_button->isActive;
//		break;
//	}
//	case BUTTON::kNavMeshTriangles :
//	{
//		MCM::settings::showNavmeshTriangles = a_button->isActive;
//		break;
//	}
//	case BUTTON::kNavMeshCover :
//	{
//		MCM::settings::showNavmeshCover = a_button->isActive;
//		break;
//	}
//	case BUTTON::kOcclusion :
//	{
//		MCM::settings::showOcclusion = a_button->isActive;
//		break;
//	}
//	case BUTTON::kOcclusionPlus :
//	{
//		SetNewRange(MCM::settings::occlusionRange, true);
//		g_DebugMenu->SetText("occlusionRangeText", MCM::settings::occlusionRange);
//		break;
//	}
//	case BUTTON::kOcclusionMinus :
//	{
//		SetNewRange(MCM::settings::occlusionRange, false);
//		g_DebugMenu->SetText("occlusionRangeText", MCM::settings::occlusionRange);
//
//		break;
//	}
//	case BUTTON::kCoordinates :
//	{
//		MCM::settings::showCoordinates = a_button->isActive;
//		break;
//	}
//	case BUTTON::kMarkers :
//	{
//		MCM::settings::showMarkers = a_button->isActive;
//		break;
//	}
//	case BUTTON::kMarkersPlus :
//	{
//		SetNewRange(MCM::settings::markersRange, true);
//		g_DebugMenu->SetText("markersRangeText", MCM::settings::markersRange);
//		break;
//	}
//	case BUTTON::kMarkersMinus :
//	{
//		SetNewRange(MCM::settings::markersRange, false);
//		g_DebugMenu->SetText("markersRangeText", MCM::settings::markersRange);
//		break;
//	}
//	case BUTTON::kSelectMarkers :
//	{
//		break;
//	}
//}

