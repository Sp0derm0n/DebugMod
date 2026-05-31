#pragma once

#include "Element.h"

namespace ScaleformUI
{
	class Button : public Element
	{
		public:
			enum class BUTTON_TYPE
			{
				kUNDEFINED = 0,
				kCLICK = 1,
				kTOGGLE = 2,
				kCYCLIC = 3
			};

			Button(std::string& a_name);
			Button* AsButton() override { return static_cast<Button*>(this); }
			bool IsButton() override { return true; }

			void SetToggleValuePtr(bool* a_toggleValue) override; 
			void SetCyclicValuePtr(uint32_t* a_cyclicValue) override;
			void SetCorrectToggleValue() override;
			void CycleValue() override;
			void PrintOrderOfCyclicImages() override;

			void SetVisibleOnToggleImage();
			void SetVisibleOffToggleImage();
			void SetVisibleCorrectToggleImage();
			void SetVisibleCorrectCycleImage();

			bool IsToggleSettingTrue() const { if (toggleValue) { return *toggleValue; } else return false; }

			bool IsToggleButton();
			bool IsClickButton();
			bool IsCyclicButton();

		private:
			friend class Element;
			BUTTON_TYPE	type = BUTTON_TYPE::kUNDEFINED;
			bool* toggleValue = nullptr;
			uint32_t* cyclicValue = nullptr;
			uint32_t numCyclicElements = 0;

			void SetToggleOnOffImage(bool a_enableOn);

			void SetAsToggleButton();
			void SetAsClickButton();
			void SetAsCyclicButton();

	};
}