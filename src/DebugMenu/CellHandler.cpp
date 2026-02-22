#include "CellHandler.h"
#include "DebugMenu.h"
#include "MCM.h"

namespace DebugMenu
{
	CellHandler::CellHandler()
	{
		logger::debug("Initialized CellHandler");
	}

	void CellHandler::Draw()
	{
		if (!MCM::settings::showCellBorders) return;

		RE::TESObjectCELL* cell = RE::PlayerCharacter::GetSingleton()->GetParentCell();
		if (!cell || cell->IsInteriorCell()) return;

		if (const auto cellCoords = cell->GetCoordinates())
		{
			float maxHeight = cell->GetRuntimeData().cellLand->loadedData->heightExtents[1];
			auto cellHeights = cell->GetRuntimeData().cellLand->loadedData->heights;

			uintptr_t loadedData_addr = reinterpret_cast<uintptr_t>(cell->GetRuntimeData().cellLand->loadedData);
			float baseHeight = *reinterpret_cast<float*>(loadedData_addr + 0x49C0);

			float cellSize = 4096.0f;
			float halfCellSize = cellSize / 2;
			float cellInternalGridLength = cellSize / 32;

			std::vector<RE::NiPoint3> northBorder;
			std::vector<RE::NiPoint3> eastBorder;
			std::vector<RE::NiPoint3> southBorder;
			std::vector<RE::NiPoint3> westBorder;

			std::vector<RE::NiPoint3> quadSN; // south-north
			std::vector<RE::NiPoint3> quadWE; //  west-east

			for (int i = 0; i < 33; i++) // each quad is split into a 32x32 grid. The heights array are the the vaulues at the grid corners -> 33x33 corners
			{
				float step = i * cellInternalGridLength;

				int northQuad = 2;
				int  eastQuad = 3;
				int southQuad = 1;
				int  westQuad = 0;

				int SNQuad = 1;
				int WEQuad = 2;

				int index = i;

				if (i > 16)
				{
					northQuad = 3;
					eastQuad = 1;
					southQuad = 0;
					westQuad = 2;

					SNQuad = 3;
					WEQuad = 3;

					index -= 16;
				}

				float heightNorth = baseHeight + cellHeights[northQuad][272 + index];
				float heightEast = baseHeight + cellHeights[eastQuad][288 - 17 * index];
				float heightSouth = baseHeight + cellHeights[southQuad][16 - index];
				float heightWest = baseHeight + cellHeights[westQuad][17 * index];

				northBorder.push_back(RE::NiPoint3(cellCoords->worldX + step, cellCoords->worldY + cellSize, heightNorth));
				eastBorder.push_back(RE::NiPoint3(cellCoords->worldX + cellSize, cellCoords->worldY + cellSize - step, heightEast));
				southBorder.push_back(RE::NiPoint3(cellCoords->worldX + cellSize - step, cellCoords->worldY, heightSouth));
				westBorder.push_back(RE::NiPoint3(cellCoords->worldX, cellCoords->worldY + step, heightWest));


				if (MCM::settings::showCellQuads)
				{
					float heightSN = baseHeight + cellHeights[SNQuad][17 * index];
					float heightWE = baseHeight + cellHeights[WEQuad][index];
					quadSN.push_back(RE::NiPoint3(cellCoords->worldX + halfCellSize, cellCoords->worldY + step, heightSN));
					quadWE.push_back(RE::NiPoint3(cellCoords->worldX + step, cellCoords->worldY + halfCellSize, heightWE));
				}


			}

			for (int i = 0; i < 32; i++) // cell border along the ground
			{
				auto formID = cell->formID;

				GetDrawHandler()->DrawLine(northBorder[i], northBorder[i + 1], 12, MCM::settings::cellBorderColor, MCM::settings::cellBorderAlpha);
				GetDrawHandler()->DrawLine(eastBorder[i], eastBorder[i + 1], 12, MCM::settings::cellBorderColor, MCM::settings::cellBorderAlpha);
				GetDrawHandler()->DrawLine(southBorder[i], southBorder[i + 1], 12, MCM::settings::cellBorderColor, MCM::settings::cellBorderAlpha);
				GetDrawHandler()->DrawLine(westBorder[i], westBorder[i + 1], 12, MCM::settings::cellBorderColor, MCM::settings::cellBorderAlpha);

				if (MCM::settings::showCellQuads)
				{
					if (!(i % 2)) // even
					{
						GetDrawHandler()->DrawLine(quadSN[i], quadSN[i + 1], 12, MCM::settings::cellQuadsColor, MCM::settings::cellQuadsAlpha);
						GetDrawHandler()->DrawLine(quadWE[i], quadWE[i + 1], 12, MCM::settings::cellQuadsColor, MCM::settings::cellQuadsAlpha);
					}

				}
			}


			if (MCM::settings::showCellQuads && MCM::settings::showInfoOnHover)
			{
				float quarterCellSize = 4096.0f / 4;

				// Cell structure:
				//  
				//  3 | 4
				// ---|---
				//  1 | 2
				//
				for (int i = 0; i < 4; i++)
				{

					float quadCenterX = cellCoords->worldX;
					float quadCenterY = cellCoords->worldY;

					if (i == 0)
					{
						quadCenterX += quarterCellSize;
						quadCenterY += quarterCellSize;
					}
					else if (i == 1)
					{
						quadCenterX += halfCellSize + quarterCellSize;
						quadCenterY += quarterCellSize;
					}
					else if (i == 2)
					{
						quadCenterX += quarterCellSize;
						quadCenterY += halfCellSize + quarterCellSize;
					}
					else if (i == 3)
					{
						quadCenterX += halfCellSize + quarterCellSize;
						quadCenterY += halfCellSize + quarterCellSize;
					}

					float quadCenterZ = baseHeight + cellHeights[i][144];

					RE::NiPoint3 quadCenter{ quadCenterX, quadCenterY, quadCenterZ };

					auto quadMetaData = CreateMetaData();
					quadMetaData->cell = cell;
					quadMetaData->infoType = InfoType::kQuad;
					quadMetaData->quad = i;
					GetDrawHandler()->DrawPoint(quadCenter, 40.0f, MCM::settings::cellQuadsColor, MCM::settings::cellQuadsAlpha, quadMetaData);
				}
			}

			if (!MCM::settings::showCellWalls) return;

			for (int i = 0; i < 32; i++)
			{

				std::vector<RE::NiPoint3> squaresNorth{ northBorder[i], northBorder[i + 1] };
				RE::NiPoint3 top1 = northBorder[i + 1];
				RE::NiPoint3 top2 = northBorder[i];
				top1.z = maxHeight + MCM::settings::cellWallsHeight;
				top2.z = maxHeight + MCM::settings::cellWallsHeight;
				squaresNorth.push_back(top1);
				squaresNorth.push_back(top2);

				std::vector<RE::NiPoint3> squaresEast{ eastBorder[i], eastBorder[i + 1] };
				top1 = eastBorder[i + 1];
				top2 = eastBorder[i];
				top1.z = maxHeight + MCM::settings::cellWallsHeight;
				top2.z = maxHeight + MCM::settings::cellWallsHeight;
				squaresEast.push_back(top1);
				squaresEast.push_back(top2);

				std::vector<RE::NiPoint3> squaresSouth{ southBorder[i], southBorder[i + 1] };
				top1 = southBorder[i + 1];
				top2 = southBorder[i];
				top1.z = maxHeight + MCM::settings::cellWallsHeight;
				top2.z = maxHeight + MCM::settings::cellWallsHeight;
				squaresSouth.push_back(top1);
				squaresSouth.push_back(top2);

				std::vector<RE::NiPoint3> squaresWest{ westBorder[i], westBorder[i + 1] };
				top1 = westBorder[i + 1];
				top2 = westBorder[i];
				top1.z = maxHeight + MCM::settings::cellWallsHeight;
				top2.z = maxHeight + MCM::settings::cellWallsHeight;
				squaresWest.push_back(top1);
				squaresWest.push_back(top2);

				GetDrawHandler()->DrawPolygon(squaresNorth, 4, MCM::settings::cellWallsColor, MCM::settings::cellWallsAlpha, MCM::settings::cellWallsAlpha + 5);
				GetDrawHandler()->DrawPolygon(squaresEast, 4, MCM::settings::cellWallsColor, MCM::settings::cellWallsAlpha, MCM::settings::cellWallsAlpha + 5);
				GetDrawHandler()->DrawPolygon(squaresSouth, 4, MCM::settings::cellWallsColor, MCM::settings::cellWallsAlpha, MCM::settings::cellWallsAlpha + 5);
				GetDrawHandler()->DrawPolygon(squaresWest, 4, MCM::settings::cellWallsColor, MCM::settings::cellWallsAlpha, MCM::settings::cellWallsAlpha + 5);
			}

			// Corners in the air
			RE::NiPoint3 corner1{ cellCoords->worldX,			 cellCoords->worldY,			maxHeight + MCM::settings::cellWallsHeight };
			RE::NiPoint3 corner2{ cellCoords->worldX,			 cellCoords->worldY + cellSize, maxHeight + MCM::settings::cellWallsHeight };
			RE::NiPoint3 corner3{ cellCoords->worldX + cellSize, cellCoords->worldY + cellSize, maxHeight + MCM::settings::cellWallsHeight };
			RE::NiPoint3 corner4{ cellCoords->worldX + cellSize, cellCoords->worldY,			maxHeight + MCM::settings::cellWallsHeight };

			// lines in the air
			GetDrawHandler()->DrawLine(corner1, corner2, 12, MCM::settings::cellBorderColor, MCM::settings::cellBorderAlpha);
			GetDrawHandler()->DrawLine(corner2, corner3, 12, MCM::settings::cellBorderColor, MCM::settings::cellBorderAlpha);
			GetDrawHandler()->DrawLine(corner3, corner4, 12, MCM::settings::cellBorderColor, MCM::settings::cellBorderAlpha);
			GetDrawHandler()->DrawLine(corner4, corner1, 12, MCM::settings::cellBorderColor, MCM::settings::cellBorderAlpha);

			// vertical corner lines from the ground to the air
			GetDrawHandler()->DrawLine(corner1, westBorder[0], 12, MCM::settings::cellBorderColor, MCM::settings::cellBorderAlpha);
			GetDrawHandler()->DrawLine(corner2, northBorder[0], 12, MCM::settings::cellBorderColor, MCM::settings::cellBorderAlpha);
			GetDrawHandler()->DrawLine(corner3, eastBorder[0], 12, MCM::settings::cellBorderColor, MCM::settings::cellBorderAlpha);
			GetDrawHandler()->DrawLine(corner4, southBorder[0], 12, MCM::settings::cellBorderColor, MCM::settings::cellBorderAlpha);
		}
	}
}

