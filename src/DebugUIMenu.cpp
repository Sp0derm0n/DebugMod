#include "DebugUIMenu.h"
#include "DebugMenu/DebugMenu.h"
#include "MCM.h"

DebugMenuUI::DebugMenuUI()
{

	auto menu = static_cast<RE::IMenu*>(this);
	menu->depthPriority = 1; // must be higher than depth of draw menu
	auto scaleformManager = RE::BSScaleformManager::GetSingleton();


	const auto success = scaleformManager->LoadMovieEx(this, MENU_PATH, RE::GFxMovieView::ScaleModeType::kNoBorder, 0.0, 
													   [](RE::GFxMovieDef* a_def) -> void 
	{
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
	//menu->inputContext = Context::kMenuMode; // Blocks movement
	if (uiMovie) movie = menu->uiMovie;
	else logger::debug("uiMovie not obtained");

}

void DebugMenuUI::Register()
{
	auto ui = RE::UI::GetSingleton();
	if (ui) 
	{
		ui->Register(MENU_NAME, Creator);
		logger::debug("Registered menu '{}'", MENU_NAME);
	}
}

RE::UI_MESSAGE_RESULTS DebugMenuUI::ProcessMessage(RE::UIMessage& a_message)
{
	if (a_message.type == RE::UI_MESSAGE_TYPE::kShow)
	{
		if (auto controlMap = RE::ControlMap::GetSingleton())
		{
			// kUsesCursor does not disable wheelzoom
			controlMap->ToggleControls(RE::UserEvents::USER_EVENT_FLAG::kWheelZoom, false, false);
			//controlMap->ToggleControls(RE::UserEvents::USER_EVENT_FLAG::kLooking, false, false);
		}
	}

	else if (a_message.type == RE::UI_MESSAGE_TYPE::kHide)
	{
		if (auto controlMap = RE::ControlMap::GetSingleton())
		{
			controlMap->ToggleControls(RE::UserEvents::USER_EVENT_FLAG::kWheelZoom, true, false);
			//controlMap->ToggleControls(RE::UserEvents::USER_EVENT_FLAG::kLooking, true, false);
		}
		MCM::DebugMenuPresets::SaveMarkerSettings(0);
	}
			

	return RE::UI_MESSAGE_RESULTS::kHandled;
}