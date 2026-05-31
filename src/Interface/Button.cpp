#include "Button.h"

ScaleformUI::Button::Button(std::string& a_name) : Element(a_name) 
{
	elementType = ELEMENT_TYPE::kBUTTON;
	interactable = true;
}

void ScaleformUI::Button::SetToggleValuePtr(bool* a_toggleValue)
{
	SetAsToggleButton();
	toggleValue = a_toggleValue;
	SetVisibleCorrectToggleImage();
	if (toggleValue && *toggleValue)
	{
		mouseState.set(MOUSE_STATE::kCHECKED);
	}
}

void ScaleformUI::Button::SetCyclicValuePtr(uint32_t* a_cyclicValue)
{
	SetAsCyclicButton();
	cyclicValue = a_cyclicValue;
	if (cyclicValue)
	{
		auto elementGFx = GetGFx();

		if (!elementGFx.HasMember(LAYOUT_NAME))
		{
			logger::debug("UI ERROR: Cyclic button '{}' has no layout", GetInstanceName());
			return;
		}

		RE::GFxValue layout;
		elementGFx.GetMember(LAYOUT_NAME, &layout);

		uint32_t memberIndex = 0;
		layout.VisitMembers([&](const char* a_memberName, RE::GFxValue a_memberGFx)
		{
			numCyclicElements++;
		});

		SetVisibleCorrectCycleImage();
	}
}

void ScaleformUI::Button::SetCorrectToggleValue()
{
	if (!IsToggleButton())
	{
		logger::debug("UI WARNING: Tried to toggle non toggle button '{}'", GetInstanceName());
		return;
	}
	if (!toggleValue)
	{
		logger::debug("UI WARNING: Tried to toggle toggle button with no toggle value'{}'", GetInstanceName());
		return;
	}

	if (IsChecked()) *toggleValue = true;
	else *toggleValue = false;

	SetVisibleCorrectToggleImage();
}

void ScaleformUI::Button::CycleValue()
{
	if (!IsCyclicButton())
	{
		logger::debug("UI WARNING: Tried to cycle non cyclic button '{}'", GetInstanceName());
		return;
	}
	if (!cyclicValue)
	{
		logger::debug("UI WARNING: Tried to cycle cyclic button with no cyclic value'{}'", GetInstanceName());
		return;
	}


	if (*cyclicValue == numCyclicElements - 1) 
	{
		*cyclicValue = 0;
	}
	else *cyclicValue += 1;

	SetVisibleCorrectCycleImage();
}

void ScaleformUI::Button::SetVisibleCorrectToggleImage()
{
	if (toggleValue) SetToggleOnOffImage(*toggleValue);
}

void ScaleformUI::Button::SetVisibleOnToggleImage()
{
	SetToggleOnOffImage(true);
}

void ScaleformUI::Button::SetVisibleOffToggleImage()
{
	SetToggleOnOffImage(false);
}

void ScaleformUI::Button::SetToggleOnOffImage(bool a_enableOn)
{
	auto elementGFx = GetGFx();

	if (elementGFx.HasMember(LAYOUT_NAME))
	{
		RE::GFxValue layout;
		elementGFx.GetMember(LAYOUT_NAME, &layout);

		if (layout.HasMember(TOGGLE_ON_NAME) && layout.HasMember(TOGGLE_OFF_NAME))
		{
			RE::GFxValue on, off;
			layout.GetMember(TOGGLE_ON_NAME, &on);
			layout.GetMember(TOGGLE_OFF_NAME, &off);
			RE::GFxValue::DisplayInfo onInfo, offInfo;
			on.GetDisplayInfo(&onInfo);
			off.GetDisplayInfo(&offInfo);

			onInfo.SetVisible(a_enableOn);
			offInfo.SetVisible(!a_enableOn);

			on.SetDisplayInfo(onInfo);
			off.SetDisplayInfo(offInfo);
		}
	}
}

void ScaleformUI::Button::SetVisibleCorrectCycleImage()
{
	if (!IsCyclicButton()) return;

	auto elementGFx = GetGFx();

	if (elementGFx.HasMember(LAYOUT_NAME))
	{
		RE::GFxValue layout;
		elementGFx.GetMember(LAYOUT_NAME, &layout);

		uint32_t memberIndex = 0;
		layout.VisitMembers([&](const char* a_memberName, RE::GFxValue a_memberGFx)
		{
			RE::GFxValue::DisplayInfo displayInfo;
			a_memberGFx.GetDisplayInfo(&displayInfo);
			if (memberIndex == *cyclicValue)
			{
				displayInfo.SetVisible(true);
			}
			else
			{
				displayInfo.SetVisible(false);
			}

			a_memberGFx.SetDisplayInfo(displayInfo);

			memberIndex++;
		});
	}
}

void ScaleformUI::Button::PrintOrderOfCyclicImages()
{
	if (!IsCyclicButton()) return;

	logger::debug("Children of cylic button: {}", GetInstanceName());
	
	auto elementGFx = GetGFx();

	RE::GFxValue layout;
	elementGFx.GetMember(LAYOUT_NAME, &layout);

	uint32_t memberIndex = 0;
	layout.VisitMembers([&](const char* a_memberName, RE::GFxValue)
	{
		logger::debug(" -Index {}: {}", memberIndex, a_memberName);
		memberIndex++;
	});
}

void ScaleformUI::Button::SetAsToggleButton()
{
	type = BUTTON_TYPE::kTOGGLE;
}

void ScaleformUI::Button::SetAsClickButton()
{
	type = BUTTON_TYPE::kCLICK;
}

void ScaleformUI::Button::SetAsCyclicButton()
{
	type = BUTTON_TYPE::kCYCLIC;
}

bool ScaleformUI::Button::IsToggleButton()
{
	return type == BUTTON_TYPE::kTOGGLE;
}

bool ScaleformUI::Button::IsClickButton()
{
	return type == BUTTON_TYPE::kCLICK;
}

bool ScaleformUI::Button::IsCyclicButton()
{
	return type == BUTTON_TYPE::kCYCLIC;
}