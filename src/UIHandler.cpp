#include "UIHandler.h"
#include "DebugUIMenu.h"
#include "DebugHandler.h"
#include "MCM.h"

void UIHandler::Init()
{
	
	DebugMenu::OpenMenu();

	auto uiTask = [&]() 
	{
		auto g_Debug = DebugHandler::GetSingleton();
		if (!g_Debug->hasDebugMenuBeenOpenedBefore) 
			g_Debug->hasDebugMenuBeenOpenedBefore = true; // never sat to false again

		GetDebugMenu();
		canvasWidth = g_DebugMenu->movie->GetVisibleFrameRect().right;
		canvasHeight = g_DebugMenu->movie->GetVisibleFrameRect().bottom;
		g_Cursor = RE::MenuCursor::GetSingleton();
		cursorWidthInPercentage = canvasWidth/g_Cursor->screenWidthX;
		cursorHeightInPercentage = canvasHeight/g_Cursor->screenWidthY;
		isMenuOpen = true;
	
		g_DebugMenu->ShowMenu();

		menuItems = g_DebugMenu->GetMenuItems();

		switch (MCM::settings::dayNightIndex)
		{
			case 0:
			{
				g_DebugMenu->ButtonMethod(menuItems.buttons[BUTTON::kDayNight].name, "SetSunIcon");
				break;
			}
			case 1:
			{
				g_DebugMenu->ButtonMethod(menuItems.buttons[BUTTON::kDayNight].name, "SetMoonIcon");
				break;
			}
			case 2:
			{
				g_DebugMenu->ButtonMethod(menuItems.buttons[BUTTON::kDayNight].name, "SetAutoIcon");
				break;
			}
		}

		if (MCM::settings::showCellWalls)
		{
			g_DebugMenu->ButtonMethod(menuItems.buttons[BUTTON::kCellWalls].name, "ON");
			menuItems.buttons[BUTTON::kCellWalls].isActive = true;
		}
		if (MCM::settings::showCellQuads)
		{
			g_DebugMenu->ButtonMethod(menuItems.buttons[BUTTON::kCellQuads].name, "ON");
			menuItems.buttons[BUTTON::kCellQuads].isActive = true;
		}
		if (MCM::settings::showCellBorders)
		{
			g_DebugMenu->ButtonMethod(menuItems.buttons[BUTTON::kCellBorder].name, "ON");
			menuItems.buttons[BUTTON::kCellBorder].isActive = true;	
		}
		else
		{
			g_DebugMenu->ButtonMethod(menuItems.buttons[BUTTON::kCellWalls].name, "OFF");
			g_DebugMenu->ButtonMethod(menuItems.buttons[BUTTON::kCellQuads].name, "OFF");
		}

		if (MCM::settings::showNavmesh)
		{

			g_DebugMenu->ButtonMethod(menuItems.buttons[BUTTON::kNavMesh].name, "ON");
			menuItems.buttons[BUTTON::kNavMesh].isActive = true;
		}

		if (g_Debug->useRuntimeNavmesh)
		{
			logger::info("Using runtime navmesh");
			g_DebugMenu->ButtonMethod(menuItems.buttons[BUTTON::kNavMeshMode].name, "ON");
			menuItems.buttons[BUTTON::kNavMeshMode].isActive = true;
		}

		if (MCM::settings::showOcclusion)
		{
			g_DebugMenu->ButtonMethod(menuItems.buttons[BUTTON::kOcclusion].name, "ON");
			menuItems.buttons[BUTTON::kOcclusion].isActive = true;
		}
	};
	SKSE::GetTaskInterface()->AddUITask(uiTask);
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
	for (auto& button : menuItems.buttons)
	{
		if (button.type == BUTTON::kCellQuads || button.type == BUTTON::kCellWalls)
			if(menuItems.buttons[BUTTON::kCellBorder].isActive == false) 
				continue;

		if (x > button.xMin && x < button.xMax && y > button.yMin && y < button.yMax)
		{
			if (mousePressed)
			{
				if (button.state != BUTTON_STATE::kHIT)
				{
					g_DebugMenu->ButtonMethod(button.name, "HIT");
					button.state = BUTTON_STATE::kHIT;
				}
			}
			else if (mouseReleased)
			{
				if (button.isActive) 
				{
					g_DebugMenu->ButtonMethod(button.name, "OFF");
					button.state = BUTTON_STATE::kOFF;
					button.isActive = false;
					ProcessButtonClick(button.type, button.isActive);
				}
				else
				{
					g_DebugMenu->ButtonMethod(button.name, "ON");
					button.state = BUTTON_STATE::kON;
					button.isActive = true;
					ProcessButtonClick(button.type, button.isActive);
				}
			}
			else if (!button.isActive && button.state != BUTTON_STATE::kHOVER)
			{
				g_DebugMenu->ButtonMethod(button.name, "HOVER");
				button.state = BUTTON_STATE::kHOVER;
			}
		}

		// This also sets buttons to the correct on/off state when the menu is opened as long as the active state of the buttons are set in Init()
		else if (!mousePressed || button.state != BUTTON_STATE::kHIT)
		{
			if (button.isActive && button.state != BUTTON_STATE::kON)
			{
				g_DebugMenu->ButtonMethod(button.name, "ON");
				button.state = BUTTON_STATE::kON;
			}
			else if(!button.isActive && button.state != BUTTON_STATE::kOFF)
			{
				g_DebugMenu->ButtonMethod(button.name, "OFF");
				button.state = BUTTON_STATE::kOFF;
			}
		}
	}
	if (mouseReleased) mouseReleased = false;
}

void UIHandler::ProcessButtonClick(BUTTON a_button, bool a_isActive)
{
	auto g_DebugHandler = DebugHandler::GetSingleton();
	g_DebugHandler->timeSinceLastUpdate = 2; // the slowest update is once per second

	switch (a_button)
	{
		case BUTTON::kDayNight :
		{ 
			if (MCM::settings::dayNightIndex < 2) MCM::settings::dayNightIndex++;
			else MCM::settings::dayNightIndex = 0;		

			g_DebugMenu->ButtonMethod(menuItems.buttons[a_button].name, "CycleIcon");
			break;
		}

		case BUTTON::kCellBorder : 
		{
			MCM::settings::showCellBorders = a_isActive;

			if (!a_isActive)
			{
				// show cellWalls and cellQuad buttons as off (no need to change the isActive status, as they won't be shown anyway when cellborders are not active
				g_DebugMenu->ButtonMethod(menuItems.buttons[BUTTON::kCellWalls].name, "OFF");
				g_DebugMenu->ButtonMethod(menuItems.buttons[BUTTON::kCellQuads].name, "OFF");
			}
			else
			{
				if (MCM::settings::showCellWalls) g_DebugMenu->ButtonMethod(menuItems.buttons[BUTTON::kCellWalls].name, "ON");
				if (MCM::settings::showCellQuads) g_DebugMenu->ButtonMethod(menuItems.buttons[BUTTON::kCellQuads].name, "ON");
			}
			break;
		}
		case BUTTON::kCellWalls :
		{
			MCM::settings::showCellWalls = a_isActive;
			break;
		}
		case BUTTON::kCellQuads :
		{
			MCM::settings::showCellQuads = a_isActive;
			break;
		}
		case BUTTON::kNavMesh : 
		{
			MCM::settings::showNavmesh = a_isActive;
			break;
		}
		case BUTTON::kNavMeshPlus : 
		{
			float currentRange = MCM::settings::navmeshRange;
			float maxRange = g_DebugHandler->maxRange;
			float step = MCM::settings::rangeStep;
			float newRange = std::min(currentRange + step, maxRange);

			MCM::settings::navmeshRange = newRange;
			g_DebugMenu->SetText("navMeshRangeText", newRange);

			break;
		}
		case BUTTON::kNavMeshMinus : 
		{
			float currentRange = MCM::settings::navmeshRange;
			float minRange = g_DebugHandler->minRange;
			float step = MCM::settings::rangeStep;
			float newRange = std::max(currentRange - step, minRange);

			MCM::settings::navmeshRange = newRange;
			g_DebugMenu->SetText("navMeshRangeText", newRange);

			break;
		}
		case BUTTON::kNavMeshMode :
		{
			g_DebugHandler->useRuntimeNavmesh = a_isActive;
			break;
		}
		case BUTTON::kOcclusion :
		{
			MCM::settings::showOcclusion = a_isActive;
			break;
		}
		case BUTTON::kOcclusionPlus :
		{
			float currentRange = MCM::settings::occlusionRange;
			float maxRange = g_DebugHandler->maxRange;
			float step = MCM::settings::rangeStep;
			float newRange = std::min(currentRange + step, maxRange);

			MCM::settings::occlusionRange = newRange;
			g_DebugMenu->SetText("occlusionRangeText", newRange);
			break;
		}
		case BUTTON::kOcclusionMinus :
		{
			float currentRange = MCM::settings::occlusionRange;
			float minRange = g_DebugHandler->minRange;
			float step =MCM::settings::rangeStep;
			float newRange = std::max(currentRange - step, minRange);

			MCM::settings::occlusionRange = newRange;
			g_DebugMenu->SetText("occlusionRangeText", newRange);

			break;
		}
	}
}

void UIHandler::GetDebugMenu()
{
	auto ui = RE::UI::GetSingleton();
			g_DebugMenu = ui ? ui->GetMenu<DebugMenu>(DebugMenu::MENU_NAME) : nullptr;
}

