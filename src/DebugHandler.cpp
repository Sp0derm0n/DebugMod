#include "DebugHandler.h"
#include "DrawMenu.h"
#include "UIHandler.h"
#include "DrawHandler.h"
#include "MCM.h"




void DebugHandler::Init()
{
	g_DrawHandler = DrawHandler::GetSingleton();
	g_UI = UIHandler::GetSingleton();
	logger::info("Initialized DebugHandler");
}

// usually <1 �s per file, sometimes ~10 and very rarely 20.
void DebugHandler::OnCellLoad(RE::TESObjectCELL* const& a_cell)
{
	// the sourceFiles array of the navmeshes only contain the first (when not bugged) and last file that edits it. the cell load is called by all mods editing the cell, so we just get the sourceFiles on all these loads
	if (!a_cell || !a_cell->GetRuntimeData().navMeshes) 
	{
		return;
	}
	
	auto& navmeshes = a_cell->GetRuntimeData().navMeshes->navMeshes;
	for (auto& navmesh : navmeshes)
	{
		if(navmesh) OnNavMeshLoad(navmesh.get());
	}
}

// if a navmesh is not referenced by a new mod, sometimes a_cell->GetRuntimeData().navMeshes will not be defined,
// but it can still be found in the nav mesh load
// 
// on the other hand, on navmesh load, the source array will only display one of the base game (+dlc) .esps (i think), 
// but on cell load, new mods can also be found, though it wont always find eg. heathfires.esm when the navmesh is defined in
// skyrim.esm, but these are somethimes caught on navmesh load

// usually <1 �s per file, sometimes ~10 and very rarely 20.
void DebugHandler::OnNavMeshLoad(RE::NavMesh* const& a_navmesh)
{
	if(!a_navmesh) return;

	RE::FormID formID = a_navmesh->GetFormID();
	
	

	if (auto cell = a_navmesh->GetSaveParentCell(); cell)
	{
		CacheNavmesh(a_navmesh, cell->GetFormID());
	}

	if (!a_navmesh->sourceFiles.array) return;

	RE::TESFile** files = a_navmesh->sourceFiles.array->data();
	int numberOfFiles = a_navmesh->sourceFiles.array->size();

	// somehow the code below causes the navmesh to fuck up sometimes

	// yoinked from more informative console sauce https://github.com/Liolel/More-Informative-Console/blob/1613cda4ec067e86f97fb6aae4a7c85533afe031/src/Scaleform/MICScaleform_GetReferenceInfo.cpp#L57
	if ((formID >> 24) == 0x00)  //Refs from Skyrim.ESM will have 00 for the first two hexidecimal digits
	{						     //And refs from all other mods will have a non zero value, so a bitwise && of those two digits with FF will be nonzero for all non Skyrim.ESM mods
		
		if (numberOfFiles == 0 || std::string(files[0]->GetFilename()) != "Skyrim.esm")
		{
			if (!sourceFiles[formID].contains("Skyrim.esm"))
			{
				sourceFiles[formID].insert("Skyrim.esm");
				sourceFilesOrdered[formID].push_back("Skyrim.esm");
			}
		}
	}

	for (int i = 0; i < numberOfFiles; i++)
	{
		if (!sourceFiles[formID].contains(files[i]->GetFilename()))
		{
			sourceFiles[formID].insert(files[i]->GetFilename());
			sourceFilesOrdered[formID].push_back(files[i]->GetFilename());
		}
	}
}

// Sometimes, the navmesh only exists in memory when being loaded (seems to only happen to navmeshes not edited by new mods)
// so they are cached.
void DebugHandler::CacheNavmesh(RE::NavMesh* a_navmesh, RE::FormID a_cellID)
{
	auto formID = a_navmesh->GetFormID();
	if (!cachedNavmeshes.contains(a_cellID))
	{
		std::vector<NavmeshInfo> navmeshes;
		cachedNavmeshes[a_cellID] = navmeshes;
	}

	for (auto& navmeshInfo : cachedNavmeshes[a_cellID])
	{
		if (navmeshInfo.formID == formID) // if the navmesh exists, replace its information
		{
			navmeshInfo.triangles = a_navmesh->triangles;
			navmeshInfo.vertices = a_navmesh->vertices;
			navmeshInfo.extraEdgeInfo = a_navmesh->extraEdgeInfo;
			return;
		}
	}

	NavmeshInfo newInfo;
	newInfo.formID = formID;
	newInfo.triangles = a_navmesh->triangles;
	newInfo.vertices = a_navmesh->vertices;
	newInfo.extraEdgeInfo = a_navmesh->extraEdgeInfo;

	cachedNavmeshes[a_cellID].push_back(newInfo);
}

void DebugHandler::CacheCellNavmeshes(const RE::TESObjectCELL* a_cell) // call on cell fully loaded
{
	auto cellID = a_cell->GetFormID();

	if (!a_cell->GetRuntimeData().navMeshes) 
	{
		return;
	}

	for (const auto& navmesh : a_cell->GetRuntimeData().navMeshes->navMeshes)
	{
		CacheNavmesh(navmesh.get(), cellID);
	}
}

void DebugHandler::DrawAll()
{
	g_DrawHandler->ClearAll();
	if (MCM::settings::showCellBorders) DrawCellBorders();
	if (MCM::settings::showNavmesh) DrawNavmesh(RE::PlayerCharacter::GetSingleton()->GetPosition(), MCM::settings::navmeshRange);
	if (MCM::settings::showOcclusion) DrawOcclusion(RE::PlayerCharacter::GetSingleton()->GetPosition(), MCM::settings::occlusionRange);
    //DrawLightBulbs(RE::PlayerCharacter::GetSingleton()->GetPosition(), 4000.0f);
	//DrawTest();
}

// borrowed from smoothcam
RE::NiPointer<RE::NiCamera> GetNiCamera(RE::PlayerCamera* camera) 
{
	if (camera->cameraRoot->GetChildren().size() == 0) return nullptr;
	for (auto& entry : camera->cameraRoot->GetChildren()) 
	{
		auto asCamera = skyrim_cast<RE::NiCamera*>(entry.get());
		if (asCamera) return RE::NiPointer<RE::NiCamera>(asCamera);
	}
	return nullptr;
}

void DebugHandler::DrawTest()
{
	auto pos3 = Linalg::Vector4(36803.42, -17045.783, -4170.1133, 1);
	auto pos4 = Linalg::Vector4(34662.684, -17012.322, -4400.0654, 1);
}

bool DebugHandler::isAnyDebugON()
{
	return MCM::settings::showCellBorders || MCM::settings::showNavmesh || MCM::settings::showOcclusion || MCM::settings::showCoordinates;
}

void DebugHandler::OpenDrawMenu()
{
	DrawMenu::OpenMenu();
	auto uiTask = [&]() 
	{;
		g_DrawHandler->Init();
	};
	SKSE::GetTaskInterface()->AddUITask(uiTask);
}

void DebugHandler::CloseDrawMenu()
{
	DrawMenu::CloseMenu();
	g_DrawHandler->ClearAll();
	g_DrawHandler->g_DrawMenu = nullptr;
}

void DebugHandler::Update(float a_delta)
{
	if (g_UI && g_UI->isMenuOpen)
	{
		g_UI->Update();
	}

	timeSinceLastUpdate += a_delta;

	if (g_DrawHandler && g_DrawHandler->g_DrawMenu)
	{
		if (MCM::settings::updateRate == 0 || timeSinceLastUpdate > 1.0f/MCM::settings::updateRate)
		{
			timeSinceLastUpdate = 0;
			g_DrawHandler->alphaMultiplier = GetLightLevel();
			DrawAll();
		}
		g_DrawHandler->Update(a_delta);
		ShowCoordinates();
	}

	// has to be lowest in this function
	if (g_DrawHandler && g_DrawHandler->isMenuOpen == false)
	{
		if (hasDebugMenuBeenOpenedBefore && isAnyDebugON())
			OpenDrawMenu();
		else
			return;
	}
	else if (!isAnyDebugON())
		CloseDrawMenu();
}

void DebugHandler::ShowCoordinates()
{
    if (!g_DrawHandler->isMenuOpen) return;// should only try to open the coordinatebox whe the drawMenu is open

	if (MCM::settings::showCoordinates) 
	{
		if (!isCoordinatesBoxVisible) 
		{
			g_DrawHandler->g_DrawMenu->ShowBox("coordinatesBox");
			isCoordinatesBoxVisible = true;
		}
		RE::NiPoint3 playerPosition = RE::PlayerCharacter::GetSingleton()->GetPosition();
		g_DrawHandler->g_DrawMenu->SetCoordinates(playerPosition.x, playerPosition.y, playerPosition.z);
	}
	else if (isCoordinatesBoxVisible)
	{
		g_DrawHandler->g_DrawMenu->HideBox("coordinatesBox");
		isCoordinatesBoxVisible = false;
	}
}

void DebugHandler::DrawCellBorders()
{
	RE::TESObjectCELL* cell = RE::PlayerCharacter::GetSingleton()->GetParentCell();
	if(!cell || cell->IsInteriorCell()) return;

	if (const auto cellCoords = cell->GetCoordinates(); cellCoords)
	{
		float maxHeight = cell->GetRuntimeData().cellLand->loadedData->heightExtents[1];
		auto cellHeights = cell->GetRuntimeData().cellLand->loadedData->heights;

		uintptr_t loadedData_addr = reinterpret_cast<uintptr_t>(cell->GetRuntimeData().cellLand->loadedData);
		float baseHeight = *reinterpret_cast<float *>(loadedData_addr + 0x49C0);

		float cellSize = 4096.0f;
		float halfCellSize = cellSize/2;
		float cellInternalGridLength = cellSize/32;

		std::vector<RE::NiPoint3> northBorder;
		std::vector<RE::NiPoint3> eastBorder;
		std::vector<RE::NiPoint3> southBorder;
		std::vector<RE::NiPoint3> westBorder;

		std::vector<RE::NiPoint3> quadSN; // south-north
		std::vector<RE::NiPoint3> quadWE; //  west-east

		for (int i = 0; i < 33; i++) // each quad is split into a 32x32 grid. The heights array are the the vaulues at the grid corners -> 33x33 corners
		{
			float step = i*cellInternalGridLength;

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

			float heightNorth = baseHeight + cellHeights[northQuad][272+index]; 
			float heightEast  = baseHeight + cellHeights[ eastQuad][288-17*index]; 
			float heightSouth = baseHeight + cellHeights[southQuad][16 - index]; 
			float heightWest  = baseHeight + cellHeights[ westQuad][17*index]; 

			northBorder.push_back(RE::NiPoint3(cellCoords->worldX + step,			 cellCoords->worldY + cellSize,        heightNorth));
			 eastBorder.push_back(RE::NiPoint3(cellCoords->worldX + cellSize,		 cellCoords->worldY + cellSize - step, heightEast));
			southBorder.push_back(RE::NiPoint3(cellCoords->worldX + cellSize - step, cellCoords->worldY,				   heightSouth));
			 westBorder.push_back(RE::NiPoint3(cellCoords->worldX,					 cellCoords->worldY + step,			   heightWest));
		

			if (MCM::settings::showCellQuads)
			{
				float heightSN = baseHeight + cellHeights[SNQuad][17*index];
				float heightWE = baseHeight + cellHeights[WEQuad][index];
				quadSN.push_back(RE::NiPoint3(cellCoords->worldX + halfCellSize, cellCoords->worldY + step,			heightSN));
				quadWE.push_back(RE::NiPoint3(cellCoords->worldX + step,		 cellCoords->worldY + halfCellSize,	heightWE));
			}


		}
		
		for (int i = 0; i < 32; i++) // cell border along the ground
		{
			g_DrawHandler->DrawLine(northBorder[i], northBorder[i+1], 12, MCM::settings::cellBorderColor, MCM::settings::cellBorderAlpha, true);
			g_DrawHandler->DrawLine( eastBorder[i],  eastBorder[i+1], 12, MCM::settings::cellBorderColor, MCM::settings::cellBorderAlpha, true);
			g_DrawHandler->DrawLine(southBorder[i], southBorder[i+1], 12, MCM::settings::cellBorderColor, MCM::settings::cellBorderAlpha, true);
			g_DrawHandler->DrawLine( westBorder[i],  westBorder[i+1], 12, MCM::settings::cellBorderColor, MCM::settings::cellBorderAlpha, true);

			if (MCM::settings::showCellQuads)
			{
				if (!(i % 2)) // even
				{
					g_DrawHandler->DrawLine(quadSN[i], quadSN[i+1], 12, MCM::settings::cellQuadsColor, MCM::settings::cellQuadsAlpha, true);
					g_DrawHandler->DrawLine(quadWE[i], quadWE[i+1], 12, MCM::settings::cellQuadsColor, MCM::settings::cellQuadsAlpha, true);
				}
				
			}
		}

		
		if (MCM::settings::showCellQuads && MCM::settings::showInfoOnHover)
		{
			float quarterCellSize = 4096.0f/4;
			
			// Cell structure:
			//  
			//  3 | 4
			// ---|---
			//  1 | 2
			//
			for (int i = 0; i < 4; i++)
			{
				std::string infoStr = GetQuadInfo(cell, i);

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
				std::vector<RE::NiPoint3> dummyPolygon{ quadCenter, quadCenter, quadCenter };

				// draw invisible dummy polygon at the quad center to be able to show info since info can only be show by looking at polygon corners
				g_DrawHandler->DrawPolygon(dummyPolygon, 0, 0x000000, 0, 0, infoStr);
				g_DrawHandler->DrawPoint(quadCenter, 40.0f, MCM::settings::cellQuadsColor, MCM::settings::cellQuadsAlpha);
			}
		}

		if(!MCM::settings::showCellWalls) return;

		for (int i = 0; i < 32; i++)
		{

			std::vector<RE::NiPoint3> squaresNorth{ northBorder[i], northBorder[i+1] };
			RE::NiPoint3 top1 = northBorder[i+1];
			RE::NiPoint3 top2 = northBorder[i];
			top1.z = maxHeight + MCM::settings::cellWallsHeight;
			top2.z = maxHeight + MCM::settings::cellWallsHeight;
			squaresNorth.push_back(top1);
			squaresNorth.push_back(top2);

			std::vector<RE::NiPoint3> squaresEast{ eastBorder[i], eastBorder[i+1] };
			top1 = eastBorder[i+1];
			top2 = eastBorder[i];
			top1.z = maxHeight + MCM::settings::cellWallsHeight;
			top2.z = maxHeight + MCM::settings::cellWallsHeight;
			squaresEast.push_back(top1);
			squaresEast.push_back(top2);

			std::vector<RE::NiPoint3> squaresSouth{ southBorder[i], southBorder[i+1] };
			top1 = southBorder[i+1];
			top2 = southBorder[i];
			top1.z = maxHeight + MCM::settings::cellWallsHeight;
			top2.z = maxHeight + MCM::settings::cellWallsHeight;
			squaresSouth.push_back(top1);
			squaresSouth.push_back(top2);

			std::vector<RE::NiPoint3> squaresWest{ westBorder[i], westBorder[i+1] };
			top1 = westBorder[i+1];
			top2 = westBorder[i];
			top1.z = maxHeight + MCM::settings::cellWallsHeight;
			top2.z = maxHeight + MCM::settings::cellWallsHeight;
			squaresWest.push_back(top1);
			squaresWest.push_back(top2);

			g_DrawHandler->DrawPolygon(squaresNorth, 4, MCM::settings::cellWallsColor, MCM::settings::cellWallsAlpha, MCM::settings::cellWallsAlpha + 5);
			g_DrawHandler->DrawPolygon(squaresEast, 4, MCM::settings::cellWallsColor, MCM::settings::cellWallsAlpha, MCM::settings::cellWallsAlpha + 5);
			g_DrawHandler->DrawPolygon(squaresSouth, 4, MCM::settings::cellWallsColor, MCM::settings::cellWallsAlpha, MCM::settings::cellWallsAlpha + 5);
			g_DrawHandler->DrawPolygon(squaresWest, 4, MCM::settings::cellWallsColor, MCM::settings::cellWallsAlpha, MCM::settings::cellWallsAlpha + 5);
		}

		// Corners in the air
		RE::NiPoint3 corner1{ cellCoords->worldX,			 cellCoords->worldY,			maxHeight + MCM::settings::cellWallsHeight };
		RE::NiPoint3 corner2{ cellCoords->worldX,			 cellCoords->worldY + cellSize, maxHeight + MCM::settings::cellWallsHeight };
		RE::NiPoint3 corner3{ cellCoords->worldX + cellSize, cellCoords->worldY + cellSize, maxHeight + MCM::settings::cellWallsHeight };
		RE::NiPoint3 corner4{ cellCoords->worldX + cellSize, cellCoords->worldY,			maxHeight + MCM::settings::cellWallsHeight };

		// lines in the air
		g_DrawHandler->DrawLine(corner1, corner2, 12, MCM::settings::cellBorderColor, MCM::settings::cellBorderAlpha, false);
		g_DrawHandler->DrawLine(corner2, corner3, 12, MCM::settings::cellBorderColor, MCM::settings::cellBorderAlpha, false);
		g_DrawHandler->DrawLine(corner3, corner4, 12, MCM::settings::cellBorderColor, MCM::settings::cellBorderAlpha, false);
		g_DrawHandler->DrawLine(corner4, corner1, 12, MCM::settings::cellBorderColor, MCM::settings::cellBorderAlpha, false);

		// vertical corner lines from the ground to the air
		g_DrawHandler->DrawLine(corner1, westBorder[0], 12, MCM::settings::cellBorderColor, MCM::settings::cellBorderAlpha, false);
		g_DrawHandler->DrawLine(corner2, northBorder[0], 12, MCM::settings::cellBorderColor, MCM::settings::cellBorderAlpha, false);
		g_DrawHandler->DrawLine(corner3, eastBorder[0], 12, MCM::settings::cellBorderColor, MCM::settings::cellBorderAlpha, false);
		g_DrawHandler->DrawLine(corner4, southBorder[0], 12, MCM::settings::cellBorderColor, MCM::settings::cellBorderAlpha, false);
	}
}

void DebugHandler::ForEachCellInRange(RE::NiPoint3 a_origin, float a_range, std::function<void(const RE::TESObjectCELL* a_cell)> a_callback)
{
	if (const auto* TES = RE::TES::GetSingleton(); TES)
	{
		if (TES->interiorCell)
		{
			a_callback(TES->interiorCell);
		}
		else if (const auto gridLength = TES->gridCells ? TES->gridCells->length : 0; gridLength > 0)
		{
			const float yPlus = a_origin.y + a_range;
			const float yMinus = a_origin.y - a_range;
			const float xPlus = a_origin.x + a_range;
			const float xMinus = a_origin.x - a_range;

			for (uint32_t x = 0; x < gridLength; x++)
			{
				for (uint32_t y = 0; y < gridLength; y++)
				{
					if (const auto cell = TES->gridCells->GetCell(x, y); cell && cell->IsAttached())
					{
						if (const auto cellCoords = cell->GetCoordinates(); cellCoords)
						{
							const RE::NiPoint2 worldPos{ cellCoords->worldX, cellCoords->worldY };
							if (worldPos.x < xPlus && (worldPos.x + 4096.0f) > xMinus && worldPos.y < yPlus && (worldPos.y + 4096.0f) > yMinus) // if some of the cell is in range
							{
								a_callback(cell);
							}
						}
					}
				}
			}
		}
		if (const auto ws = TES->GetRuntimeData2().worldSpace) 
		{
			if (const auto skyCell = ws ? ws->GetSkyCell() : nullptr; skyCell) 
			{
				a_callback(skyCell);
			}
		}
	}
}

void DebugHandler::DrawNavmesh(RE::NiPoint3 a_origin, float a_range)
{
	ForEachCellInRange(a_origin, a_range, [&](const RE::TESObjectCELL* a_cell)
	{
		RE::FormID cellID = a_cell->GetFormID();

		if (isCellsCacheFinalized[cellID] == false)
		{
			if (a_cell->GetRuntimeData().navMeshes);
			{
				CacheCellNavmeshes(a_cell);
			}
			isCellsCacheFinalized[cellID] = true;
		}

		if (!cachedNavmeshes.contains(cellID))
		{
			return;
		}
		
		/*
		if (a_cell->GetRuntimeData().navMeshes)
		{
		
			logger::debug("CELL: {:X} {}", cellID, a_cell->GetFormEditorID());
			for (const auto& navmesh : a_cell->GetRuntimeData().navMeshes->navMeshes)
			{
				
				debug(" navmesh {:X} address: {:X}", navmesh->formID, reinterpret_cast<uintptr_t>(navmesh.get()));
			}
		}//*/



		for (auto& navmesh : cachedNavmeshes[cellID])
		{
			auto& vertices = navmesh.vertices;
			auto& triangles = navmesh.triangles;

			std::string infoStr = GetNavmeshInfo(navmesh.formID, a_cell);

			for (const auto vertex : vertices)
			{
			
				break; // dont draw vertices
				auto dx = a_origin.x - vertex.location.x;
				auto dy = a_origin.y - vertex.location.y;
				if (dx*dx + dy*dy < a_range * a_range)
					g_DrawHandler->DrawPoint(vertex.location, 4, 0xFFFFFF, 90);
			}	

			for (int i = 0; i < triangles.size(); i++)
			{
				const auto& triangle = triangles[i];
				uint16_t triangleFlag = triangle.triangleFlags.underlying();


				if (!MCM::settings::useRuntimeNavmesh && !(triangleFlag & inFileFlag))
				{
					if (i + 2 < triangles.size())
					{
						bool isNextTriangleInFile = triangles[i+1].triangleFlags.underlying() & inFileFlag;
						bool isNextNextTriangleInFile = triangles[i+2].triangleFlags.underlying() & inFileFlag;
						if (!isNextTriangleInFile && !isNextNextTriangleInFile)
						{
							break; // sometimes (very rarely) a triangle in the middle of the array has no inFileFlag, so if the next two triangles are in file, don't break
						}
					}
				}

				if (MCM::settings::useRuntimeNavmesh && triangle.triangleFlags.all(RE::BSNavmeshTriangle::TriangleFlag::kOverlapping))
				{
					continue;
				}

				bool skip = false;
				for (const auto i : triangle.vertices)
				{
					auto dx = a_origin.x - vertices[i].location.x;
					auto dy = a_origin.y - vertices[i].location.y;
					if (sqrtf(dx*dx + dy*dy) > a_range) 
					{
						skip = true;
						break;
					}
				}
				if (skip) continue;
				if (triangle.triangleFlags.any(RE::BSNavmeshTriangle::TriangleFlag::kDeleted)) 
				{
					continue;
				}
				uint32_t triangleColor = MCM::settings::navmeshColor;

				if (triangleFlag & doorFlag) 
					triangleColor = MCM::settings::navmeshDoorColor;

				else if (triangleFlag & waterFlag) 
					triangleColor = MCM::settings::navmeshWaterColor;

				else if (triangle.triangleFlags.any(RE::BSNavmeshTriangle::TriangleFlag::kPreferred)) 
					triangleColor = MCM::settings::navmeshPrefferedColor;

				float borderThickness = triangleColor == MCM::settings::navmeshColor ? 4.0f : 5.0f;

				auto vertex0 = vertices[triangle.vertices[0]].location;
				auto vertex1 = vertices[triangle.vertices[1]].location;
				auto vertex2 = vertices[triangle.vertices[2]].location;
			
				if (MCM::settings::showNavmeshTriangles)
				{
					g_DrawHandler->DrawPolygon({ vertex0, vertex1, vertex2 }, borderThickness, triangleColor, MCM::settings::navmeshAlpha, MCM::settings::navmeshBorderAlpha, infoStr);
				
					for (int edge = 0; edge < 3; edge++)
					{
						uint16_t edgeFlag = 1 << edge;
						if (triangleFlag & edgeFlag)
						{
							uint16_t edgeInfoIndex = triangle.triangles[edge];
							if (edgeInfoIndex < navmesh.extraEdgeInfo.size())
							{
								uint32_t edgeLinkColor = MCM::settings::navmeshCellEdgeLinkColor;
								uint32_t edgeLinkAlpha = MCM::settings::navmeshEdgeLinkAlpha;
								EdgeLinkPosition edgeLinkPosition = EdgeLinkPosition::kCenter;
								if (navmesh.extraEdgeInfo.data()[edgeInfoIndex].type.any(RE::EDGE_EXTRA_INFO_TYPE::kLedgeUp))
								{
									edgeLinkColor = MCM::settings::navmeshLedgeEdgeLinkColor;
									edgeLinkAlpha = static_cast<uint32_t>(100 - (100 - edgeLinkAlpha)*(100 - edgeLinkAlpha)/100.0f); // cell border edgelinks usually overlap almost entirely, so their combined opacity is probably (1-(1-opacity)^2), ie. if they are at 20% opcaity, combined they are probably at 7% opacity
									edgeLinkPosition = EdgeLinkPosition::kAbove;
								}
								else if (navmesh.extraEdgeInfo.data()[edgeInfoIndex].type.any(RE::EDGE_EXTRA_INFO_TYPE::kLedgeDown))
								{
									edgeLinkColor = MCM::settings::navmeshLedgeEdgeLinkColor;
									edgeLinkAlpha = static_cast<uint32_t>(100 - (100 - edgeLinkAlpha)*(100 - edgeLinkAlpha)/100.0f);
									edgeLinkPosition = EdgeLinkPosition::kBelow;
								}

								DrawNavmeshEdgeLink(vertices[triangle.vertices[edge]].location, vertices[triangle.vertices[edge == 2 ? 0 : edge+1]].location, edgeLinkColor, edgeLinkAlpha, edgeLinkPosition);
							}
						}
					}
				}			

				// quarter flag = height of 16 units,
				// half flag = height of 32 units,
				// tri flag = height of 64 units,
				// full flag = height of 128 units

				if (MCM::settings::showNavmeshCover)
				{
					bool isLeftCover = false;
					bool isRightCover = false;
					int32_t height = 0;

					if (triangle.traversalFlags.any(RE::BSNavmeshTriangle::TraversalFlag::kEdge0_CoverValueQuarter)) height += 16;
					if (triangle.traversalFlags.any(RE::BSNavmeshTriangle::TraversalFlag::kEdge0_CoverValueHalf)) height += 32;
					if (triangle.traversalFlags.any(RE::BSNavmeshTriangle::TraversalFlag::kEdge0_CoverValueTri)) height += 64;
					if (triangle.traversalFlags.any(RE::BSNavmeshTriangle::TraversalFlag::kEdge0_CoverValueFull)) height += 128;

					if (triangle.traversalFlags.any(RE::BSNavmeshTriangle::TraversalFlag::kEdge0_Left)) isLeftCover = true;
					if (triangle.traversalFlags.any(RE::BSNavmeshTriangle::TraversalFlag::kEdge0_Right)) isRightCover= true;

					if (height) 
					{

						DrawNavmeshCover(vertex0, vertex1, height, isLeftCover, isRightCover);
					}

					height = 0;
					if (triangle.traversalFlags.any(RE::BSNavmeshTriangle::TraversalFlag::kEdge1_CoverValueQuarter)) height += 16;
					if (triangle.traversalFlags.any(RE::BSNavmeshTriangle::TraversalFlag::kEdge1_CoverValueHalf)) height += 32;
					if (triangle.traversalFlags.any(RE::BSNavmeshTriangle::TraversalFlag::kEdge1_CoverValueTri)) height += 64;
					if (triangle.traversalFlags.any(RE::BSNavmeshTriangle::TraversalFlag::kEdge1_CoverValueFull)) height += 128;

					if (triangle.traversalFlags.any(RE::BSNavmeshTriangle::TraversalFlag::kEdge1_Left)) isLeftCover = true;
					if (triangle.traversalFlags.any(RE::BSNavmeshTriangle::TraversalFlag::kEdge1_Right)) isRightCover= true;

					if (height) 
					{
						DrawNavmeshCover(vertex1, vertex2, height, isLeftCover, isRightCover);
					}
				}
			}
		}
	});
}

void DebugHandler::DrawNavmeshEdgeLink(const RE::NiPoint3& a_point1, const RE::NiPoint3& a_point2, uint32_t a_color, uint32_t a_alpha, EdgeLinkPosition a_position)
{
	RE::NiPoint3 directionAlongLine = a_point2 - a_point1;
	RE::NiPoint3 center = (a_point1 + a_point2)/2;

	float boxHeight = 24.0f;
	float heightAbove; 
	float heightBelow;
	directionAlongLine /= 5;

	switch (a_position)
	{
		case EdgeLinkPosition::kAbove:
		{
			heightAbove = boxHeight * 5/6.0f;
			heightBelow = boxHeight * 1/6.0f;
			break;
		}
		case EdgeLinkPosition::kCenter:
		{
			heightAbove = boxHeight / 2;
			heightBelow = boxHeight / 2;
			break;
		}
		case EdgeLinkPosition::kBelow:
		{
			heightAbove = boxHeight * 1/6.0f;
			heightBelow = boxHeight * 5/6.0f;
			break;
		}
	}
	
	RE::NiPoint3 corner1 = center + directionAlongLine; corner1.z += heightAbove;
	RE::NiPoint3 corner2 = center + directionAlongLine; corner2.z -= heightBelow;
	RE::NiPoint3 corner3 = center - directionAlongLine; corner3.z -= heightBelow;
	RE::NiPoint3 corner4 = center - directionAlongLine; corner4.z += heightAbove;

	std::vector<RE::NiPoint3> polygon{ corner1, corner2, corner3, corner4 };

	g_DrawHandler->DrawPolygon(polygon, 0, a_color, a_alpha, 0);
}

void DebugHandler::DrawNavmeshCover(const RE::NiPoint3& a_rightPoint, const RE::NiPoint3& a_leftPoint, int32_t a_height, bool a_left, bool a_right)
{
	if (!MCM::settings::showCoverBeams)
	{
		a_left = false;
		a_right = false;
	}
	// maybe horizontal line every 16 or 32 height?
	uint32_t color = MCM::settings::navmeshCoverColor;
	uint32_t alpha = MCM::settings::navmeshCoverAlpha;
	uint32_t borderColor = MCM::settings::navmeshCoverBorderColor;
	uint32_t borderAlpha = MCM::settings::navmeshCoverBorderAlpha;

	float height = static_cast<float>(a_height);

	if (a_height == 240)
	{
		color = MCM::settings::navmeshMaxCoverColor;
		alpha = MCM::settings::navmeshMaxCoverAlpha;
		borderColor = MCM::settings::navmeshMaxCoverBorderColor;
		borderAlpha = MCM::settings::navmeshMaxCoverBorderAlpha;
	}
	else if (a_height < 64) // cover is ledge cover
	{
		height = -height;
	}

	RE::NiPoint3 directionAlongLine = a_rightPoint - a_leftPoint;
	float directionAlongLineLength = directionAlongLine.Length();
	RE::NiPoint3 unitDirectionAlongLine = directionAlongLine/directionAlongLineLength;

	float borderThickness = 3.0f;
	float coverBeamWidth = 20.0f;
	if (directionAlongLineLength < coverBeamWidth) coverBeamWidth = directionAlongLineLength * 0.4f;

	RE::NiPoint3 leftOffset = unitDirectionAlongLine*coverBeamWidth;
	RE::NiPoint3 rightOffset = -leftOffset;

	if (!a_left) leftOffset = RE::NiPoint3(0.0f, 0.0f, 0.0f);
	if (!a_right) rightOffset = RE::NiPoint3(0.0f, 0.0f, 0.0f);

	// Draw cover
	RE::NiPoint3 corner1 = a_leftPoint + leftOffset*1.02; 
	RE::NiPoint3 corner2 = a_leftPoint + leftOffset*1.02;   corner2.z += height;
	RE::NiPoint3 corner3 = a_rightPoint + rightOffset*1.02; corner3.z += height;
	RE::NiPoint3 corner4 = a_rightPoint + rightOffset*1.02; 

	std::vector<RE::NiPoint3> polygon{ corner1, corner2, corner3, corner4 };

	g_DrawHandler->DrawPolygon(polygon, borderThickness, color, alpha, borderAlpha, "", borderColor, true);

	// Draw horizontal lines on cover to visualize height
	if (MCM::settings::showNavmeshCoverLines)
	{
		uint32_t stepSize = MCM::settings::linesHeight;
		for (int step = stepSize; step < a_height; step += stepSize)
		{
			int8_t multiplier = a_height < 64 ? -1 : 1;
			RE::NiPoint3 point1 = corner1; point1.z += multiplier * step;
			RE::NiPoint3 point2 = corner4; point2.z += multiplier * step;
			g_DrawHandler->DrawLine(point1, point2, borderThickness, borderColor, borderAlpha);
		}
	}
	

	// Draw dot in the middle of the triangle edge to contain info:

	if (MCM::settings::showNavmeshCoverInfo)
	{
		RE::NiPoint3 middle = a_leftPoint + directionAlongLine/2;
		std::vector<RE::NiPoint3> dummyPolygon{ middle, middle, middle};

		std::string infoStr = fmt::format("Cover height: {} units", a_height);
		if (MCM::settings::showNavmeshCoverLines) infoStr += fmt::format("\nSubsections height: {} units", MCM::settings::linesHeight);
		infoStr += fmt::format("\nLedge: {}", height < 0);
		infoStr += fmt::format("\nRight:  {}", a_right);
		infoStr += fmt::format("\nLeft:    {}", a_left);


		// draw invisible dummy polygon to able to show info since info can only be shown by looking at polygon corners
		g_DrawHandler->DrawPolygon(dummyPolygon, 0, 0x000000, 0, 0, infoStr);
		g_DrawHandler->DrawPoint(middle, 10, borderColor, borderAlpha);
	}

	
	// Draw end beams
	alpha *= 2;
	if (a_left)
	{
		corner1 = a_leftPoint;
		corner2 = a_leftPoint;				 corner2.z += height;
		corner3 = a_leftPoint + leftOffset;  corner3.z += height;
		corner4 = a_leftPoint + leftOffset;

		std::vector<RE::NiPoint3> beamPolygon{ corner1, corner2, corner3, corner4 };
		g_DrawHandler->DrawPolygon(beamPolygon, 0.0f, 0x000000, alpha, 0);
	}

	if (a_right)
	{
		corner1 = a_rightPoint;
		corner2 = a_rightPoint;				  corner2.z += height;
		corner3 = a_rightPoint + rightOffset; corner3.z += height;
		corner4 = a_rightPoint + rightOffset;

		std::vector<RE::NiPoint3> beamPolygon{ corner1, corner2, corner3, corner4 };
		g_DrawHandler->DrawPolygon(beamPolygon, 0.0f, 0x000000, alpha, 0);
	}
}

void DebugHandler::DrawOcclusion(RE::NiPoint3 a_origin, float a_range)
{
	ForEachCellInRange(a_origin, a_range, [&](const RE::TESObjectCELL* a_cell)
	{
		a_cell->ForEachReferenceInRange(a_origin, a_range, [&](RE::TESObjectREFR* a_ref)
		{	
			if (a_ref->GetBaseObject()->formID == 0x17) // planemarker
			{
				auto extra = &a_ref->extraList;
				for (const auto& data : a_ref->extraList)
				{
					if (data.GetType() == RE::ExtraDataType::kPrimitive)
					{
						uintptr_t dataAddr = reinterpret_cast<uintptr_t>(&data);
						uintptr_t primitiveAddr = *reinterpret_cast<uintptr_t *>(dataAddr+0x10);
						float xBound = *reinterpret_cast<float *>(primitiveAddr + 0x0C); // the sizes are half the box length
						float yBound = *reinterpret_cast<float *>(primitiveAddr + 0x10);
						float zBound = *reinterpret_cast<float *>(primitiveAddr + 0x14);
						
						std::string infoStr = GetOcclusionInfo(a_cell, a_ref,  RE::NiPoint3(2*xBound, 2*yBound, 2*zBound), false);

						RE::NiAVObject* AV = a_ref->Get3D();

						if (a_ref->IsDisabled() || !AV) continue;

						RE::NiMatrix3 M = AV->world.rotate; 

						uint32_t baseColor = MCM::settings::occlusionColor;

						// infoStr = GetOcclusionInfo(a_cell, a_ref,  RE::NiPoint3(2*xBound, 2*yBound, 2*zBound), false);
						// define matrix by: a_ref->GetAngleX(), a_ref->GetAngleY(), a_ref->GetAngleZ()
						
						RE::NiPoint3 center = a_ref->GetPosition();
						RE::NiPoint3 upperLeft1  = M * RE::NiPoint3( +xBound, +yBound, +zBound ); 
						RE::NiPoint3 upperRight1 = M * RE::NiPoint3( -xBound, +yBound, +zBound ); 
						RE::NiPoint3 lowerRight1 = M * RE::NiPoint3( -xBound, +yBound, -zBound ); 
						RE::NiPoint3 lowerLeft1  = M * RE::NiPoint3( +xBound, +yBound, -zBound ); 	  
						RE::NiPoint3 upperLeft2  = M * RE::NiPoint3( +xBound, -yBound, +zBound );
						RE::NiPoint3 upperRight2 = M * RE::NiPoint3( -xBound, -yBound, +zBound );
						RE::NiPoint3 lowerRight2 = M * RE::NiPoint3( -xBound, -yBound, -zBound );
						RE::NiPoint3 lowerLeft2  = M * RE::NiPoint3( +xBound, -yBound, -zBound );
						
						std::vector<RE::NiPoint3>  frontPlane{ center+upperLeft1,  center+upperRight1, center+lowerRight1, center+lowerLeft1  };
						std::vector<RE::NiPoint3>   backPlane{ center+upperLeft2,  center+upperRight2, center+lowerRight2, center+lowerLeft2  };
						std::vector<RE::NiPoint3>   leftPlane{ center+upperLeft1,  center+lowerLeft1,  center+lowerLeft2,  center+upperLeft2  };
						std::vector<RE::NiPoint3>  rightPlane{ center+upperRight1, center+upperRight2, center+lowerRight2, center+lowerRight1 };
						std::vector<RE::NiPoint3>    topPlane{ center+upperLeft1,  center+upperLeft2,  center+upperRight2, center+upperRight1 };
						std::vector<RE::NiPoint3> bottomPlane{ center+lowerLeft1,  center+lowerLeft2,  center+lowerRight2, center+lowerRight1 };
			
						g_DrawHandler->DrawPolygon(frontPlane,  0, baseColor, MCM::settings::occlusionAlpha, 0, infoStr);
						g_DrawHandler->DrawPolygon(backPlane,   0, baseColor, MCM::settings::occlusionAlpha, 0, infoStr);
						g_DrawHandler->DrawPolygon(leftPlane,   0, baseColor, MCM::settings::occlusionAlpha, 0, infoStr);
						g_DrawHandler->DrawPolygon(rightPlane,  0, baseColor, MCM::settings::occlusionAlpha, 0, infoStr);
						g_DrawHandler->DrawPolygon(topPlane,    0, baseColor, MCM::settings::occlusionAlpha, 0, infoStr);
						g_DrawHandler->DrawPolygon(bottomPlane, 0, baseColor, MCM::settings::occlusionAlpha, 0, infoStr);

						// draw borders around the cube edges
						for (int i = 0; i < 4; i++)
						{
							int nextIndex = i == 3? 0 : i+1;
							g_DrawHandler->DrawLine(frontPlane[i], frontPlane[nextIndex], 5, MCM::settings::occlusionBorderColor, MCM::settings::occlusionBorderAlpha, false);
							g_DrawHandler->DrawLine( backPlane[i],  backPlane[nextIndex], 5, MCM::settings::occlusionBorderColor, MCM::settings::occlusionBorderAlpha, false);
							g_DrawHandler->DrawLine(frontPlane[i],  backPlane[i],		 5, MCM::settings::occlusionBorderColor, MCM::settings::occlusionBorderAlpha, false);
						}
					}
				}
			}
			return RE::BSContainer::ForEachResult::kContinue;
		});
	});
}

void DebugHandler::DrawLightBulbs(RE::NiPoint3 a_origin, float a_range)
{
	return;
	/*
    ForEachCellInRange(a_origin, a_range, [&](const RE::TESObjectCELL* a_cell) 
	{
        a_cell->ForEachReferenceInRange(a_origin, a_range, [&](RE::TESObjectREFR* a_ref) 
		{
            for (const auto& data : a_ref->extraList) 
			{
                auto extra = &a_ref->extraList;

				RE::ExtraLightData* lightBulb = extra->GetByType(RE::ExtraDataType::kLightData);

                for (auto& data : a_ref->extraList) 
				{
                    if (data.GetType() == RE::ExtraDataType::kLightData) 
					{
						RE::ExtraLightData lightData = static_cast<RE::ExtraLightData>(&data);
						RE::NiPoint3 position = a_ref->GetPosition();
						std::vector<RE::NiPoint3> dummyPolygon{ position, position, position };

						std::string infoStr = "LIGHT BULB INFO:";
						infoStr += fmt::format("\nFOV: {}", data.);


						// draw invisible dummy polygon to able to show info since info can only be shown by looking at polygon corners
						g_DrawHandler->DrawPolygon(dummyPolygon, 0, 0x000000, 0, 0, infoStr);
                        g_DrawHandler->DrawPoint(, 15, 0xFCC019, 50);
                        logger::info("Lightbulb found at: {}, {}, {}", a_ref->GetPosition().x, a_ref->GetPosition().y, a_ref->GetPosition().z);
                    }
				}
            }
			return RE::BSContainer::ForEachResult::kContinue;
        });
    });
	*/
}

std::string DebugHandler::GetNavmeshInfo(RE::FormID a_formID, const RE::TESObjectCELL* a_cell)
{
	std::string infoStr{ "CELL INFO" };

	if (!a_cell || a_cell->GetFormID() >> 24 == 0xFF)
	{
		infoStr += "\nNot available";

	}
	else
	{
		infoStr += "\nEditor ID: ";
		const char* editorID = a_cell->GetFormEditorID();
		if (editorID) infoStr += editorID;
		else infoStr += "Not available";

		infoStr += fmt::format("\nForm ID: {:08X}", a_cell->GetFormID());
		if (a_cell->IsExteriorCell())
		{
			infoStr += "\nCoordinates: ";
			infoStr += std::to_string(a_cell->GetRuntimeData().cellData.exterior->cellX);
			infoStr += ", ";
			infoStr += std::to_string(a_cell->GetRuntimeData().cellData.exterior->cellY);
		}
	}

	infoStr += "\n\nNAVMESH INFO";
	infoStr += fmt::format("\nForm ID: {:08X}", a_formID);

	infoStr += std::string("\nReferenced by (list may be incomplete): ");

	for (const auto& fileName : sourceFilesOrdered[a_formID])
	{
		infoStr += "\nMod: ";
		infoStr += std::string(fileName);
	}
	return infoStr;
}

std::string DebugHandler::GetOcclusionInfo(const RE::TESObjectCELL* a_cell, RE::TESObjectREFR* a_ref, RE::NiPoint3 a_bounds, bool a_isDisabled)
{
	RE::FormID formID = a_ref->GetFormID();

	std::string infoStr{ "CELL INFO" };
	if (!a_cell || a_cell->GetFormID() >> 24 == 0xFF)
	{
		if (const auto* TES = RE::TES::GetSingleton(); TES)
		{
			a_cell = TES->GetCell(a_ref->GetPosition());
		}
		
		if (a_cell->GetFormID() >> 24 == 0xFF)
		{
			infoStr += "\nNot available";
		}
	}
	if (a_cell)
	{
		infoStr += "\nEditor ID: ";
		infoStr += a_cell->GetFormEditorID();
		infoStr += fmt::format("\nForm ID: {:08X}", a_cell->GetFormID());

		infoStr += "\nCoordinates: ";
		infoStr += std::to_string(a_cell->GetRuntimeData().cellData.exterior->cellX);
		infoStr += ", ";
		infoStr += std::to_string(a_cell->GetRuntimeData().cellData.exterior->cellY);
	}

	infoStr += "\n\nPLANEMARKER INFO";
	if (a_isDisabled) infoStr += "\n\nPlanemarker currently disabled";
	infoStr += fmt::format("\nForm ID: {:08X}", formID);
	infoStr += fmt::format("\nPosition: {:.0f}, {:.0f}, {:.0f}", a_ref->GetPositionX(), a_ref->GetPositionY(), a_ref->GetPositionZ());
	infoStr += fmt::format("\nBounds: {:.0f}, {:.0f}, {:.0f}", a_bounds.x, a_bounds.y, a_bounds.z);

	infoStr += std::string("\nReferenced by:");
	for (const auto& fileName : GetSouceFiles(a_ref))
	{
		infoStr += "\nMod: ";
		infoStr += std::string(fileName);
	}
	return infoStr;
}

std::string DebugHandler::GetQuadInfo(const RE::TESObjectCELL* a_cell, uint8_t a_quad)
{
	std::string infoStr{ "CELL INFO" };

	if (!a_cell || a_cell->GetFormID() >> 24 == 0xFF)
	{
		infoStr += "\nNot available";
	}
	else
	{
		infoStr += "\nEditor ID: ";
		const char* editorID = a_cell->GetFormEditorID();
		if (editorID) infoStr += editorID;
		else infoStr += "Not available";

		infoStr += fmt::format("\nForm ID: {:08X}", a_cell->GetFormID());
		if (a_cell->IsExteriorCell())
		{
			;
			infoStr += "\nCoordinates: ";
			infoStr += std::to_string(a_cell->GetRuntimeData().cellData.exterior->cellX);
			infoStr += ", ";
			infoStr += std::to_string(a_cell->GetRuntimeData().cellData.exterior->cellY);
		}
	}

	infoStr += "\n\nQUAD INFO:";
	infoStr += fmt::format("\nQuad nummber: {}", a_quad + 1);

	int i = 0;
	for (const auto texture : a_cell->GetRuntimeData().cellLand->loadedData->quadTextures[a_quad])
	{
		i++;
		if (texture)
		{
			infoStr += fmt::format("\nTexture set {}", i);
			auto textureSet = texture->textureSet;
			if (textureSet)
			{
				for (const auto& texture_ : textureSet->textures)
				{
					if (!texture_.textureName.empty())
					{
						infoStr += "\n  ";
						infoStr += texture_.textureName;
					}
				}
			}
		}
	}
		
	return infoStr;
}

std::vector<std::string_view> DebugHandler::GetSouceFiles(RE::TESForm* a_form)
{
	std::vector<std::string_view> fileNames;

	RE::TESFile** files = a_form->sourceFiles.array->data();
	int numberOfFiles = a_form->sourceFiles.array->size();

	// yoinked from more informative console sauce https://github.com/Liolel/More-Informative-Console/blob/1613cda4ec067e86f97fb6aae4a7c85533afe031/src/Scaleform/MICScaleform_GetReferenceInfo.cpp#L57
	if ((a_form->GetFormID() >> 24) == 0x00)  //Refs from Skyrim.ESM will have 00 for the first two hexidecimal digits
	{								 //And refs from all other mods will have a non zero value, so a bitwise && of those two digits with FF will be nonzero for all non Skyrim.ESM mods
		if (numberOfFiles == 0 || std::string(files[0]->fileName) != "Skyrim.esm")
		{
			fileNames.push_back("Skyrim.esm");
		}
	}
	for (int i = 0; i < numberOfFiles; i++)
	{
		fileNames.push_back(files[i]->GetFilename());
	}
	return fileNames;
}

float DebugHandler::GetLightLevel()
{
	float nightMultiplier = 0.5;
	switch (MCM::settings::dayNightIndex)
	{
		case 0: //day mode
		{
			return 1;
		}
		case 1: // night mode
		{
			return nightMultiplier;
		}
		case 2:
		{
			// should be accurate for at least 100000000000 days
			float gameTime = RE::Calendar::GetSingleton()->GetCurrentGameTime(); // 0, 1, 2, ... at midnight, 0.5, 1.5, ... at noon
			float scale = 1 - nightMultiplier;
			float PI = 3.14159265359;
			return (-scale * cosf(2* gameTime * PI) + 1 + nightMultiplier)/2;
		}
	}
}

void DebugHandler::SizeofCache()
{
	unsigned long size = sizeof(cachedNavmeshes);

	int cellCount = 0;
	int triangleCount = 0;
	int vertexCount = 0;
	int extraEdgeInfoCount = 0;

	int triangleCapacity = 0;
	int vertexCapacity = 0;
	int extraEdgeInfoCapacity = 0;

	std::map<RE::FormID, std::vector<NavmeshInfo>>::iterator it;
	for (it = cachedNavmeshes.begin(); it != cachedNavmeshes.end(); it++)
	{
		cellCount ++;

		size += sizeof(it->first);
		for (const auto& item : it->second)
		{
			//size += sizeof(item);

			triangleCount += item.triangles.size();
			vertexCount += item.vertices.size();
			extraEdgeInfoCount += item.extraEdgeInfo.size();

			triangleCapacity += item.triangles.capacity();
			vertexCapacity += item.vertices.capacity();
			extraEdgeInfoCapacity += item.extraEdgeInfo.capacity();

		}
	}

	int allocSize = size;

	size += triangleCount*sizeof(RE::BSNavmeshTriangle);
	size += vertexCount*sizeof(RE::BSNavmeshVertex);
	size += extraEdgeInfoCount*sizeof(RE::BSNavmeshEdgeExtraInfo);

	allocSize += triangleCapacity*sizeof(RE::BSNavmeshTriangle);
	allocSize += vertexCapacity*sizeof(RE::BSNavmeshVertex);
	allocSize += extraEdgeInfoCapacity*sizeof(RE::BSNavmeshEdgeExtraInfo);

	logger::info("Cache details:");
	logger::info("  Used Memory size (approximately):      {:>10.3f} MB ", size/1000000.f);
	logger::info("  Allocated Memory size (approximately): {:>10.3f} MB", allocSize/1000000.f);
	logger::info("  # of cells:      {:>10}", cellCount);
	logger::info("  # of trianlges:  {:>10}", triangleCount);
	logger::info("  # of vertices:   {:>10}", vertexCount);
	logger::info("  # of extra info: {:>10}", extraEdgeInfoCount);

	/*
	IN VANILLA:
	* Running from whiterun to solitude loads ~425 cells.
	> a cell has ~400 triangles on average
	> a cell has ~300 vertices on average
	> these 425 cells' navmeshes takes up about 4.5 MB of memory (with a few percent allocated overhead)
	> skyrim has about 11000 cells (i think). Thats about 115 mb (or the equivalent six 4k textures)
	*/
}

/*
class BGSPrimitive
{
	// the values are half the bounding box. neat when the center is known
	float bounds[3];	//0C
};

class ExtraPrimitive
{
	BGSPrimitive* primitive; //10
};


struct BSNavmeshTriangle
{
	public:
		enum class TriangleFlag
		{
			kNone = 0,
			kEdge0_Link = 1 << 0,
			kEdge1_Link = 1 << 1,
			kEdge2_Link = 1 << 2,
			kDeleted = 1 << 3,
			kNoLargeCreatures = 1 << 4,
			kOverlapping = 1 << 5,
			kPreferred = 1 << 6,
			kWater = 1, << 9,
			kDoor = 1, << 10,
			kInFile = 1 << 11,
		};


		.
		.
		.

		// members
		std::uint16_t                                  vertices[3];     // 00
		std::uint16_t                                  triangles[3];    // 06 - 0xFFFF == NONE
		stl::enumeration<TriangleFlag, std::uint16_t>  triangleFlags;   // 0C
		stl::enumeration<TraversalFlag, std::uint16_t> traversalFlags;  // 0E


		// if edge0 is linked to another triangle, the index of triangles[0] refers to the element in BSTArray<BSNavmeshEdgeExtraInfo>
		// ie, to get the triangle index, do navmesh->extraEdgeInfo[triangle[0]].portal.triangle
		// however, this triangle may be in another navmesh, which has the ID navmesh->extraEdgeInfo[triangle[0]].portal.otherMeshID;
		//
		// if the new triangle belongs to a navmesh in another cell, the link is a cell link
*/