#include "UIHandler.h"
#include "DebugUIMenu.h"
#include "DrawMenu.h"
#include "Utils.h"
#include "InputHandler.h"
#include "DebugMenu/DebugMenu.h"
#include "FreeCamHandler.h"

#define LEFT ScaleformUI::Element::Direction::left
#define RIGHT ScaleformUI::Element::Direction::right
#define TOP ScaleformUI::Element::Direction::top
#define BOTTOM ScaleformUI::Element::Direction::bottom

#define CENTERH ScaleformUI::Alignment::kCenterH
#define CENTERV ScaleformUI::Alignment::kCenterV

#define INSIDE ScaleformUI::Alignment::kInside;
#define OUTSIDE ScaleformUI::Alignment::kOutside;

void ScaleformUI::UIHandler::Init()
{
	logger::debug("Initializing UIHandler");
	InitDrawMenu(); // Must initialize before debug menu UI
	InitDebugMenuUI();
	InputHandler::GetSingleton()->SetScrollUpKey(MCM::settings::scrollUpHotkey);
	InputHandler::GetSingleton()->SetScrollDownKey(MCM::settings::scrollDownHotkey);

	HandleLanguage();
}

void ScaleformUI::UIHandler::HandleLanguage()
{
	const auto iniSettingCollection = RE::INISettingCollection::GetSingleton();
	auto languageSetting = iniSettingCollection ? iniSettingCollection->GetSetting("sLanguage:General") : nullptr;
	auto fontSetting = iniSettingCollection ? iniSettingCollection->GetSetting("sFontConfigFile:Fonts") : nullptr;

	std::string language = (languageSetting && languageSetting->GetType() == RE::Setting::Type::kString) ? languageSetting->data.s : "ENGLISH"s;
	std::string fontConfigFile = (fontSetting && fontSetting->GetType() == RE::Setting::Type::kString) ? fontSetting->data.s : "Interface\FontConfig.txt";

	auto StringToLowerCase = [](std::string& a_str)
	{
		std::transform(a_str.begin(), a_str.end(), a_str.begin(), [](unsigned char c) { return std::tolower(c); });
	};
	
	StringToLowerCase(language);
	StringToLowerCase(fontConfigFile);

	if (language == "chinese"s || fontConfigFile == "interface\\fontconfig_cn.txt"s)
	{
		skyrimBooksFontTranslated = "DFMing-B5 Otf W9";
		skyrimFuturaFontTranslated = "DFMing-B5 Otf W9"; //"$DialogueFont"
	}
	else if (language == "japanese"s || fontConfigFile == "interface\\fontconfig_ja.txt"s)
	{
		skyrimBooksFontTranslated = "1_Skyrim_JP_EveryFont_0805";
		skyrimFuturaFontTranslated = "22_Skyrim_JP_BookFont_0805"; 
	}
	else if (language == "russian"s || fontConfigFile == "interface\\fontconfig_ru.txt"s)
	{
		skyrimBooksFontTranslated = "FuturaTCYLigCon";
	}


}

void ScaleformUI::UIHandler::InitDrawMenu()
{
	drawMenu = std::make_unique<Menu>(DrawMenu::MENU_NAME);
	drawMenu->SetOpenMenuFunction([&]() -> bool
	{
		return DebugMenu::GetDebugMenuHandler()->hasDebugMenuBeenOpenedBefore;
	});
	//drawMenu->SetMenuOpenKeyCode(MCM::settings::openMenuHotkey);
	BuildDrawMenu();
}

void ScaleformUI::UIHandler::InitDebugMenuUI()
{
	debugMenuUI = std::make_unique<Menu>(DebugMenuUI::MENU_NAME);
	debugMenuUI->SetOpenMenuFunction([&]() -> bool
	{
		if (!MCM::settings::modActive) return false;

	#ifndef DISPLAY_UI_ON_MAINMENU
		if (RE::UI::GetSingleton()->GameIsPaused()) return false;
	#endif

		DebugMenu::GetDebugMenuHandler()->OnDebugMenuUIOpen();

		return true;
	});
	debugMenuUI->SetMenuOpenKeyCode(MCM::settings::openMenuHotkey);

	BuildDebugMenu();
}

void ScaleformUI::UIHandler::BuildDrawMenu()
{
	auto attachNewCanvasElements = [&](Menu* a_menu)
	{


	#pragma region INFO_BOX
		auto infoBox = a_menu->AttachUIElement("InfoFrame");
		infoBox->SetHeight(Size(0.88f).vh());
		infoBox->AlignInsideParent(BOTTOM, RIGHT);
		infoBox->SetPadding(BOTTOM, Size(0.03f).vh());
		infoBox->SetPadding(RIGHT, Size(0.01f).vh());

		// structure:
		// box
		//   up, mask, down <- aligned vertically
		//   textField <- aligned inside mask

		auto infoTextGroup = infoBox->CreateGroup("(InfoTextGroup)");
		auto infoTextUpArrow = infoTextGroup->AttachUIElement("UpArrow");
		auto infoTextMask = infoTextGroup->CreateSquareElement("InfoTextMask", 0xFFFFFF);
		auto infoTextDownArrow = infoTextGroup->AttachUIElement("DownArrow");

		infoTextUpArrow->SetInvisible();
		infoTextDownArrow->SetInvisible();

		infoTextMask->UnlockAspect();
		infoTextMask->SetWidth(Size(0.9f).pw());
		infoTextMask->SetHeight(Size(0.9f).ph());

		infoTextGroup->AlignChildrenVertically(5.0f, CENTERH);
		infoTextGroup->AlignInsideParent(CENTERV, CENTERH);

		// Make text field

		auto infoTextScrollableGroup = infoBox->CreateGroup("(InfoTextScrollableGroup)");
		auto infoText = infoTextScrollableGroup->CreateTextField("InfoText");

		infoText->SetTextfieldWidth(infoTextMask->GetWidth());
		infoText->SetFontSize(16);
		infoText->SetTextAlign("left");
		infoText->SetWordWrap(true);
		infoText->SetAutoSize(true);
		infoText->SetFontColor(0xFFFFFF);
		infoText->SetFont(skyrimFuturaFont);

		infoText->AlignInsideOther(infoTextMask, TOP, LEFT);
		infoTextScrollableGroup->SetVerticalScrollable(true);
		infoTextScrollableGroup->SetScrollableArea(infoTextMask);
		infoTextScrollableGroup->SetMask(*infoTextMask);
		infoTextScrollableGroup->SetOnScrollCallback([=](float a_ratio)
		{
			float eps = 0.005f;
			if (a_ratio < eps)
			{
				infoTextUpArrow->SetInvisible();
			}
			else
			{
				infoTextUpArrow->SetVisible();
			}
			if (a_ratio > 1.0f - eps)
			{
				infoTextDownArrow->SetInvisible();
			}
			else
			{
				infoTextDownArrow->SetVisible();
			}
		});

		infoBox->SetInvisible();



	#pragma endregion

	#pragma region OVERLAY

		auto overlayFrame = a_menu->AttachUIElement("OverlayFrame");
		overlayFrame->SetWidth(infoBox->GetWidth());
		overlayFrame->AlignOutsideOther(infoBox, CENTERH, TOP);
		overlayFrame->SetPadding(BOTTOM, Size(0.01f).vh());
		auto overlayContentsGroup = overlayFrame->CreateGroup("(OverlayContentsGroup)");

		auto positionGroup = overlayContentsGroup->CreateGroup("(PositionGroup");
		auto xLabel = positionGroup->CreateTextField("xLabel");
		auto yLabel = positionGroup->CreateTextField("yLabel");
		auto zLabel = positionGroup->CreateTextField("zLabel");

		IElement* labels[3]{ xLabel, yLabel, zLabel };
		for (int i = 0; i < 3; i++)
		{
			auto label = labels[i];
			label->SetText("w: 123456");
			label->SetFont(skyrimFuturaFont);
			label->SetFontSize(16);
			label->SetAutoSize(false);
		}



		positionGroup->AlignChildrenHorizontally(Size(0.09f).pw(), CENTERV);

		overlayContentsGroup->AlignInsideParent(CENTERV, CENTERH);

		

	#pragma endregion


		// maybe detach frames from rest to set frame alpha less than 100 but keep rest at 100

		infoBox->SetLayoutAlpha(65);
		overlayFrame->SetLayoutAlpha(65);

		SetOverlayPosition = [=](float a_x, float a_y, float a_z)
		{
			auto uiTask = [=]
			{
				if (!overlayFrame->IsVisible()) return;

				xLabel->SetText(fmt::format("x: {:>7.0f}", a_x));
				yLabel->SetText(fmt::format("y: {:>7.0f}", a_y));
				zLabel->SetText(fmt::format("z: {:>7.0f}", a_z));
			};
			SKSE::GetTaskInterface()->AddUITask(uiTask);
			
		};
		ShowOverlayIfItsHidden = [=]()
		{
			auto uiTask = [=]
			{
				if (!overlayFrame->IsVisible()) overlayFrame->Show();
			};
			SKSE::GetTaskInterface()->AddUITask(uiTask);
		};
		HideOverlayIfItsVisible = [=]()
		{
			auto uiTask = [=]
			{
				if (overlayFrame->IsVisible()) overlayFrame->Hide();
			};
			SKSE::GetTaskInterface()->AddUITask(uiTask);
		};

		ShowInfoBoxIfItsHidden = [=]()
		{
			auto uiTask = [=]
			{
				if (!infoBox->IsVisible()) infoBox->Show();
			};
			SKSE::GetTaskInterface()->AddUITask(uiTask);
		};
		HideInfoBoxIfItsVisible = [=]()
		{
			auto uiTask = [=]
			{
				if (infoBox->IsVisible()) infoBox->Hide();
			};
			SKSE::GetTaskInterface()->AddUITask(uiTask);
		};
		SetInfoText = [=](std::string a_text)
		{
			auto uiTask = [=]
			{
				infoText->SetText(a_text);
				infoTextScrollableGroup->AsUIElement()->AsGroup()->ResetScroll();
				if (infoText->AsUIElement()->GetGlobalYMax() > infoTextMask->AsUIElement()->GetGlobalYMax())
				{
					infoTextDownArrow->SetVisible();
				}
				else
				{
					infoTextDownArrow->SetInvisible();
				}
			};
			SKSE::GetTaskInterface()->AddUITask(uiTask);
		};

		IsInfoBoxOpen = [=]()
		{
			return infoBox->IsVisible();
		};
	};

	drawMenu->Build(attachNewCanvasElements);
}

void ScaleformUI::UIHandler::BuildDebugMenu()
{
	// Use create object to create object of class type

	auto attachNewCanvasElements = [&](Menu* a_menu)
	{

		// constants
		const auto		mainMenuGroupsSpacing = Size(0.003).ph();
		const auto		mainMenuSmallElementsSpacing = Size(-0.005).ph();
		const uint32_t	subElementFontSize = 16;
		const float		openMainMenuTransitionTime = 0.12f;
		const float		buttonTransitionTime = 0.07f;
		const float		openYesNoBoxTransitionTime = 0.14f;
		const uint32_t	containerAlpha = 65;
		const uint32_t	tooltipsFontSize = 22;
		const float		toolTipsMaxWidthRatio = 0.25f;
		const float		toolTipsBorderSize = 0.03f * a_menu->ScreenH();
		const float		toolTipsDelay = 0.3f;

	#pragma region TOOL_TIPS_MESSAGES

		std::string dayNightToolTip;
		std::string closeMenuToolTip;
		std::string decreaseRangeToolTip;
		std::string increaseRangeToolTip;
		std::string navmeshModeToolTip;
		std::string collisionModeToolTip;
		std::string zLockToolTip;
		std::string doubleAscendToolTip;
		std::string followPlayerToolTip;

		std::vector<std::string*> toolTipStrings{
			&dayNightToolTip,
			&closeMenuToolTip,
			&decreaseRangeToolTip,
			&increaseRangeToolTip,
			&navmeshModeToolTip,
			&collisionModeToolTip,
			&zLockToolTip,
			&doubleAscendToolTip,
			&followPlayerToolTip,
		};

		SKSE::Translation::Translate("$DM_ToolTip_DayNightMode________________", dayNightToolTip);
		SKSE::Translation::Translate("$DM_ToolTip_CloseMenu___________________", closeMenuToolTip);
		SKSE::Translation::Translate("$DM_ToolTip_DecreaseRange_______________", decreaseRangeToolTip);
		SKSE::Translation::Translate("$DM_ToolTip_IncreaseRange_______________", increaseRangeToolTip);
		SKSE::Translation::Translate("$DM_ToolTip_NavmeshMode_________________", navmeshModeToolTip);
		SKSE::Translation::Translate("$DM_ToolTip_CollisionMode_______________", collisionModeToolTip);
		SKSE::Translation::Translate("$DM_ToolTip_zLock_______________________", zLockToolTip);
		SKSE::Translation::Translate("$DM_ToolTip_DoubleAscend________________", doubleAscendToolTip);
		SKSE::Translation::Translate("$DM_ToolTip_FollowPlayer________________", followPlayerToolTip);

		auto fixLineBreaksInString = [](std::string & a_string)
		{
			std::string::size_type index = 0;
			while ((index = a_string.find("\\n", index)) != std::string::npos) 
			{
				a_string.replace(index, 2, "\n");
				++index;
			}

			index = 0;
			while ((index = a_string.find("\\t", index)) != std::string::npos)
			{
				a_string.replace(index, 2, "\t");
				++index;
			}
		};

		for (auto toolTipString : toolTipStrings)
		{
			fixLineBreaksInString(*toolTipString);
		}

	#pragma endregion

		a_menu->SetDefaultFontColor(0xFFFFFF);

	#pragma region ANIMATIONS

		auto toggleButtonHoverAnimation = [=](AnimationHandler* a)
		{
			auto element = a->GetElement();

			//SetOnOffIcon(element, true);
			element->AsUIElement()->AsButton()->SetVisibleOnToggleImage();
			if (element->IsChecked())
			{
				a->PlayAlignedScaleAnimation(buttonTransitionTime, 0.9f);
				a->PlayToAlphaAnimation(buttonTransitionTime, 100);
			}
			else
			{
				a->PlayAlignedScaleAnimation(buttonTransitionTime, 1.0f);
				a->PlayToAlphaAnimation(buttonTransitionTime, 60);
			}


		};
		auto toggleButtonPressedAnimation = [=](AnimationHandler* a)
		{
			a->PlayAlignedScaleAnimation(buttonTransitionTime, 0.8f);
			a->PlayToAlphaAnimation(buttonTransitionTime, 100);
		};
		auto toggleButtonResetAnimation = [=](AnimationHandler* a)
		{
			auto anim = a->PlayResetAnimation(buttonTransitionTime);

			auto element = a->GetElement();
			if (!element->IsChecked()) element->AsUIElement()->AsButton()->SetVisibleOffToggleImage();
			//SetOnOffIcon(element, element->IsChecked());
		};

		auto clickButtonHoverAnimation = [=](AnimationHandler* a)
		{
			a->PlayAlignedScaleAnimation(buttonTransitionTime, 0.9f);
		};
		auto clickButtonPressedAnimation = [=](AnimationHandler* a)
		{
			a->PlayAlignedScaleAnimation(buttonTransitionTime, 0.8f);
		};
		auto clickButtonResetAnimation = [=](AnimationHandler* a)
		{
			auto anim = a->PlayResetAnimation(buttonTransitionTime);
		};

		auto& cyclicButtonHoverAnimation = clickButtonHoverAnimation;
		auto& cyclicButtonPressedAnimation = clickButtonPressedAnimation;
		auto& cyclicButtonResetAnimation = clickButtonResetAnimation;

		auto setLayoutVisibleStatus = [](IElement* a_element, bool a_visible)
		{
			auto elementGFx = a_element->GetGFx();

			if (elementGFx.HasMember(IElement::LAYOUT_NAME))
			{
				RE::GFxValue layout;
				elementGFx.GetMember(IElement::LAYOUT_NAME, &layout);

				RE::GFxValue::DisplayInfo displayInfo;
				layout.GetDisplayInfo(&displayInfo);

				displayInfo.SetVisible(a_visible);
				layout.SetDisplayInfo(displayInfo);
			}
		};
		auto invisibleClickButtonHoverAnimation = [=](AnimationHandler* a)
		{
			a->PlayAlignedScaleAnimation(buttonTransitionTime, 0.9f);
			a->PlayToAlphaAnimation(buttonTransitionTime, 100);

			setLayoutVisibleStatus(a->GetElement(), true);
			
		};
		auto invisibleClickButtonPressedAnimation = [=](AnimationHandler* a)
		{
			a->PlayAlignedScaleAnimation(buttonTransitionTime, 0.8f);
			a->PlayToAlphaAnimation(buttonTransitionTime, 100);

			setLayoutVisibleStatus(a->GetElement(), true);
		};
		auto invisibleClickButtonResetAnimation = [=](AnimationHandler* a)
		{
			auto anim = a->PlayToAlphaAnimation(buttonTransitionTime, 0);
			a->PlayAlignedScaleAnimation(buttonTransitionTime, 1.0f);
			anim->RestoreElementOnAnimationFinish();

			anim->AddPostCallback([=](Element* a_element)
			{
				setLayoutVisibleStatus(a_element, false);
			});
		};

		auto openMainMenuAnimation = [=](AnimationHandler* a)
		{
			a->PlayFromAlphaAnimation(openMainMenuTransitionTime, 0);
			a->PlayMoveFromAnimation(openMainMenuTransitionTime, Size(a->GetElement()->GetX()).global() + Size(0.1 * a->GetElement()->GetWidth()).global(), a->GetElement()->GetY());
			
		};
		auto hideMainMenuAnimation = [=](AnimationHandler* a)
		{
			a->PlayToAlphaAnimation(openMainMenuTransitionTime, 0);
			a->PlayMoveToAnimation(openMainMenuTransitionTime, Size(a->GetElement()->GetX()).global() + Size(0.1f * a->GetElement()->GetWidth()).global(), a->GetElement()->GetY());
		};
		auto openSelectMarkersMenuAnimation = [=](AnimationHandler* a)
		{
			a->PlayFromAlphaAnimation(openMainMenuTransitionTime, 0);
			a->PlayMoveFromAnimation(openMainMenuTransitionTime, a->GetElement()->GetX(), Size(a->GetElement()->GetY()).global() + Size(0.01f).vh());
		};
		auto hideSelectMarkersMenuAnimation = [=](AnimationHandler* a)
		{
			a->PlayToAlphaAnimation(openMainMenuTransitionTime, 0);
			a->PlayMoveToAnimation(openMainMenuTransitionTime, a->GetElement()->GetX(), Size(a->GetElement()->GetY()).global() + Size(0.01f).vh());
		};

		auto yesNoBoxOpenAnimation = [=](AnimationHandler* a)
		{
			a->PlayScaleFromAnimation(openYesNoBoxTransitionTime, 0.1f);
			if (const auto& menuCursor = RE::MenuCursor::GetSingleton())
			{
				a->PlayMoveFromAnimation(openYesNoBoxTransitionTime, Size(menuCursor->cursorPosX).global(), Size(menuCursor->cursorPosY).global());
			}
		};
		auto yesNoBoxCloseAnimation = [=](AnimationHandler* a)
		{
			a->PlayAlignedScaleAnimation(openYesNoBoxTransitionTime, 0.0f);
			if (const auto& menuCursor = RE::MenuCursor::GetSingleton())
			{
				a->PlayMoveToAnimation(openYesNoBoxTransitionTime, Size(menuCursor->cursorPosX).global(), Size(menuCursor->cursorPosY).global());
			}
		};

		auto openToolTipsAnimation = [=](AnimationHandler* a)
		{
			a->PlayFromAlphaAnimation(openMainMenuTransitionTime, 0);
			a->PlayMoveFromAnimation(openMainMenuTransitionTime, a->GetElement()->GetX(), Size(a->GetElement()->GetY()).global() + Size(0.01f).vh());
		};
		auto hideToolTipsAnimation = [=](AnimationHandler* a)
		{
			a->PlayToAlphaAnimation(openMainMenuTransitionTime, 0);
			a->PlayMoveToAnimation(openMainMenuTransitionTime, a->GetElement()->GetX(), Size(a->GetElement()->GetY()).global() + Size(0.01f).vh());
		};


	#pragma endregion

		auto updateDebugMenuCallback = [&](Element*, MOUSE_STATE a_state, bool)
		{
			switch (a_state)
			{
				case MOUSE_STATE::kCHECKED:
				{
					DebugMenu::GetDebugMenuHandler()->ResetUpdateTimer();
					break;
				}
			}
		};

	#pragma region TOOL_TIPS

		auto toolTipsGroup = a_menu->CreateGroup("(ToolTipsGroup)");

		// Fill the entire screen with a single color
		//auto whiteBG = a_menu->CreateSquareElement("WhiteBackground", 0x00FF00);
		//whiteBG->UnlockAspect();
		//whiteBG->SetWidth(Size(1.0f).vw());
		//whiteBG->SetHeight(Size(1.0f).vh());
		//whiteBG->SetLayoutAlpha(50);

		auto toolTipsFrame = toolTipsGroup->CreateSquareElement("ToolTipsFrame", 0x000000);
		toolTipsFrame->SetLayoutAlpha(80);
		auto toolTipsText = toolTipsGroup->CreateTextField("ToolTipsText");
		toolTipsText->SetFontSize(tooltipsFontSize);
		toolTipsText->SetFont(skyrimFuturaFontTranslated);
		toolTipsText->SetText("Tooltips textfield");
		toolTipsText->SetTextAlign("left");
		toolTipsText->SetFontColor(0xEEEEEE);

		toolTipsGroup->SetShowAnimation(openToolTipsAnimation);
		toolTipsGroup->SetHideAnimation(hideToolTipsAnimation);

		auto ShowToolTips = [=](std::string a_msg)
		{
			// Set autosize to true and wordwrap to false, to see how big the textfield gets
			// if it is too large, change its size, then enable wordwrap and enable autosize again, so it adjusts downwards
			toolTipsText->SetAutoSize(true);
			toolTipsText->SetWordWrap(false);
			toolTipsText->SetText(a_msg);

			if (toolTipsText->GetWidth() > toolTipsMaxWidthRatio * a_menu->ScreenW())
			{
				toolTipsText->SetTextfieldWidth(Size(toolTipsMaxWidthRatio).vw());
				toolTipsText->SetWordWrap(true);
				toolTipsText->SetAutoSize(true);
			}


			toolTipsFrame->UnlockAspect();
			// extend frame with a border proportional to view height
			toolTipsFrame->SetWidth(toolTipsText->GetWidth() + toolTipsBorderSize);
			toolTipsFrame->SetHeight(toolTipsText->GetHeight() + toolTipsBorderSize);

			toolTipsText->AlignInsideOther(toolTipsFrame, CENTERV, CENTERH);

			if (auto cursor = RE::MenuCursor::GetSingleton())
			{
				float cursorX = cursor->GetRuntimeData().cursorPosX;
				float cursorY = cursor->GetRuntimeData().cursorPosY;

				float toolTipsWidth = toolTipsFrame->GetWidth();
				float toolTipsHeight = toolTipsFrame->GetHeight();

				// Place tool tips slighly to the left of the cursor, and with a little gap down to the cursor
				float toolTipsNewX = cursorX - toolTipsWidth * 0.9f;
				float toolTipsNewY = cursorY - toolTipsHeight - 0.015f * a_menu->ScreenH();

				const float borderSize = a_menu->ScreenH() * 0.01;

				if (toolTipsNewX + toolTipsWidth > a_menu->ScreenW() - borderSize)
					toolTipsNewX = a_menu->ScreenW() - toolTipsWidth - borderSize;
				else if (toolTipsNewX < borderSize)
					toolTipsNewX = borderSize;

				if (toolTipsNewY + toolTipsHeight > a_menu->ScreenH() - borderSize)
					toolTipsNewY = a_menu->ScreenH() - toolTipsHeight - borderSize;
				else if (toolTipsNewY < borderSize)
					toolTipsNewY = borderSize;



				toolTipsGroup->SetPos(toolTipsNewX, toolTipsNewY);
			}

			toolTipsGroup->Show();

		};

		auto HideToolTips = [=]()
		{
			toolTipsGroup->Hide();
		};

		auto MakeToolTipHoverCallback = [=](std::string a_msg) -> std::function<void(float)>
		{
			return [=](float a_duration)
			{
				if (MCM::settings::showToolTips && (a_duration > toolTipsDelay) && !toolTipsGroup->IsVisible())
				{
					ShowToolTips(a_msg);
				}
				else if (a_duration < 0.0f && toolTipsGroup->IsVisible())
				{
					HideToolTips();
				}
			};
		};

		toolTipsGroup->SetInvisible();

	#pragma endregion

	#pragma region YES_NO_BOX

		auto yesNoBox = a_menu->AttachUIElement("YesNoFrame", "YesNoFrame");
		yesNoBox->SetIgnoreMenuLock(true);
		yesNoBox->SetInvisible();
		yesNoBox->SetShowAnimation(yesNoBoxOpenAnimation);
		yesNoBox->SetHideAnimation(yesNoBoxCloseAnimation);

		IElement* yesNoTextField = nullptr;
		IElement* yesButton = nullptr;
		IElement* noButton = nullptr;
		

		auto MakeYesNoBox = [&]
		{
			auto yesNoContentsGroup = yesNoBox->CreateGroup("(YesNoContentsGroup)");


			yesNoTextField = yesNoContentsGroup->CreateTextField("YesNoTextField");
			yesNoTextField->SetFontSize(16);
			yesNoTextField->SetText("Is this real?");
			yesNoTextField->SetFont(skyrimBooksFont);

			auto yesNoButtonsGroup = yesNoContentsGroup->CreateGroup("(YesNoButtonsGroup)");
				
				yesButton = yesNoButtonsGroup->CreateEmptyUIElement("YesButton", ELEMENT_TYPE::kBUTTON);
				noButton = yesNoButtonsGroup->CreateEmptyUIElement("NoButton", ELEMENT_TYPE::kBUTTON);
				auto yesText = yesButton->CreateTextField("layout");
				auto noText = noButton->CreateTextField("layout");

				yesButton->SetHoverAnimation(clickButtonHoverAnimation);
				yesButton->SetPressedAnimation(clickButtonPressedAnimation);
				yesButton->SetResetAnimation(clickButtonResetAnimation);

				noButton->SetHoverAnimation(clickButtonHoverAnimation);
				noButton->SetPressedAnimation(clickButtonPressedAnimation);
				noButton->SetResetAnimation(clickButtonResetAnimation);

				yesText->SetFontSize(16);
				yesText->SetFont(skyrimBooksFont);
				yesText->SetText("EEE");
				
				noText->SetFontSize(16);
				noText->SetFont(skyrimBooksFont);
				noText->SetText("EEE");
				
				yesNoButtonsGroup->AlignChildrenHorizontally(Size(0.1).pw(), CENTERV);
				

				yesText->SetText("Yay");
				noText->SetText("Nay");

			yesNoBox->LockAspect();

			yesNoContentsGroup->AlignChildrenVertically(Size(-0.1f).ph(), CENTERH);
			yesNoContentsGroup->AlignInsideParent(BOTTOM, CENTERH);
			yesNoContentsGroup->SetPadding(BOTTOM, Size(0.08f).ph());
		};

		MakeYesNoBox();

		// add UIFunction parameter or something to openyesnobox, and add it to its set mouse state change
		auto OpenYesNoBox = [=](std::string a_msg, std::function<void(Element*, bool)> a_yesButtonCallback, std::function<void(Element*, bool)> a_noButtonCallback)
		{
			// Must be after MakeYesNoBox(), since otherwise yesNoTextField is null
			yesNoTextField->SetText(a_msg.c_str());
			yesNoBox->Show();
			if (a_yesButtonCallback) yesButton->SetOnChecked(a_yesButtonCallback);
			if (a_noButtonCallback) noButton->SetOnChecked(a_noButtonCallback);
			a_menu->LockMenu();
		};
		auto HideYesNoBox = [=]()
		{
			yesNoBox->Hide();
			a_menu->UnlockMenu();
		};
		yesButton->SetOnChecked([=](bool)
		{
			HideYesNoBox();
		});
		noButton->SetOnChecked([=](bool)
		{
			HideYesNoBox();
		});
		
	#pragma endregion

	#pragma region SELECT_MARKERS_MENU

		auto selectMarkersContainer = a_menu->AttachUIElement("SelectMarkersContainer", "SelectMarkersContainer");
		selectMarkersContainer->LockAspect();
		selectMarkersContainer->SetShowAnimation(openSelectMarkersMenuAnimation);
		selectMarkersContainer->SetHideAnimation(hideSelectMarkersMenuAnimation);

		

		auto MakeMarkerSavePresets = [&]()
		{
			auto markersPresetGroup = selectMarkersContainer->CreateGroup("(SaveMarkersPresetGroup)");
			auto saveMarkersText = markersPresetGroup->CreateTextField("saveMarkersText");
			saveMarkersText->SetText("Save Preset");
			saveMarkersText->SetFontSize(15);
			saveMarkersText->SetFont(skyrimBooksFont);

			auto presetButtonGroup = markersPresetGroup->CreateGroup("(SaveMarkersButtonsGroup)");

			auto preset1 = presetButtonGroup->CreateEmptyUIElement("saveMarkerPreset1", ELEMENT_TYPE::kBUTTON);
			auto preset2 = presetButtonGroup->CreateEmptyUIElement("saveMarkerPreset2", ELEMENT_TYPE::kBUTTON);
			auto preset3 = presetButtonGroup->CreateEmptyUIElement("saveMarkerPreset3", ELEMENT_TYPE::kBUTTON);

			IElement* presets[3] = { preset1, preset2, preset3 };
			IElement* presetTexts[3] = { nullptr, nullptr, nullptr };

			auto CreateSavePresetFunction = [=](uint32_t a_presetIndex) -> std::function<void(Element*, bool)>
			{
				return [=](Element* a_element, bool a_isChecked)
				{
					MCM::DebugMenuPresets::SaveMarkerSettings(a_presetIndex); // 1,2,3
					HideYesNoBox();
				};
			};

			for (int i = 0; i < 3; i++)
			{
				auto preset = presets[i];
				preset->SetHoverAnimation(clickButtonHoverAnimation);
				preset->SetPressedAnimation(clickButtonPressedAnimation);
				preset->SetResetAnimation(clickButtonResetAnimation);

				auto presetText = preset->CreateTextField(IElement::LAYOUT_NAME);
				presetTexts[i] = presetText;
				presetText->SetFontSize(16);
				presetText->SetText("0");
				presetText->SetFont(skyrimBooksFont);

				auto bbox = preset->CreateSquareBBox();
				bbox->UnlockAspect();
				bbox->SetWidth(bbox->GetHeight());
				bbox->AlignInsideParent(CENTERV, CENTERH);

				preset->SetOnChecked([=](bool)
				{
					yesNoBox->AlignOutsideOther(saveMarkersText, TOP, CENTERH);
					OpenYesNoBox("Are you sure?", CreateSavePresetFunction(i+1), nullptr);
				});
			}
			presetButtonGroup->AlignChildrenHorizontally(Size(0.12).pw(), CENTERV);
			markersPresetGroup->AlignChildrenVertically(Size(0.001).ph(), CENTERH);
			markersPresetGroup->AlignInsideParent(BOTTOM, CENTERH);
			markersPresetGroup->SetPadding(BOTTOM, Size(0.002f).ph());
			for (int i = 0; i < 3; i++) presetTexts[i]->SetText(fmt::format("{}", i+1));

		};

		MakeMarkerSavePresets();

		auto selectMarkersMask = selectMarkersContainer->AttachUIElement("MarkersMenuMask");
		selectMarkersMask->AlignInsideParent(CENTERH);
		selectMarkersMask->Move(0.0f, Size(0.015f).ph());

		auto selectMarkersMaskGroup = selectMarkersContainer->CreateGroup("(MarkersMenuMaskGroup)");

		auto MakeMarkerSettingsGroup = [&](MCM::MarkerSettings::ShowMarkerSetting* a_groupSetting)
		{
			auto settingsGroup = selectMarkersMaskGroup->CreateGroup(fmt::format("({}_SettingsGroup)", a_groupSetting->GetInstanceName()));
			
			auto createButtonGroup = [&](MCM::MarkerSettings::ShowMarkerSetting* a_setting, IElement* a_parentElement)
			{
				uint32_t textOffColor = 0xA1A1A1;
				uint32_t textOnColor = 0xFFFFFF;

				auto buttonGroup = a_parentElement->CreateGroup(fmt::format("({}_buttonGroup)", a_setting->GetInstanceName()));

				auto buttonIcon = buttonGroup->AttachUIElement("ArrowButtonIcon", a_setting->GetInstanceName(), ELEMENT_TYPE::kBUTTON);
				auto buttonText = buttonGroup->CreateTextField(fmt::format("{}_TextField", a_setting->GetInstanceName()));
				buttonText->SetText(a_setting->GetLabel());
				buttonText->SetFontSize(16);

				bool isHeader = !a_setting->HasParent();

				if (isHeader)
				{
					buttonText->SetFont(skyrimBooksFont);

				}
				else
				{
					buttonText->SetFont(skyrimFuturaFont);
					buttonText->SetFontColor(textOffColor); // gray;

				}

				buttonIcon->SetHoverAnimation(toggleButtonHoverAnimation);
				buttonIcon->SetPressedAnimation(toggleButtonPressedAnimation);
				buttonIcon->SetResetAnimation(toggleButtonResetAnimation);

				if (isHeader)
				{
					buttonIcon->SetOnChecked([=](Element* a_element, bool a_isChecked)
					{
						DebugMenu::GetDebugMenuHandler()->ResetUpdateTimer();
						DebugMenu::GetMarkerHandler()->HideAllMarkers();

						auto& siblings = a_parentElement->AsUIElement()->children;

						for (int i = 1; i < siblings.size(); i++) // first sibling is header group
						{
							auto* sibling = siblings[i]->AsUIElement();
							auto& siblingButton = sibling->children[0];
							auto& siblingText = sibling->children[1];
							
							if (a_isChecked && siblingButton->IsChecked()) siblingText->SetFontColor(textOnColor);
							else siblingText->SetFontColor(textOffColor);
						}
					});
				}
				else
				{
					buttonIcon->SetOnChecked([=](Element* a_element, bool a_isChecked)
					{
						DebugMenu::GetDebugMenuHandler()->ResetUpdateTimer();
						DebugMenu::GetMarkerHandler()->HideAllMarkers();

						auto& siblings = a_parentElement->AsUIElement()->children;

						auto* headerButton = siblings[0]->AsUIElement()->children[0]->AsUIElement();

						if (!headerButton->IsChecked())
						{
							if (a_isChecked)
							{
								// This will set correct font color
								headerButton->ToggleChecked();
							}
							// If header buttons isn't checked, but sub button was, it now isnt't, but it should
							else
							{
								// this will recursively call this function again, which will toggle header, 
								// setting the right color on this setting, and return
								a_element->ToggleChecked(); 
							}
						}
						else if(a_isChecked)
						{
							auto& buttonTextField = a_element->parent->children[2];
							buttonText->SetFontColor(textOnColor);
						}
						else
						{
							buttonText->SetFontColor(textOffColor);

							uint32_t numberOfCheckedSiblings = 0;
							for (int i = 1; i < siblings.size(); i++) // first sibling is header group
							{
								auto* sibling = siblings[i]->AsUIElement();
								auto& siblingButton = sibling->children[0];
								if (siblingButton->IsChecked()) numberOfCheckedSiblings++;
							}

							if (numberOfCheckedSiblings == 0 && headerButton->IsChecked())
							{
								headerButton->ToggleChecked();
							}
						}
					});
				}
				
				//buttonIcon->SetOnMouseStateChange([=](Element* a_element, MOUSE_STATE a_state, bool a_sat)
				//{
				//	//This code is kinda spaghetti
				//	switch (a_state)
				//	{
				//		case MOUSE_STATE::kCHECKED:
				//		{
				//			DebugMenu::GetDebugMenuHandler()->ResetUpdateTimer();
				//			if (!a_sat)
				//			{
				//				MCM::settings::updateVisibleMarkers = true;
				//			}

				//			if (isHeader) // set subsettings grey when turning off
				//			{
				//				auto* settingsGroupGroup = a_element->AsUIElement()->parent->parent;
				//				settingsGroupGroup->TraverseChildren([&](Element* a_child)
				//				{
				//					if (a_child->IsTextfield())
				//					{
				//						bool isRelatedButtonChecked = false;
				//						for (auto& child : a_child->parent->children)
				//						{
				//							if (child->IsChecked()) 
				//							{
				//								isRelatedButtonChecked = true;
				//								break;
				//							}
				//						}

				//						if (isRelatedButtonChecked)
				//						{
				//							if (a_sat) a_child->SetFontColor(textOnColor);
				//							else if (!a_sat) a_child->SetFontColor(textOffColor);
				//						}
				//					}
				//				});
				//			}
				//			else // Set textcolor gray when turning off
				//			{
				//				auto* parentGroup = a_element->AsUIElement()->parent;
				//				for (auto& child : parentGroup->children)
				//				{
				//					if (child->AsUIElement()->IsTextfield())
				//					{
				//						if (a_sat) child->SetFontColor(textOnColor);
				//						else child->SetFontColor(textOffColor);
				//					}
				//				}

				//				bool areSiblingsSat = false;
				//				for (const auto& child : a_setting->GetParent()->GetSubSettings())
				//				{
				//					if (child->IsEnabled())
				//					{
				//						areSiblingsSat = true;
				//						break;
				//					}
				//				}

				//				auto* header = parentGroup->parent->children[0]->AsUIElement()->children[0]->AsUIElement();
				//				// If header is not sat, and sub setting is sat, set header
				//				if (a_sat && !header->IsChecked()) 
				//				{
				//					header->ToggleChecked();
				//					header->PlayResetAnimation();
				//				}
				//				// If header is sat, and sub setting is unsat, and no other sub settings are sat, unset header
				//				else if (!a_sat && !areSiblingsSat && header->IsChecked())
				//				{
				//					header->ToggleChecked();
				//					header->PlayResetAnimation();
				//				}
				//				// If header is not sat, and sub setting was sat before header was unset, a click on sub setting
				//				// will unset the sub setting, ie. a_sat == false. However, we want this to turn on the sub setting
				//				// so we set the sub setting
				//				else if (!a_sat && !header->IsChecked())
				//				{
				//					a_element->ToggleChecked();
				//					a_element->PlayResetAnimation();
				//				}
				//			}


				//			break;
				//		}
				//	}
				//});
				buttonIcon->SetToggleValuePtr(a_setting->GetPtr());
				if (!isHeader && 
					buttonIcon->IsChecked() && 
					a_parentElement->GetChildren()[0]->GetChildren()[0]->IsChecked()) // Header button
				{
					buttonText->SetFontColor(textOnColor);
				}

				auto bbox = buttonIcon->CreateSquareBBox();
				bbox->UnlockAspect();
				bbox->SetWidth(0.8f*selectMarkersContainer->GetWidth());
				bbox->SetYScale(1.2f);
				bbox->AlignInsideParent(CENTERV);

				buttonGroup->AlignChildrenHorizontally(0, CENTERV);
				if (!isHeader) 
				{
					buttonGroup->Move(buttonIcon->GetWidth(), 0.0f);
					bbox->Move(-buttonIcon->GetWidth(), 0.0f);
				}

			};

			createButtonGroup(a_groupSetting, settingsGroup); // Headers
			for (const auto& subSetting : a_groupSetting->GetSubSettings()) 
			{
				createButtonGroup(subSetting.get(), settingsGroup); // sub settings
			}


			settingsGroup->AlignChildrenVertically(-5.0f, Alignment::kNone);

		};

		for (auto& markerSetting : MCM::MSettings()->markerGroupSettings)
		{
			MakeMarkerSettingsGroup(markerSetting.get());
		}
		
		selectMarkersMaskGroup->AlignChildrenVertically(Size(0.01f).ph(), LEFT);
		selectMarkersMaskGroup->SetVerticalScrollable(true);
		selectMarkersMaskGroup->SetScrollableArea(selectMarkersMask);
		selectMarkersMaskGroup->AlignInsideParent(LEFT);
		selectMarkersMaskGroup->SetY(selectMarkersMask->GetHeight() * 0.03 + selectMarkersMask->GetY()); // works because they are in the same frame
		selectMarkersMaskGroup->SetScrollableTopRatio(0.03);
		selectMarkersMaskGroup->SetPadding(LEFT, Size(0.1).pw());
		
		selectMarkersMaskGroup->SetMask(*selectMarkersMask);
		selectMarkersContainer->SetInvisible();

	#pragma endregion

	#pragma region MAIN_MENU

		auto mainMenuContainer = a_menu->AttachUIElement("MainMenuContainer", "MainMenuContainer");
		mainMenuContainer->AlignInsideParent(CENTERV, RIGHT);
		mainMenuContainer->SetPadding(RIGHT, Size(0.02f).vw());
		mainMenuContainer->LockAspect();
		mainMenuContainer->SetHeight(Size(0.8f).vh());
		mainMenuContainer->SetShowAnimation(openMainMenuAnimation);
		mainMenuContainer->SetHideAnimation(hideMainMenuAnimation);

		IElement* TFCFrame = nullptr;

		selectMarkersContainer->SetScale(mainMenuContainer->GetYScale());
		selectMarkersContainer->AlignOutsideOther(mainMenuContainer, LEFT);
		selectMarkersContainer->SetPadding(RIGHT, Size(0.001).vw());
		selectMarkersContainer->Move(0.0f, Size(0.114).vh());

		yesNoBox->SetScale(mainMenuContainer->GetYScale());

		auto MakeDayNightIcon = [&]()
		{
			auto dayNightIcon = mainMenuContainer->AttachUIElement("DayNightIcon", ELEMENT_TYPE::kBUTTON);
			dayNightIcon->SetHoverAnimation(cyclicButtonHoverAnimation);
			dayNightIcon->SetPressedAnimation(cyclicButtonPressedAnimation);
			dayNightIcon->SetResetAnimation(cyclicButtonResetAnimation);
			dayNightIcon->SetCyclicValuePtr(&MCM::settings::dayNightIndex);
			//dayNightIcon->PrintOrderOfCyclicImages();
			dayNightIcon->SetOnMouseStateChange(updateDebugMenuCallback);
			dayNightIcon->AlignInsideParent(CENTERH, TOP);
			dayNightIcon->SetPadding(TOP, Size(0.007).ph());
			dayNightIcon->SetWhileHoverCallback(MakeToolTipHoverCallback(dayNightToolTip));

			auto bbox = dayNightIcon->CreateSquareBBox();
			bbox->LockAspect();
			bbox->AlignInsideParent(CENTERH, CENTERV);
			bbox->SetScale(1.4f);
		};
		auto MakeCloseMenuButton = [&]()
		{
			auto closeButton = mainMenuContainer->AttachUIElement("CloseMenuIcon", ELEMENT_TYPE::kBUTTON);
			closeButton->SetScale(1.3f);
			closeButton->SetHoverAnimation(invisibleClickButtonHoverAnimation);
			closeButton->SetPressedAnimation(invisibleClickButtonPressedAnimation);
			closeButton->SetResetAnimation(invisibleClickButtonResetAnimation);
			closeButton->AlignInsideParent(TOP, RIGHT);
			closeButton->SetPadding(RIGHT, -closeButton->GetWidth()/3);
			closeButton->SetWhileHoverCallback(MakeToolTipHoverCallback(closeMenuToolTip));

			auto bbox = closeButton->CreateSquareBBox();
			bbox->LockAspect();
			bbox->SetScale(1.2f);
			bbox->AlignInsideParent(CENTERH, CENTERV);
			
			setLayoutVisibleStatus(closeButton, false);

			closeButton->SetOnChecked([&](bool)
			{
				DebugMenu::GetDebugMenuHandler()->CloseAndReset();
			});
		};
		auto MakeTFCButtons = [&]()
		{
			TFCFrame = mainMenuContainer->AttachUIElement("TFCFrame");
			TFCFrame->SetWidth(Size(0.95f).pw());
			TFCFrame->AlignOutsideParent(BOTTOM, CENTERH);

			auto TFCGroup = TFCFrame->CreateGroup("(TFCGroup)");

			auto tfcButton = TFCGroup->CreateEmptyUIElement("TFC", ELEMENT_TYPE::kBUTTON);
			auto tfcLayout = tfcButton->CreateEmptyUIElement(IElement::LAYOUT_NAME);
			auto tfcOnText = tfcLayout->CreateTextField(IElement::TOGGLE_ON_NAME);
			auto tfcOffText = tfcLayout->CreateTextField(IElement::TOGGLE_OFF_NAME);
			tfcOnText->SetText("TFC");
			tfcOffText->SetText("TFC");
			tfcOnText->SetFont(skyrimBooksFont);
			tfcOffText->SetFont(skyrimBooksFont);

			tfcOffText->SetFontColor(0x999999);

			if (MCM::settings::enableFreeCamOnOpen)
				tfcButton->AsUIElement()->AsButton()->SetVisibleOnToggleImage();
			else
				tfcButton->AsUIElement()->AsButton()->SetVisibleOffToggleImage();


			auto zLockButton = TFCGroup->AttachUIElement("ZLock", ELEMENT_TYPE::kBUTTON);
			auto doubleAscendButton = TFCGroup->AttachUIElement("DoubleAscend", ELEMENT_TYPE::kBUTTON);
			auto followPlayerButton = TFCGroup->AttachUIElement("FollowPlayer", ELEMENT_TYPE::kBUTTON);

			IElement* buttonIcons[4] = { tfcButton, zLockButton, doubleAscendButton, followPlayerButton };
			
			

			tfcButton->SetHeight(Size(0.9f).ph());
			zLockButton->SetHeight(Size(0.5f).ph());
			doubleAscendButton->SetHeight(Size(0.5f).ph());
			followPlayerButton->SetWidth(Size(0.6f).ph());

			for (int i = 0; i < 4; i++)
			{
				auto buttonIcon = buttonIcons[i];

				buttonIcon->SetHoverAnimation(clickButtonHoverAnimation);
				buttonIcon->SetPressedAnimation(clickButtonPressedAnimation);
				buttonIcon->SetResetAnimation(clickButtonResetAnimation);

				
				const float bboxScale = 1.6f;
				auto bbox = buttonIcon->CreateSquareBBox();

				bool isWidthBiggerThanHeight = buttonIcon->GetWidth() > buttonIcon->GetHeight();
					
				bbox->UnlockAspect();

				if (i != 0)
				{
					if (isWidthBiggerThanHeight)
					{
						bbox->SetHeight(Size(bboxScale).pw());
						bbox->SetXScale(bboxScale);
					}
					else
					{
						bbox->SetWidth(Size(bboxScale).ph());
						bbox->SetYScale(bboxScale);
					}
					
				}
				else
				{
					bbox->SetXScale(1.3);
				}

				bbox->AlignInsideParent(CENTERV, CENTERH);
			}


			TFCGroup->AlignChildrenHorizontally(Size(0.1f).pw(), CENTERV); // aligns children vertically, x's are sat below
			TFCGroup->AlignInsideParent(CENTERV);

			tfcButton->SetX(Size(73.5 - tfcButton->GetWidth()/2).local());
			zLockButton->SetX(Size(160.0f - zLockButton->GetWidth() / 2).local());
			doubleAscendButton->SetX(Size(225.0f - doubleAscendButton->GetWidth() / 2).local());
			followPlayerButton->SetX(Size(290.0f - followPlayerButton->GetWidth() / 2).local());

			zLockButton->SetToggleValuePtr(&MCM::settings::lockFreeCamToZPlane);
			doubleAscendButton->SetToggleValuePtr(&MCM::settings::useDoubleAscendToFly);
			followPlayerButton->SetToggleValuePtr(&MCM::settings::playerFollowsCamera);

			SetTFCButtonCorrectOnOffIcon = [=]()
			{
				auto uiTask = [=]
				{
					auto g_freeCamHandler = FreeCamHandler::GetSingleton();

					if (g_freeCamHandler->IsCustomFreeCamEnabled())
					{
						tfcButton->AsUIElement()->AsButton()->SetVisibleOnToggleImage();
					}
					else
					{
						tfcButton->AsUIElement()->AsButton()->SetVisibleOffToggleImage();
					}
				};
				SKSE::GetTaskInterface()->AddUITask(uiTask);
			};

			tfcButton->SetOnChecked([](IElement* a_this, bool)
			{
				if (!RE::UI::GetSingleton()->GameIsPaused())
				{
					FreeCamHandler::GetSingleton()->ToggleTFC();
				}
			});
			zLockButton->SetOnChecked([](bool a_isChecked)
			{
				if (auto playerCam = RE::PlayerCamera::GetSingleton())
				{
					auto tfcHandler = FreeCamHandler::GetSingleton();
					if (playerCam->IsInFreeCameraMode() && tfcHandler->IsCustomFreeCamEnabled())
					{
						if (auto freeCam = tfcHandler->GetFreeCamera())
						{
							freeCam->lockToZPlane = a_isChecked;
						}
					}
				}
			});
			followPlayerButton->SetOnChecked([](bool a_isChecked)
			{
				if (auto playerCam = RE::PlayerCamera::GetSingleton())
				{
					auto tfcHandler = FreeCamHandler::GetSingleton();
					if (playerCam->IsInFreeCameraMode() && tfcHandler->IsCustomFreeCamEnabled())
					{
						tfcHandler->PutPlayerBackOnGround();
					}
				}
			});

			zLockButton->SetWhileHoverCallback(MakeToolTipHoverCallback(zLockToolTip));
			doubleAscendButton->SetWhileHoverCallback(MakeToolTipHoverCallback(doubleAscendToolTip));
			followPlayerButton->SetWhileHoverCallback(MakeToolTipHoverCallback(followPlayerToolTip));

		};


		auto mainMenuContainerMask = mainMenuContainer->AttachUIElement("MainContainerMask", "MainContainerMask");
		mainMenuContainerMask->AlignInsideParent(CENTERH);
		mainMenuContainerMask->Move(0.0f, Size(0.055f).ph());

		auto mainMenuMaskGroup = mainMenuContainer->CreateGroup("(MainMenuMaskGroup)");
		mainMenuMaskGroup->SetVerticalScrollable(true);
		mainMenuMaskGroup->SetScrollableArea(mainMenuContainerMask);

		auto MakeTitle = [&](IElement* a_parent, std::string a_elementPrefix, std::string a_title) -> IElement*
		{
			auto title = a_parent->CreateTextField(a_elementPrefix + "Title"s);
			title->SetText(a_title.c_str());
			title->SetFontColor(0xFFFFFF);
			title->SetFont(skyrimBooksFont);
			title->SetFontSize(14);
			return title;
		};
		auto MakeRangeGroup = [&](IElement* a_parent, std::string a_elementPrefix, float* a_rangeSetting)
		{
			if (!a_rangeSetting) 
			{
				logger::debug("Forgot to specify range setting");
				return;
			}

			auto rangeGroup = a_parent->CreateGroup("("s + a_elementPrefix + "RangeGroup)"s);
			auto minusIcon = rangeGroup->AttachUIElement("MinusIcon"s, a_elementPrefix + "MinusIcon"s, ELEMENT_TYPE::kBUTTON);
			auto rangeText = rangeGroup->CreateTextField(a_elementPrefix + "RangeText"s);
			auto plusIcon = rangeGroup->AttachUIElement("PlusIcon", a_elementPrefix + "PlusIcon"s, ELEMENT_TYPE::kBUTTON);

			rangeText->SetText("00000"); // force the textbox to have a width of 5 wide digits before aligning
			rangeText->SetFont(skyrimFuturaFont);
			rangeText->SetFontSize(14);
			rangeText->SetTextAlign("center");
			rangeText->SetAutoSize(false);

			rangeGroup->AlignChildrenHorizontally(0.0f, CENTERV);

			rangeText->SetText(fmt::format("{:.0f}", *a_rangeSetting));


			minusIcon->SetHoverAnimation(clickButtonHoverAnimation);
			minusIcon->SetPressedAnimation(clickButtonPressedAnimation);
			minusIcon->SetResetAnimation(clickButtonResetAnimation);
			minusIcon->SetOnMouseStateChange([=](Element* a_element, MOUSE_STATE a_state, bool)
			{
				switch (a_state)
				{
					case MOUSE_STATE::kCHECKED:
					{	
						DebugMenu::GetDebugMenuHandler()->ResetUpdateTimer();
						float minRange = MCM::settings::minRange;
						float step = MCM::settings::rangeStep;

						*a_rangeSetting = std::max(*a_rangeSetting - step, minRange);
						rangeText->SetText(fmt::format("{:.0f}", *a_rangeSetting));

						break;
					}
				}
			});
			plusIcon->SetHoverAnimation(clickButtonHoverAnimation);
			plusIcon->SetPressedAnimation(clickButtonPressedAnimation);
			plusIcon->SetResetAnimation(clickButtonResetAnimation);
			plusIcon->SetOnMouseStateChange([=](Element* a_element, MOUSE_STATE a_state, bool)
			{
				switch (a_state)
				{
					case MOUSE_STATE::kCHECKED:
					{
						DebugMenu::GetDebugMenuHandler()->ResetUpdateTimer();
						float maxRange = MCM::settings::maxRange;
						float step = MCM::settings::rangeStep;

						*a_rangeSetting = std::min(*a_rangeSetting + step, maxRange);
						rangeText->SetText(fmt::format("{:.0f}", *a_rangeSetting));

						break;
					}
				}
			});

			minusIcon->SetWhileHoverCallback(MakeToolTipHoverCallback(decreaseRangeToolTip));
			plusIcon->SetWhileHoverCallback(MakeToolTipHoverCallback(increaseRangeToolTip));

			auto minusBBox = minusIcon->CreateSquareBBox();
			auto plusBBox = plusIcon->CreateSquareBBox();
			minusBBox->SetScale(1.4);
			plusBBox->SetScale(1.4);
			minusBBox->AlignInsideParent(CENTERV, CENTERH);
			plusBBox->AlignInsideParent(CENTERV, CENTERH);

		};
		auto MakeSubButtons = [&](IElement* a_parent, std::string a_elementPrefix, std::vector<std::string> a_buttons, std::vector<bool*> a_boolSettings)
		{
			auto buttonsGroup = a_parent->CreateGroup("("s + a_elementPrefix + ")"s);

			for (int i = 0; i < a_buttons.size(); i++)
			{
				std::string prefix = Utils::RemoveWhitespaceFromString(a_buttons[i]);
				auto group_i = buttonsGroup->CreateGroup("("s + prefix + "Group)"s);
				auto button_i = group_i->AttachUIElement("ArrowButtonIcon", prefix + "ArrowButtonIcon", ELEMENT_TYPE::kBUTTON);
				button_i->SetHoverAnimation(toggleButtonHoverAnimation);
				button_i->SetPressedAnimation(toggleButtonPressedAnimation);
				button_i->SetResetAnimation(toggleButtonResetAnimation);
				button_i->SetToggleValuePtr(a_boolSettings[i]);
				button_i->SetOnMouseStateChange(updateDebugMenuCallback);

				auto text_i = group_i->CreateTextField(prefix + "Text");
				text_i->SetText(a_buttons[i]);
				text_i->SetFont(skyrimFuturaFont);
				text_i->SetFontSize(subElementFontSize);

				auto bbox = button_i->CreateSquareBBox();
				bbox->UnlockAspect();
				float bboxScale = 1.2;
				float fieldWidth = (button_i->GetWidth() + text_i->GetWidth());
				bbox->SetWidth(fieldWidth*bboxScale);
				bbox->SetHeight(text_i->GetHeight()*0.8);
				bbox->AlignInsideParent(LEFT, CENTERV);
				// if button+text is scaled by 120%, move the bbox (1-1.2)/2 = 10% to the left
				bbox->SetPadding(LEFT, (1-bboxScale)/2*fieldWidth); 

				group_i->AlignChildrenHorizontally(0, CENTERV);
			}
			buttonsGroup->AlignChildrenVertically(mainMenuSmallElementsSpacing, LEFT);
		};
		auto MakeCellBorderGroup = [&]()
		{
			auto cellBordersGroup = mainMenuMaskGroup->CreateGroup("(CellBordersGroup)");

			MakeTitle(cellBordersGroup, "CellBorders", "Cell Borders");

			auto cellBordersIcon = cellBordersGroup->AttachUIElement("CellBorderIcon", ELEMENT_TYPE::kBUTTON);
			cellBordersIcon->SetHoverAnimation(toggleButtonHoverAnimation);
			cellBordersIcon->SetPressedAnimation(toggleButtonPressedAnimation);
			cellBordersIcon->SetResetAnimation(toggleButtonResetAnimation);
			cellBordersIcon->SetToggleValuePtr(&MCM::settings::showCellBorders);
			cellBordersIcon->SetOnMouseStateChange(updateDebugMenuCallback);

			MakeSubButtons(cellBordersGroup, "CellBorders", { "Walls"s, "Quads"s }, { &MCM::settings::showCellWalls, &MCM::settings::showCellQuads });

			cellBordersGroup->AlignChildrenVertically(mainMenuGroupsSpacing, CENTERH);
		};

		auto MakeNavmeshGroup = [&]()
		{
			auto navmeshGroup = mainMenuMaskGroup->CreateGroup("(NavmeshGroup)");

			auto navmeshTitle = MakeTitle(navmeshGroup, "Navmesh", "Navmesh");

			auto navmeshModeIcon = navmeshGroup->AttachUIElement("NavMeshModeIcon", ELEMENT_TYPE::kBUTTON);
			navmeshModeIcon->SetHoverAnimation(cyclicButtonHoverAnimation);
			navmeshModeIcon->SetPressedAnimation(cyclicButtonPressedAnimation);
			navmeshModeIcon->SetResetAnimation(cyclicButtonResetAnimation);
			navmeshModeIcon->SetCyclicValuePtr(&MCM::settings::navmeshModeIndex);
			//navmeshModeIcon->PrintOrderOfCyclicImages();
			navmeshModeIcon->SetOnMouseStateChange([&](Element* a_element, MOUSE_STATE a_state, bool)
			{
				switch (a_state)
				{
					case MOUSE_STATE::kCHECKED:
					{
						DebugMenu::GetDebugMenuHandler()->ResetUpdateTimer();
						DebugMenu::GetCollisionHandler()->HideAllCollisions();
						break;
					}
				}
			});
			

			navmeshModeIcon->SetWhileHoverCallback(MakeToolTipHoverCallback(navmeshModeToolTip));

			auto navmeshIcon = navmeshGroup->AttachUIElement("NavMeshIcon", "NavMeshIcon", ELEMENT_TYPE::kBUTTON);
			navmeshIcon->SetHoverAnimation(toggleButtonHoverAnimation);
			navmeshIcon->SetPressedAnimation(toggleButtonPressedAnimation);
			navmeshIcon->SetResetAnimation(toggleButtonResetAnimation);
			navmeshIcon->SetToggleValuePtr(&MCM::settings::showNavmesh);
			navmeshIcon->SetOnMouseStateChange(updateDebugMenuCallback);

			MakeRangeGroup(navmeshGroup, "Navmesh", &MCM::settings::navmeshRange);

			MakeSubButtons(navmeshGroup, "Navmesh", { "Triangles"s, "Cover"s }, { &MCM::settings::showNavmeshTriangles, &MCM::settings::showNavmeshCover });

			navmeshGroup->AlignChildrenVertically(mainMenuGroupsSpacing, CENTERH);
			navmeshTitle->Move(0.0f, Size(0.01).ph());
		};
		auto MakeBoxesGroup = [&]()
		{
			auto boxesGroup = mainMenuMaskGroup->CreateGroup("(BoxesGroup)");

			MakeTitle(boxesGroup, "Boxes", "Boxes");

			auto boxesIcon = boxesGroup->AttachUIElement("OcclusionIcon", ELEMENT_TYPE::kBUTTON);
			boxesIcon->SetHoverAnimation(toggleButtonHoverAnimation);
			boxesIcon->SetPressedAnimation(toggleButtonPressedAnimation);
			boxesIcon->SetResetAnimation(toggleButtonResetAnimation);
			boxesIcon->SetToggleValuePtr(&MCM::settings::showBoxes);
			boxesIcon->SetOnMouseStateChange(updateDebugMenuCallback);

			MakeRangeGroup(boxesGroup, "Boxes", &MCM::settings::boxesRange);
			MakeSubButtons(boxesGroup, "Boxes", { "Occlusion"s, "Collision"s }, { &MCM::settings::showOcclusion, &MCM::settings::showCollisionMarkers });


			boxesGroup->AlignChildrenVertically(mainMenuGroupsSpacing, CENTERH);

		};
		auto MakeOverlayGroup = [&]()
		{
			auto overlayGroup = mainMenuMaskGroup->CreateGroup("(PositionGroup)");

			auto overlayButton = overlayGroup->AttachUIElement("ArrowButtonIcon", "PositionArrowButtonIcon", ELEMENT_TYPE::kBUTTON);
			overlayButton->SetHoverAnimation(toggleButtonHoverAnimation);
			overlayButton->SetPressedAnimation(toggleButtonPressedAnimation);
			overlayButton->SetResetAnimation(toggleButtonResetAnimation);
			overlayButton->SetToggleValuePtr(&MCM::settings::showCoordinates);
			//overlayButton->SetOnChecked([=](bool a_isChecked)
			//{
			//	if (a_isChecked) ShowOverlay();
			//	else HideOverlay();
			//});
			//overlayButton->SendCheckedEventWhenMenuOpens(true);

			auto overlayText = overlayGroup->CreateTextField("OverlayText");
			overlayText->SetText("Position");
			overlayText->SetFont(skyrimBooksFont);
			overlayText->SetFontSize(14);

			overlayGroup->AlignChildrenHorizontally(0, CENTERV);

			auto bbox = overlayButton->CreateSquareBBox();
			auto bboxScale = 1.2;
			auto bboxWidth = (overlayButton->GetWidth() + overlayText->GetWidth())*bboxScale;
			bbox->UnlockAspect();
			bbox->SetWidth(bboxWidth);
			bbox->SetYScale(2);
			
			bbox->AlignInsideOther(overlayGroup, CENTERV, CENTERH);



		};
		auto MakeMarkersGroup = [&]()
		{
			auto markersGroup = mainMenuMaskGroup->CreateGroup("(MarkersGroup)");

			MakeTitle(markersGroup, "Markers", "Markers");

			auto markersIcon = markersGroup->AttachUIElement("MarkersIcon", ELEMENT_TYPE::kBUTTON);
			markersIcon->SetHoverAnimation(toggleButtonHoverAnimation);
			markersIcon->SetPressedAnimation(toggleButtonPressedAnimation);
			markersIcon->SetResetAnimation(toggleButtonResetAnimation);
			markersIcon->SetToggleValuePtr(&MCM::settings::showMarkers);
			markersIcon->SetOnMouseStateChange([&](Element* a_element, MOUSE_STATE a_state, bool a_sat)
			{
				switch(a_state)
				{
					case MOUSE_STATE::kCHECKED:
					{
						DebugMenu::GetDebugMenuHandler()->ResetUpdateTimer();
						DebugMenu::GetMarkerHandler()->HideAllMarkers();
						break;
					}
				}

			});
			

			MakeRangeGroup(markersGroup, "Markers", &MCM::settings::markersRange);


			auto selectMarkersButton = markersGroup->CreateEmptyUIElement("SelectMarkersButton", ELEMENT_TYPE::kBUTTON);
			selectMarkersButton->SetHoverAnimation(clickButtonHoverAnimation);
			selectMarkersButton->SetPressedAnimation(clickButtonPressedAnimation);
			selectMarkersButton->SetResetAnimation(clickButtonResetAnimation);
			selectMarkersButton->SetOnMouseStateChange([=](Element*, MOUSE_STATE a_state, bool a_sat)
			{
				switch (a_state)
				{
					case MOUSE_STATE::kCHECKED:
					{
						selectMarkersContainer->ToggleShowHide();
						MCM::DebugMenuPresets::SaveMarkerSettings(0);
						break;
					}
				}
			});
			
			auto selectMarkersText = selectMarkersButton->CreateTextField(IElement::LAYOUT_NAME);
			selectMarkersText->SetText("Select Markers");
			selectMarkersText->SetFont(skyrimFuturaFont);
			selectMarkersText->SetFontSize(subElementFontSize);
			auto selectBBox = selectMarkersButton->CreateSquareBBox();
			selectBBox->UnlockAspect();
			selectBBox->SetXScale(1.2);
			selectBBox->AlignInsideParent(CENTERH, CENTERV);

			auto markersPresetGroup = markersGroup->CreateGroup("(LoadMarkersPresetGroup)");
			auto preset1 = markersPresetGroup->CreateEmptyUIElement("loadMarkerPreset1", ELEMENT_TYPE::kBUTTON);
			markersPresetGroup->AttachUIElement("TaperedLine", "loadMarkerPresetsTaperedLine1");
			auto preset2 = markersPresetGroup->CreateEmptyUIElement("loadMarkerPreset2", ELEMENT_TYPE::kBUTTON);
			markersPresetGroup->AttachUIElement("TaperedLine", "loadMarkerPresetsTaperedLine2");
			auto preset3 = markersPresetGroup->CreateEmptyUIElement("loadMarkerPreset3", ELEMENT_TYPE::kBUTTON);

			IElement* presets[3] = { preset1, preset2, preset3 };
			IElement* presetTexts[3] = {nullptr, nullptr, nullptr};

			auto CreateLoadPresetFunction = [=](uint32_t a_presetIndex) -> std::function<void(Element*, bool)>
			{
				return [=](Element* a_element, bool a_isChecked)
				{
					MCM::DebugMenuPresets::LoadMarkerSettings(a_presetIndex);

					// Update select markers buttons visually
					for (auto& settingGroup : selectMarkersMaskGroup->GetChildren())
					{
						auto& settings = settingGroup->GetChildren();
						auto* headerButton = settings[0]->GetChildren()[0]->AsUIElement();

						bool isHeaderEnabled = headerButton->AsButton()->IsToggleSettingTrue();

						for (int i = 1; i < settings.size(); i++) // skip header settings
						{
							auto* settingButton = settings[i]->GetChildren()[0]->AsUIElement();
							if (settingButton->AsButton()->IsToggleSettingTrue() != settingButton->IsChecked())
							{
								settingButton->ToggleChecked();
							}
						}
						if (isHeaderEnabled != headerButton->IsChecked()) headerButton->ToggleChecked();
					}

					HideYesNoBox();
				};
			};

			for (int i = 0; i < 3; i++)
			{
				auto preset = presets[i];
				preset->SetHoverAnimation(clickButtonHoverAnimation);
				preset->SetPressedAnimation(clickButtonPressedAnimation);
				preset->SetResetAnimation(clickButtonResetAnimation);

				auto presetText = preset->CreateTextField(IElement::LAYOUT_NAME);
				presetTexts[i] = presetText;

				presetText->SetFontSize(16);

				// Set same text so the alignment works correctly
				presetText->SetText("0");

				presetText->SetFont(skyrimBooksFont);

				auto bbox = preset->CreateSquareBBox();

				bbox->UnlockAspect();
				bbox->SetWidth(bbox->GetHeight());
				bbox->AlignInsideParent(CENTERV, CENTERH);

				preset->SetOnChecked([=](bool)
				{
					yesNoBox->AlignOutsideOther(markersPresetGroup, TOP, CENTERH);
					OpenYesNoBox("Load Preset?", CreateLoadPresetFunction(i+1), nullptr);
				});
			}

			markersPresetGroup->AlignChildrenHorizontally(Size(0.06).pw(), CENTERV);

			for (int i = 0; i < 3; i++) presetTexts[i]->SetText(fmt::format("{}", i+1));

			markersGroup->AlignChildrenVertically(mainMenuGroupsSpacing, CENTERH);

		};
		auto MakeCollisionsGroup = [&]()
		{
			auto collisionGroup = mainMenuMaskGroup->CreateGroup("(CollisionGroup)");

			auto collisionTitle = MakeTitle(collisionGroup, "Collision", "Collision");

			auto collisionDisplay = collisionGroup->AttachUIElement("CollisionDisplayModeIcon", ELEMENT_TYPE::kBUTTON);
			collisionDisplay->SetHoverAnimation(cyclicButtonHoverAnimation);
			collisionDisplay->SetPressedAnimation(cyclicButtonPressedAnimation);
			collisionDisplay->SetResetAnimation(cyclicButtonResetAnimation);
			collisionDisplay->SetCyclicValuePtr(&MCM::settings::collisionDisplayIndex);
			collisionDisplay->SetOnMouseStateChange([&](Element* a_element, MOUSE_STATE a_state, bool)
			{
				switch (a_state)
				{
					case MOUSE_STATE::kCHECKED:
					{
						DebugMenu::GetDebugMenuHandler()->ResetUpdateTimer();
						DebugMenu::GetCollisionHandler()->HideAllCollisions();
						break;
					}
				}
			});
			collisionDisplay->SetWhileHoverCallback(MakeToolTipHoverCallback(collisionModeToolTip));

			auto collisionIcon = collisionGroup->AttachUIElement("CollisionIcon", ELEMENT_TYPE::kBUTTON);
			collisionIcon->SetHoverAnimation(toggleButtonHoverAnimation);
			collisionIcon->SetPressedAnimation(toggleButtonPressedAnimation);
			collisionIcon->SetResetAnimation(toggleButtonResetAnimation);
			collisionIcon->SetToggleValuePtr(&MCM::settings::showCollision);
			collisionIcon->SetOnMouseStateChange(updateDebugMenuCallback);

			if (!MCM::settings::useD3D)
			{
				collisionGroup->BlockChildInteractions();
				auto d3dWarning = collisionIcon->AttachUIElement("D3D11Warning");
				d3dWarning->AlignInsideParent(CENTERV, CENTERH);
				if (collisionIcon->IsChecked()) collisionIcon->AsUIElement()->ToggleChecked();
			}


			MakeRangeGroup(collisionGroup, "Collision", &MCM::settings::collisionRange);

			auto clearSelectionButton = collisionGroup->CreateEmptyUIElement("ClearSelectionButton", ELEMENT_TYPE::kBUTTON);
			auto clearSelectionText = clearSelectionButton->CreateTextField(IElement::LAYOUT_NAME);
			clearSelectionText->SetFont(skyrimFuturaFont);
			clearSelectionText->SetFontSize(16);
			clearSelectionText->SetText("Clear Selection");
			auto* bbox = clearSelectionButton->CreateSquareBBox();
			bbox->UnlockAspect();
			bbox->SetXScale(1.3f);
			bbox->AlignInsideParent(CENTERV, CENTERH);

			clearSelectionButton->SetHoverAnimation(clickButtonHoverAnimation);
			clearSelectionButton->SetPressedAnimation(clickButtonPressedAnimation);
			clearSelectionButton->SetResetAnimation(clickButtonResetAnimation);

			clearSelectionButton->SetOnChecked([&](bool)
			{
				DebugMenu::GetCollisionHandler()->ResetSelectedRefs();
			});

			MakeSubButtons(collisionGroup, "Collision", { "Occlude"s, "Character\nController" }, { &MCM::settings::collisionOcclude, &MCM::settings::showCharController });


			collisionGroup->AlignChildrenVertically(mainMenuGroupsSpacing, CENTERH);

			collisionTitle->Move(0.0f, Size(0.01).ph());

		};	

		MakeDayNightIcon();
		MakeCloseMenuButton();
		MakeTFCButtons();

		MakeCellBorderGroup();
		MakeNavmeshGroup();
		MakeBoxesGroup();
		MakeMarkersGroup();
		MakeOverlayGroup();
		MakeCollisionsGroup();


		mainMenuMaskGroup->AlignChildrenVertically(Size(0.01).ph(), CENTERH);

		mainMenuMaskGroup->AlignInsideParent(CENTERH);
		mainMenuMaskGroup->SetY(mainMenuContainerMask->GetHeight() * 0.02 + mainMenuContainerMask->GetY());
		mainMenuMaskGroup->SetScrollableTopRatio(0.02);
		
		mainMenuMaskGroup->SetMask(*mainMenuContainerMask);

	#pragma endregion

		yesNoBox->MoveToFront();
		toolTipsGroup->MoveToFront();

		yesNoBox->SetLayoutAlpha(containerAlpha);
		selectMarkersContainer->SetLayoutAlpha(containerAlpha);
		mainMenuContainer->SetLayoutAlpha(containerAlpha);
		TFCFrame->SetLayoutAlpha(containerAlpha);

		//a_menu->PrintFontList();

	};

	debugMenuUI->Build(attachNewCanvasElements);
}
