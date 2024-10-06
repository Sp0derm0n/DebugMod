#include "DebugHandler.h"
#include "HUDHandler.h"

void DebugHandler::init()
{
	g_HUD = HUDHandler::GetSingleton();
	logger::info("Initialized DebugHandler");
}

void DebugHandler::DrawAll()
{
	ClearAll();
	if (showNavmesh) DrawNavmesh(navmeshRange);
	if (showOcclusion) DrawOcclusion(occlusionRange);
	DrawTest();
}

void DebugHandler::ClearAll()
{
	g_HUD->pointsToDraw.clear();
}

void DebugHandler::DrawTest()
{
	RE::NiPoint3 pos1{ 36803.42, -17045.783, -4090.1133 };
	RE::NiPoint3 pos2{ 34662.684, -17012.322, -4251.0654 };
	g_HUD->DrawPoint(pos1, 20);
	g_HUD->DrawPoint(pos2, 20);
	g_HUD->DrawLine(pos1, pos2, 5);

}

void DebugHandler::update()
{
	if (g_HUD && g_HUD->g_DrawMenu)
	{
		if (updateCount == 0)
		{
			updateCount = updateRate;
			DrawAll();
		}
		else updateCount -= 1;

		g_HUD->updateCameraPosition();
		g_HUD->updateHUD();
		//g_HUD->g_DrawMenu->DrawPoint(RE::NiPoint2(300,300), 3, 0xFFFFFF, 100);
		//g_HUD->g_DrawMenu->DrawPoint(RE::NiPoint2(800,500), 3, 0xFFFFFF, 100);
		//g_HUD->g_DrawMenu->DrawLine(RE::NiPoint2(300,300), RE::NiPoint2(800,500), 200, 200, 0xFFFFFF, 100);
	}
}

void DebugHandler::DrawNavmesh(float a_range)
{
	if (const auto TES = RE::TES::GetSingleton(); TES)
	{
		if (const auto gridLength = TES->gridCells ? TES->gridCells->length : 0; gridLength > 0) 
		{
			const auto originPos = RE::PlayerCharacter::GetSingleton()->GetPosition();
			const float yPlus  = originPos.y + a_range;
			const float yMinus = originPos.y - a_range;
			const float xPlus  = originPos.x + a_range;
			const float xMinus = originPos.x - a_range;

			for (uint32_t x = 0; x < gridLength; x++)
			{
				for (uint32_t y = 0; y < gridLength; y++)
				{
					if (auto cell = TES->gridCells->GetCell(x, y); cell && cell->IsAttached()) 
					{
						if (const auto cellCoords = cell->GetCoordinates(); cellCoords) 
						{
							const RE::NiPoint2 worldPos{ cellCoords->worldX, cellCoords->worldY };
							if (worldPos.x < xPlus && (worldPos.x + 4096.0f) > xMinus && worldPos.y < yPlus && (worldPos.y + 4096.0f) > yMinus) // if some of the cell is in range
							{
								DrawNavmesh(cell, originPos, a_range);
							}
						}
					}
				}
			}
		}
	}
}

void DebugHandler::DrawNavmesh(RE::TESObjectCELL* a_cell, RE::NiPoint3 a_origin, float a_range)
{
	auto& navmeshes = a_cell->GetRuntimeData().navMeshes->navMeshes;
	for (const auto& navmesh : navmeshes)
	{
		for (const auto vertex : navmesh->vertices)
		{
			auto dx = a_origin.x - vertex.location.x;
			auto dy = a_origin.y - vertex.location.y;
			if (sqrtf(dx*dx + dy*dy) < a_range)
			{
				g_HUD->DrawPoint(vertex.location, 20, 0xFFFFFF, 90);
			}
		}
	}
}

void DebugHandler::DrawOcclusion(float a_range)
{
	auto playerPosition = RE::PlayerCharacter::GetSingleton()->GetPosition();
	g_HUD->DrawPoint(playerPosition, 20, 0xFFFFFF, 90);

	if (const auto TES = RE::TES::GetSingleton(); TES)
	{
		//logger::info("TES obtained");
		TES->ForEachReferenceInRange(RE::PlayerCharacter::GetSingleton(), a_range, [&](RE::TESObjectREFR* a_ref)
		{	
			auto base = a_ref->GetBaseObject();
			if (a_ref->As<RE::TESObjectREFR>() != RE::PlayerCharacter::GetSingleton() && (a_ref->Is(RE::FormType::NPC) || base && base->Is(RE::FormType::NPC)))
			{
				if (auto actor = a_ref->As<RE::Actor>(); actor)
				{
					if (!actor->IsDisabled() && !actor->IsDead() && actor->Get3D())
					{
						//logger::info("actor: {}", actor->GetDisplayFullName());
						g_HUD->DrawPoint(actor->GetPosition(), 20, 0xFFFFFF, 90);
					}	
				}
			}
			return RE::BSContainer::ForEachResult::kContinue;
		});
	}

}