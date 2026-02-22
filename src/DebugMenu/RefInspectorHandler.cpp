#include "RefInspectorHandler.h"
#include "DebugMenu.h"

DebugMenu::RefInspectorHandler::RefInspectorHandler()
{
	logger::debug("Initialized RefInspectorHandler");
}

void DebugMenu::RefInspectorHandler::Draw()
{
	Utils::ForEachCellInRange(GetCenter(), GetRange(), [&](const RE::TESObjectCELL* a_cell)
	{
		a_cell->ForEachReference([&](RE::TESObjectREFR* a_ref)
		{
			if (!a_ref) return RE::BSContainer::ForEachResult::kContinue;

			float dx = GetCenter().x - a_ref->GetPositionX();
			float dy = GetCenter().y - a_ref->GetPositionY();
			if (dx * dx + dy * dy > GetRange() * GetRange()) return RE::BSContainer::ForEachResult::kContinue;

			auto metaData = CreateMetaData();
			metaData->ref = a_ref;
			metaData->infoType = InfoType::kRef;
			GetDrawHandler()->DrawPoint(a_ref->GetPosition(), 15.0f, 0xF0CA22 /* yellow */, 100, metaData);



		});
	});
}

float DebugMenu::RefInspectorHandler::GetRange()
{
	return 5000.0f;
}