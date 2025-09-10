#include "DebugHandler.h"
#include "DrawMenu.h"
#include "UIHandler.h"
#include "DrawHandler.h"
#include "MCM.h"
#include "Utils.h"

void DebugHandler::Init()
{
	g_DrawHandler = DrawHandler::GetSingleton();
	g_UI = UIHandler::GetSingleton();

	auto modelName = "marker_light.nif"s;
	auto shapeName = "marker_light:0"s;

	CopyEffectShaderMaterial("marker_light.nif"s, "marker_light:0"s, lightBulbMaterial);

	logger::debug("Initialized DebugHandler");
}

void DebugHandler::CopyEffectShaderMaterial(const std::string& a_filepath, const std::string& a_shapeName, RE::BSEffectShaderMaterial*& a_materialOut)
{
	RE::NiPointer<RE::NiNode> model;
	RE::BSModelDB::DBTraits::ArgsType args;
	RE::BSResource::ErrorCode errorCode = RE::BSModelDB::Demand(a_filepath.c_str(), model, args);
	if (errorCode == RE::BSResource::ErrorCode::kNone && model)
	{
		if (auto shape = model->AsNode()->GetObjectByName(a_shapeName))
		{
			if (auto shaderProperty = netimmerse_cast<RE::BSEffectShaderProperty*>(shape->AsGeometry()->GetGeometryRuntimeData().properties[RE::BSGeometry::States::kEffect].get()))
			{
				// inspired by https://github.com/powerof3/LightPlacer/blob/830e15d7e5fa1fbccb7427852024458564eccc0b/src/LightData.cpp#L396
				if (const auto shaderMaterial = static_cast<RE::BSEffectShaderMaterial*>(shaderProperty->material))
				{
					if (a_materialOut = static_cast<RE::BSEffectShaderMaterial*>(shaderMaterial->Create()))
					{
						a_materialOut->CopyMembers(shaderMaterial);
					}
				}
			}
		}
	}
}

// usually <1 µs per file, sometimes ~10 and very rarely 20.
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

void DebugHandler::OnCellFullyLoaded(RE::TESObjectCELL* a_cell)
{
	if (!a_cell) return;

	CacheCellNavmeshes(a_cell);
	//CreateDoorTeleportMarkers(a_cell);
}

// if a navmesh is not referenced by a new mod, sometimes a_cell->GetRuntimeData().navMeshes will not be defined,
// but it can still be found in the nav mesh load
// 
// on the other hand, on navmesh load, the source array will only display one of the base game (+dlc) .esps (i think), 
// but on cell load, new mods can also be found, though it wont always find eg. heathfires.esm when the navmesh is defined in
// skyrim.esm, but these are somethimes caught on navmesh load

// usually <1 µs per file, sometimes ~10 and very rarely 20.
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
	RE::NiPoint3 playerPos = RE::PlayerCharacter::GetSingleton()->GetPosition();
	if (MCM::settings::showCellBorders) DrawCellBorders();
	if (MCM::settings::showNavmesh) DrawNavmesh(playerPos, MCM::settings::navmeshRange);
	if (MCM::settings::showOcclusion) DrawOcclusion(playerPos, MCM::settings::occlusionRange);
	if (MCM::settings::showMarkers) DrawMarkers(playerPos, MCM::settings::markersRange);
	else if (visibleMarkers.size() > 0) HideAllMarkers();
	/*for (int i = 0; i < 10; i++)
	{
		if (MCM::settings::showMarkers) DrawMarkers(playerPos, MCM::settings::markersRange);
	}*/
    //DrawLightBulbs(RE::PlayerCharacter::GetSingleton()->GetPosition(), 4000.0f);
	DrawTest();
	//GetLandscapeHeightAtLocation(RE::PlayerCharacter::GetSingleton()->GetPosition(), RE::PlayerCharacter::GetSingleton()->parentCell);
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
	for (const auto& pos : debugPoints)
	{
		g_DrawHandler->DrawPoint(pos, 300.0, 0xFF0000, 100);
	}
}

bool DebugHandler::isAnyDebugON()
{
	return MCM::settings::showCellBorders || MCM::settings::showNavmesh || MCM::settings::showOcclusion || MCM::settings::showCoordinates || MCM::settings::showMarkers;
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

void DebugHandler::ResetUpdateTimer()
{
	timeSinceLastUpdate = 1000.0f;
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

	if (const auto cellCoords = cell->GetCoordinates())
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
		
		for (auto& navmesh : cachedNavmeshes[cellID])
		{
			auto& vertices = navmesh.vertices;
			auto& triangles = navmesh.triangles;

			std::string infoStr = GetNavmeshInfo(a_cell, navmesh.formID);

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
	float heightAbove = 0.0f; 
	float heightBelow = 0.0f;
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

void DebugHandler::DrawMarkers(RE::NiPoint3 a_origin, float a_range)
{
	// Debug duplicates
	/*
	int numberOfDuplicates = 0;
	std::map<RE::FormID, uint32_t> formIDcounts;
	for (const auto& marker : visibleMarkers)
	{
		if (formIDcounts.contains(marker->ref->formID))
		{
			formIDcounts[marker->formID]++;
			logger::info("  {:X}", marker->formID);
			numberOfDuplicates++;
		}
		else
			formIDcounts[marker->formID] = 1;
	}

	logger::info("currently {}/{} markers are drawn. {} Duplicates", visibleMarkers.size(), visibleMarkersIDs.size(), numberOfDuplicates);
	//*/
	//logger::info("currently {} markers are drawn", visibleMarkers.size());


	if (MCM::settings::updateVisibleMarkers)
	{
		MCM::settings::updateVisibleMarkers = false;
		HideAllMarkers();

		#ifdef TRACEOBJECTS
			logger::debug("Update visible markers");
		#endif
	}

	for (int i = visibleMarkers.size() - 1; i > -1; i--)
	{
		std::unique_ptr<Marker>& marker = visibleMarkers[i];

		if (!IsMarkerLoaded(marker)) // whenever worldspaces change, the reference is still loaded in memory, and may still be in range, but there is no reason to keep them in the list
		{
			HideMarker(i);
			continue;
		}

		float dx = a_origin.x - marker->ref->GetPositionX();
		float dy = a_origin.y - marker->ref->GetPositionY();
		
		float range = a_range;
		if (marker->drawWhenFar) range = drawWhenFarRange;

		if (dx*dx + dy*dy > range * range)
		{
			HideMarker(i);
		}
	}

	ForEachCellInRange(a_origin, drawWhenFarRange, [&](const RE::TESObjectCELL* a_cell)
	{
		a_cell->ForEachReference([&](RE::TESObjectREFR* ref)
		{
			if (!ref) return RE::BSContainer::ForEachResult::kContinue;
			
			#ifdef TRACEOBJECTS

				auto base = ref->GetBaseObject();
				auto pos = ref->GetPosition();
				
				if (ref->formID == debugFormID)
				{
					logger::debug("");
					logger::debug("Tracing object {:X} edid: <{}> address: {:X}", ref->formID, ref->GetFormEditorID(), reinterpret_cast<uintptr_t>(ref));
					logger::debug(" |-IsInitiallyDisabled? {}; Disabled? {}", ref->IsInitiallyDisabled(), ref->IsDisabled());
					logger::debug(" |-Base object: {:X} edid: <{}>", base->formID, base->GetFormEditorID());
					logger::debug(" |-Cell: {:X} edid: <{}> address: {:X}", a_cell->formID, a_cell->GetFormEditorID(), reinterpret_cast<uintptr_t>(a_cell));
					logger::debug(" |-|-Cell game flags: {:016b}", a_cell->cellGameFlags);
					logger::debug(" |-|-Cell state: {:08b}", a_cell->cellState.underlying());
					logger::debug(" |-|-Cell form flags {:032b}", a_cell->formFlags);
					logger::debug(" |-|-Cell form flags {:016b}", a_cell->inGameFormFlags.underlying());
					logger::debug(" |-|-Cell is initialized? {}", a_cell->IsInitialized());
				} 
			#endif


			bool shouldMarkerBeDrawnWhenFar = ShouldMarkerBeDrawnWhenFar(ref);

			float range = a_range;
			if (shouldMarkerBeDrawnWhenFar) range = drawWhenFarRange;

			float dx = a_origin.x - ref->GetPositionX();
			float dy = a_origin.y - ref->GetPositionY();

			if (dx * dx + dy * dy > range * range) return RE::BSContainer::ForEachResult::kContinue;
			
			#ifdef TRACEOBJECTS
				if (ref->formID == debugFormID) logger::debug(" |-Object withing range");
			#endif

			uint32_t numberOfDrawnMarkers = visibleMarkers.size();

			ShowMarker(ref);

			
			/*for (int i = 0; i < 1000; i++)
			{
				ShouldMarkerBeDrawnWhenFar(ref);
			}
			logger::info()*/

			if (shouldMarkerBeDrawnWhenFar && visibleMarkers.size() > numberOfDrawnMarkers)
			{
				visibleMarkers[numberOfDrawnMarkers]->drawWhenFar = true;
			}

			return RE::BSContainer::ForEachResult::kContinue;
		});
	});

}

void DebugHandler::AddVisibleMarker(RE::TESObjectREFR* a_ref, RE::BSFixedString a_markerName, bool a_cullWhenHiding)
{
	#ifdef TRACEOBJECTS
		if (a_ref->formID == debugFormID) 
		{
			debugMarkerIndex = visibleMarkers.size();
			logger::debug(" |-|-AddVisibleMarker; Marker name: {}; Marker index = {}", a_markerName, debugMarkerIndex);
			
		}
	#endif
	//logger::info("Adding marker {:X} {} at index {}", a_ref->formID, a_ref->Get3D()->name, visibleMarkers.size());

	visibleMarkers.push_back(std::make_unique<Marker>(a_ref, a_markerName));
	visibleMarkers.back().get()->cullWhenHiding = a_cullWhenHiding;
	visibleMarkersIDs.insert(a_ref->formID);
}

void DebugHandler::RemoveVisibleMarker(uint32_t a_markerIndex)
{
	visibleMarkersIDs.erase(visibleMarkers[a_markerIndex]->formID);
	visibleMarkers.erase(visibleMarkers.begin() + a_markerIndex);
}

void DebugHandler::HideAllMarkers()
{
	for (int i = visibleMarkers.size() - 1; i > -1; i--)
	{
		HideMarker(i);
	}
}

void DebugHandler::HideMarker(uint32_t a_markerIndex)
{
	#ifdef TRACEOBJECTS
		if (a_markerIndex == debugMarkerIndex)
		{
			logger::debug(" |-Hiding Marker; MarkerIndex: {}; VisibleMarkersSize? {}", a_markerIndex, visibleMarkers.size());
			if (a_markerIndex < visibleMarkers.size())
			{
				logger::debug(" |-|-IsMarkerLoaded? {}", IsMarkerLoaded(visibleMarkers[a_markerIndex]));
			}
		}
	#endif

	if (a_markerIndex >= visibleMarkers.size()) return;

	std::unique_ptr<Marker>& marker = visibleMarkers[a_markerIndex];



	if (!IsMarkerLoaded(marker))
	{
		RemoveVisibleMarker(a_markerIndex);
		return;
	}

	marker->SetDefaultState();

	#ifdef TRACEOBJECTS
		if (marker->ref->formID == debugFormID) 
		{
			logger::debug(" |-Hiding Marker. Culltree after setting default:");
			marker->PrintCullTree(" |-|-");
		}
	#endif
	
	RemoveVisibleMarker(a_markerIndex);
	
}

void DebugHandler::ShowMarker(RE::TESObjectREFR* a_ref)
{
	#ifdef TRACEOBJECTS
		if (a_ref->formID == debugFormID) logger::debug(" |-Showing object");
	#endif

	if (auto baseObject = a_ref->GetBaseObject())
	{
	#ifdef TRACEOBJECTS
		if (a_ref->formID == debugFormID) 
		{
			logger::debug(" |-|-Formtype: {}", baseObject->GetFormType());
			logger::debug(" |-|-ShouldBeDrawn: {}", ShouldMarkerBeDrawn(a_ref));
		}
	#endif
		// if the object itself is the marker (like x-markers), call ShouldMarkersBeDrawn first
		// else, if the marker is attached to the object, check the setting inside the corresponding ShowHide<...>Marker function (typically added to the "if (!a_show) return;")
		switch (baseObject->GetFormType())
		{
			case RE::FormType::Furniture :
			{
				if (MCM::settings::GetShowFurnitureMarkers())
					ShowFurnitureMarker(a_ref);
				break;
			}
			case RE::FormType::Light:
			{
				if (MCM::settings::GetShowLightMarkers())
					ShowLightMarker(a_ref);
				break;
			}
			case RE::FormType::Sound:
			{
				if (MCM::settings::GetShowSoundMarkers())
					ShowSoundMarker(a_ref);
				break;
			}
			case RE::FormType::Door:
			{
				if (MCM::settings::GetShowDoorTeleportMarkers())
					ShowDoorTeleportMarker(a_ref);
				break;
			}
			case RE::FormType::MovableStatic :
			case RE::FormType::IdleMarker :
			case RE::FormType::Activator :
			case RE::FormType::NPC :
			case RE::FormType::Hazard :
			case RE::FormType::TextureSet :
			case RE::FormType::Static :
			{
				if (ShouldMarkerBeDrawn(a_ref))
				{
					#ifdef TRACEOBJECTS
						if (a_ref->formID == debugFormID) logger::debug(" |-Calling ShowHideStaticMarker");
					#endif
					ShowStaticMarker(a_ref);
				}
				break;
			}
		}
	}
}

// Main 3D should have its world pos = its local pos = the position it should be
// its children can then have local pos = 0 if its ontop of the main 3D, otherwise an offset

DebugHandler::MarkerInfo DebugHandler::GetMarkerInfo(RE::TESObjectREFR* a_ref)
{
	MarkerInfo info;

	auto baseObject = a_ref->GetBaseObject();

	if (baseObject->GetFormType() == RE::FormType::Sound)
	{
		info.path = "marker_sound.nif";
		info.name = "SoundMarkerVis";
	}

	std::string path = ""s;

	switch (baseObject->GetFormType())
	{
		case RE::FormType::MovableStatic :
			if (const auto& movableStatic = baseObject->As<RE::BGSMovableStatic>()) path = movableStatic->model;
			break;
		case RE::FormType::IdleMarker :
			if (const auto& idleMarker = baseObject->As<RE::BGSIdleMarker>()) path = idleMarker->model;
			break;
		case RE::FormType::Activator :
			if (const auto& activator = baseObject->As<RE::TESObjectACTI>()) path = activator->model;
			break;
		case RE::FormType::Hazard :
			if (const auto& hazard = baseObject->As<RE::BGSHazard>()) path = hazard->model;
			break;
		case RE::FormType::Static :
			if (const auto& stat = baseObject->As<RE::TESObjectSTAT>()) path = stat->model;
			break;
	}
	if (!path.empty())
	{
		info.path = path;
		std::string filename = path;
		if (path.rfind("\\") != std::string::npos)
		{
			filename = std::string(path.substr(path.rfind("\\") + 1));
		}
		info.name = filename.substr(0, filename.size()-4); // remove '.nif' from name
	}

	//switch (baseObject->GetFormID())
	//{
	//	case 0x1C035 : // ComplexSceneMARKER
	//		info.path = "markers\\misc\\complexscenemarker01.nif";
	//		info.name = "ComplexSceneMarker";
	//		break;
	//	case 0x57A8C : // FireLgPlacedHazard
	//		info.path = "effects\\fxfire01new.nif";
	//		info.name = "FireLgPlacedHazardMarker";
	//		break;
	//}

	return info;
}

void DebugHandler::Marker::SetDefaultState()
{
	if (auto obj = ref->Get3D())
	{
		//Utils::SetNodeTreeDefaultAppCull(obj);

		if (auto node = obj->AsNode())
		{
			Utils::DetachChildrenByName(ref->Get3D()->AsNode(), markerName);

			bool cullNode = true;
			bool cullEditorMarker = true;
			if(!cullWhenHiding) cullNode = false;
			Utils::CullNode(node, cullNode, cullEditorMarker); // MUST be after children have been detached
		}

		
	}
}

RE::NiNode* DebugHandler::TrySet3DByName(RE::TESObjectREFR* a_ref, const char* a_modelName, const RE::BSFixedString a_markerName)
{
	RE::NiPointer<RE::NiNode> markerModel_;
	RE::BSModelDB::DBTraits::ArgsType args;
	RE::BSResource::ErrorCode errorCode = RE::BSModelDB::Demand(a_modelName, markerModel_, args);

	if (errorCode != RE::BSResource::ErrorCode::kNone || !markerModel_ || !markerModel_.get()) return nullptr;

	auto clone = markerModel_->Clone();

	clone->name = a_markerName;
	RE::NiNode* node = clone->AsNode();

	if (!node) return nullptr;

	a_ref->Set3D(clone, true);
	a_ref->Update3DPosition(true);
	clone->CullNode(true); // node must be culled by default, so the culltree will know that it should be hidden when deactivated

	if (!node->parent)
	{
		auto cellStaticNode = Utils::GetCellStaticNode(a_ref->GetParentCell());
		if (cellStaticNode)
		{
			Utils::AttachChildNode(cellStaticNode, node);
		}
	}

	return node;

}

RE::NiNode* DebugHandler::GetNodeFromRef(RE::TESObjectREFR* a_ref)
{
	if (auto obj = a_ref->Get3D())
	{
		if (auto node = obj->AsNode())
		{
		#ifdef TRACEOBJECTS
			if (a_ref->formID == debugFormID) logger::debug(" |-|-|-ref has 3D");
		#endif
			return node;
		}
	}
	else
	{
		MarkerInfo info = GetMarkerInfo(a_ref);
		if (!info.path.empty())
		{
		#ifdef TRACEOBJECTS
			if (a_ref->formID == debugFormID) logger::debug(" |-|-|-getting ref 3D from path: {}; name: {}", info.path, info.name);
		#endif
			return TrySet3DByName(a_ref, info.path.c_str(), info.name);
		}
	}
	#ifdef TRACEOBJECTS
		if (a_ref->formID == debugFormID) logger::debug(" |-|-ref has no 3D");
	#endif
	return nullptr;
}

bool DebugHandler::IsMarkerLoaded(const std::unique_ptr<Marker>& a_marker)
{
	return a_marker->ref && a_marker->ref.get() && a_marker->ref->parentCell && a_marker->ref->Is3DLoaded();
}

bool DebugHandler::ShowNodeIfNeeded(RE::TESObjectREFR* a_ref)
{
	auto node = a_ref->Get3D()->AsNode();
	if (ShouldMarkerBeDrawn(a_ref))
	{
		Utils::CullNode(node, false);
		return true;
	}
	return false;
}

void DebugHandler::ShowFurnitureMarker(RE::TESObjectREFR* a_ref)
{
	#ifdef TRACEOBJECTS
		if (a_ref->formID == debugFormID) logger::debug(" |-|-ShowFurnitureMarker");
	#endif
	
	auto node = GetNodeFromRef(a_ref);
	if (!node) return;

	#ifdef TRACEOBJECTS
		if (a_ref->formID == debugFormID)
		{
			logger::debug(" |-|-|-node? {}; HasChildrenOfName({})? {}", node ? true : false, furnitureMarkerName, Utils::HasChildrenOfName(node, furnitureMarkerName));
			logger::debug(" |-|-|-|-Name: {}", node->name);

		}
	#endif

	RE::NiExtraData* furnitureMarkerNode_ = a_ref->Get3D()->GetExtraData("FRN");
	RE::BSFurnitureMarkerNode* furnitureMarkerNode = reinterpret_cast<RE::BSFurnitureMarkerNode*>(furnitureMarkerNode_);
	if (!furnitureMarkerNode) return;

	uint8_t markerType = 0;

	if (!Utils::HasChildrenOfName(node, furnitureMarkerName))
	{
		for (const auto& marker : furnitureMarkerNode->markers)
		{
			RE::TESObjectSTAT* markerStatic = nullptr;

			if (marker.animationType.any(RE::BSFurnitureMarker::AnimationType::kSit) && MCM::settings::GetShowSitMarkers())
			{
				markerStatic = RE::TESForm::LookupByID<RE::TESObjectSTAT>(0x64);
				markerType |= 0b001;
			}

			else if (marker.animationType.any(RE::BSFurnitureMarker::AnimationType::kSleep) && MCM::settings::GetShowSleepMarkers())
			{
				markerStatic = RE::TESForm::LookupByID<RE::TESObjectSTAT>(0x65);
				markerType |= 0b010;
			}

			else if (marker.animationType.any(RE::BSFurnitureMarker::AnimationType::kLean) && MCM::settings::GetShowLeanMarkers())
			{
				markerStatic = RE::TESForm::LookupByID<RE::TESObjectSTAT>(0x66);
				markerType |= 0b100;
			}

			if (!markerStatic) continue;

			RE::BSFixedString markerModelPath = markerStatic->model;

			if (markerModelPath.empty()) continue;

			RE::NiAVObject* markerModel = nullptr;


			auto pos = a_ref->GetPosition();

			if (!Utils::TryAttachChildByName(node, markerModelPath.c_str(), furnitureMarkerName, markerModel)) continue;

			//RE::NiUpdateData updateData;
			//updateData.flags = static_cast<RE::NiUpdateData::Flag>(0x2); // seems to be flagged 2 when run by the game
			//updateData.time = 0.0f;

			//markerModel->UpdateDownwardPass(updateData, 0); // necessary for most markers to show (and to get proper transforms), args = 0

			markerModel->local.translate = marker.offset;
			markerModel->local.rotate = RE::NiMatrix3(0.0f, 0.0f, marker.heading);

			// apparently, not all local transformations move the marker mesh, unless this is called :^)
			a_ref->InitNonNPCAnimation(*markerModel->AsNode());

			/*
			// draw cubes

			RE::NiAVObject* markerModel2 = nullptr;
			if (!TryAttachChildByName(node, "markers\\rigidbodydummy.nif", furnitureMarkerName, markerModel2)) continue;
			if (!markerModel2) continue;
			markerModel2->GetObjectByName("EditorMarker")->CullNode(false);
			markerModel2->local.translate = marker.offset;
			markerModel2->local.rotate = RE::NiMatrix3(45.0f, 35.0f, 30.0f);
			a_ref->InitNonNPCAnimation(*markerModel2->AsNode()); // the square moves to the bounds of the ref (or not)

			//a_ref->Set3D(markerModel->Clone());
			//a_ref->Update3DPosition(true);
			*/

		}
	}

	uint32_t numberOfAttachedMarkers = Utils::HowManyChildrenOfName(node, furnitureMarkerName);

	#ifdef TRACEOBJECTS
		if (a_ref->formID == debugFormID) 
		{
			logger::debug(" |-|-|-HasAttachedMarkers? {}; does marker already exist? {}; makeCullTreeFromFile? {}", hasAttachedMarkers, markerAlreadyExist, makeCullTreeFromFile);
		}
	#endif

	if (numberOfAttachedMarkers > 0 && !visibleMarkersIDs.contains(a_ref->formID))
	{
		//bool isNodeShown = ShowNodeIfNeeded(a_ref); // check if marker should be drawn (such as wallLeanMarker)
		bool cullNodeWhenHiding = false;
		if (node->GetObjectByName("EditorMarker"))
		{
			// If the EditorMarker is the only node (besides the attached furniture marker nodes) then the main node
			// should be hidden, when the EditorMarker is culled, otherwise not.
			
			if (node->GetChildren().size() == 1 + numberOfAttachedMarkers) 
			{
				//if (node->GetAppCulled() == false) logger::info("Object {:X} is visible but will be culled when hiding", a_ref->formID);
				cullNodeWhenHiding = true;
			}
			Utils::CullNode(node, false);


			
		}
		AddVisibleMarker(a_ref, furnitureMarkerName, cullNodeWhenHiding);
		
	}


}

void DebugHandler::ShowStaticMarker(RE::TESObjectREFR* a_ref)
{
	RE::NiNode* node = GetNodeFromRef(a_ref);
	if (!node) return;
	
	auto baseObject = a_ref->GetBaseObject();
	
	#ifdef TRACEOBJECTS
		if (a_ref->formID == debugFormID) 
		{
			logger::debug(" |-|-Inside ShowHideStaticMarker");
			logger::debug(" |-|-IsNodeVisible: {}", Utils::IsNodeVisible(node));

		}
	#endif
	

	if(!visibleMarkersIDs.contains(a_ref->formID))
	{
		Utils::CullNode(node, false);
		AddVisibleMarker(a_ref);
	}
	return;
}

void DebugHandler::ShowLocRefType(RE::TESObjectREFR* a_ref)
{
	/*RE::BSFixedString locationRefTypeMarkerName = "LocRefTypeMarkerVis";

	if (HasMarkers(node, locationRefTypeMarkerName))
	{
		if (a_visibility == Visibility::kHide) DetachChildrenByName(node, locationRefTypeMarkerName);
	}
	else
	{
		if (a_visibility == Visibility::kShow )
		{
			if (auto editorLocation = a_ref->GetEditorLocation(); editorLocation && !editorLocation->specialRefs.empty())
			{
				auto* locationRefTypePtr = &editorLocation->specialRefs[0];
			#ifdef TRACEOBJECTS
				if (a_ref->formID == debugFormID) logger::info(" |-|-|-Location Ref Type address: {:X}", reinterpret_cast<uintptr_t>(locationRefTypePtr));
			#endif
				auto locationRefType = editorLocation->specialRefs[0].type->GetFormEditorID();
				RE::NiAVObject* cube;
				TryAttachChildByName(node, "markers\\rigidbodydummy.nif", locationRefTypeMarkerName, cube);
				cube->local.translate += RE::NiPoint3(0.0f, 0.0f, 20.0f);
				a_ref->InitNonNPCAnimation(*cube->AsNode());
			}
		}
	}*/
}

void DebugHandler::ShowSkyMarkerBeam(RE::TESObjectREFR* a_ref)
{
	//if (a_ref->formFlags & RE::TESObjectREFR::RecordFlags::kSkyMarker)
	//{
	//	auto node = a_ref->Get3D()->AsNode();
	//	if (HasMarkers(node, skyMarkerBeamName))
	//	{
	//		if (a_visibility == Visibility::kHide) DetachChildrenByName(node, skyMarkerBeamName);
	//		return;
	//	}
	//	if (a_visibility == Visibility::kShow)
	//	{
	//		RE::NiAVObject* skyMarkerBeam;
	//		TryAttachChildByName(node, "markers\\rigidbodydummy.nif", skyMarkerBeamName, skyMarkerBeam);
	//		skyMarkerBeam->local.translate += RE::NiPoint3(0.0f, 0.0f, -20.0f);
	//		
	//		float groundHeight = GetLandscapeHeightAtLocation(a_ref->GetPosition(), a_ref->parentCell);
	//		
	//		logger::info("{} {}", groundHeight, a_ref->GetPositionZ());

	//		if (groundHeight > zFloor && a_ref->GetPositionZ() > groundHeight)
	//		{
	//			float deltaHeight = a_ref->GetPositionZ() - groundHeight;
	//			float beamHeight = skyMarkerBeam->worldBound.radius*2;
	//			skyMarkerBeam->local.translate += RE::NiPoint3(0.0f, 0.0f, -deltaHeight/2);
	//			skyMarkerBeam->local.scale *= deltaHeight*beamHeight;
	//			// problem, as scale is not three dimensional

	//		}


	//		a_ref->InitNonNPCAnimation(*skyMarkerBeam->AsNode());
	//	}
	//}
}

void DebugHandler::ShowDoorTeleportMarker(RE::TESObjectREFR* a_ref)
{
	auto teleportDoorRef = a_ref->extraList.GetTeleportLinkedDoor().get();
	if (!teleportDoorRef) return;

	RE::ExtraTeleport* teleportExtraData = teleportDoorRef->extraList.GetByType<RE::ExtraTeleport>();
	if (!teleportExtraData) return;

	auto node = GetNodeFromRef(a_ref);
	if (!node) return;

	if (!Utils::HasChildrenOfName(node, doorTeleportMarkerName))
	{
		auto localPosition = teleportExtraData->teleportData->position - a_ref->GetPosition();
		auto localRotation = teleportExtraData->teleportData->rotation;

		RE::NiAVObject* markerModel = nullptr;
		if (!Utils::TryAttachChildByName(node, "MarkerTeleport.nif", doorTeleportMarkerName, markerModel)) return;

		auto parentRotation = a_ref->Get3D()->world.rotate;

		// When the marker is attached to the door, it will be rotated according to the doors rotation. So we inverse the rotation of the door here (R(theta)^-1 = R(theta)^T)
		markerModel->local.translate = (parentRotation.Transpose()*localPosition)/a_ref->GetScale();
		markerModel->local.rotate = parentRotation.Transpose()*RE::NiMatrix3(localRotation);
	}
	
	if (!visibleMarkersIDs.contains(a_ref->formID))
	{
		bool isNodeShown = ShowNodeIfNeeded(a_ref);
		AddVisibleMarker(a_ref, doorTeleportMarkerName, isNodeShown);
	}

	
}

void DebugHandler::ShowLightMarker(RE::TESObjectREFR* a_ref)
{
	if (MCM::settings::showInfoOnHover && MCM::settings::showMarkerInfo && MCM::settings::GetShowLightMarkers())
	{
		auto markerInfo = GetLightMarkerInfo(a_ref->parentCell, a_ref);
		auto infoPosition = a_ref->GetPosition();
		g_DrawHandler->DrawPolygon(std::vector<RE::NiPoint3>{ infoPosition, infoPosition, infoPosition}, 0.0f, 0, 0, 0, markerInfo);
		g_DrawHandler->DrawPoint(infoPosition, 20.0f, MCM::settings::lightBulbInfoColor, MCM::settings::markerInfoAlpha);
	}

	auto node = GetNodeFromRef(a_ref);
	if (!node) return;
	
	if (Utils::HasChildrenOfName(node, lightMarkerName)) return;
	
	RE::TESObjectLIGH* light = a_ref->GetBaseObject()->As<RE::TESObjectLIGH>();
	
	const char* modelName = "marker_light.nif";
	const char* shapeName = "marker_light:0";
	
	if (light)
	{
		if (light->data.flags.any(RE::TES_LIGHT_FLAGS::kHemiShadow)) // WhiterunDragonreachBasement
		{
			modelName = "marker_halfomni.nif";
			shapeName = "marker_halfomni:0";
		}
		else if (light->data.flags.any(RE::TES_LIGHT_FLAGS::kSpotShadow)) // PotemasCatacombs02
		{
			modelName = "marker_spotlight.nif";
			shapeName = "marker_spotlight:0";
		}
		else if (light->data.flags.any(RE::TES_LIGHT_FLAGS::kOmniShadow))
		{
			modelName = "marker_lightshadow.nif";
			shapeName = "marker_lightshadow:0";
		}
	}
	
	RE::NiAVObject* markerModel = nullptr;	
	if (!Utils::TryAttachChildByName(node, modelName, lightMarkerName, markerModel)) return;
	a_ref->InitNonNPCAnimation(*markerModel->AsNode());

	//some lights will not be visible without bounds
	node->UpdateWorldBound();

	if (!visibleMarkersIDs.contains(a_ref->formID))
	{
		AddVisibleMarker(a_ref, lightMarkerName, false);
	}

	if (auto shape = markerModel->AsNode()->GetObjectByName(shapeName))
	{
		if (auto shaderProperty = netimmerse_cast<RE::BSEffectShaderProperty*>(shape->AsGeometry()->GetGeometryRuntimeData().properties[RE::BSGeometry::States::kEffect].get()))
		{
			if (lightBulbMaterial)
			{
				shaderProperty->lastRenderPassState = std::numeric_limits<std::int32_t>::max();
				shaderProperty->SetMaterial(lightBulbMaterial, true);
				shaderProperty->SetupGeometry(shape->AsGeometry());
				shaderProperty->FinishSetupGeometry(shape->AsGeometry());
				shaderProperty->SetMaterialAlpha(MCM::settings::lightBulbAplha / 100.0f * lightBulbAlphaMax); // alpha of 1 is barely visible. above 2.25 it gets wacky	

			}
		}
		if (MCM::settings::lightBulbAplha > 99.5f && lightBulbMaterial)
		{
			if (auto alphaProperty = netimmerse_cast<RE::NiAlphaProperty*>(shape->AsGeometry()->GetGeometryRuntimeData().properties[RE::BSGeometry::States::kProperty].get()))
			{
				alphaProperty->SetAlphaBlending(false);
				alphaProperty->SetAlphaTesting(true);
				alphaProperty->alphaThreshold = 40;
			}
		}
	}
	
}

void DebugHandler::ShowSoundMarker(RE::TESObjectREFR* a_ref)
{
	if (MCM::settings::showInfoOnHover && MCM::settings::showMarkerInfo)
	{
		auto pos = a_ref->GetPosition();

		std::string info = GetSoundMarkerInfo(a_ref->parentCell, a_ref);

		g_DrawHandler->DrawPolygon(std::vector<RE::NiPoint3>{ pos, pos, pos }, 0.0f, 0x0, 0, 0, info);
		g_DrawHandler->DrawPoint(pos, 20.0f, MCM::settings::soundMarkerInfoColor, MCM::settings::markerInfoAlpha);
	}

	auto node = GetNodeFromRef(a_ref);

	if (!Utils::IsNodeTreeVisible(node) && !visibleMarkersIDs.contains(a_ref->formID))
	{
		node->CullNode(false);
		AddVisibleMarker(a_ref);
	}
}

RE::TESObjectCELL* DebugHandler::GetCellFromCoordinates(RE::NiPoint3 a_location)
{
	if (const auto* TES = RE::TES::GetSingleton(); TES)
	{
		if (TES->interiorCell)
		{
			return TES->interiorCell;
		}
		else if (const auto gridLength = TES->gridCells ? TES->gridCells->length : 0; gridLength > 0)
		{
			for (uint32_t x = 0; x < gridLength; x++)
			{
				for (uint32_t y = 0; y < gridLength; y++)
				{
					if (auto cell = TES->gridCells->GetCell(x, y); cell && cell->IsAttached())
					{
						if (const auto cellCoords = cell->GetCoordinates(); cellCoords) // cell coords are the coordinates of the south west corner
						{
							const RE::NiPoint2 worldPos{ cellCoords->worldX, cellCoords->worldY };
							if (a_location.x > worldPos.x && a_location.x < (worldPos.x + 4096.0f) && a_location.y > worldPos.y && a_location.y < (worldPos.y + 4096.0f)) // if some of the cell is in range
							{
								return cell;
							}
						}
					}
				}
			}
		}
		if (const auto ws = TES->GetRuntimeData2().worldSpace) 
		{
			if (auto skyCell = ws ? ws->GetSkyCell() : nullptr; skyCell) 
			{
				return skyCell;
			}
		}
	}
	return nullptr;
}

RE::TESObjectREFR* DebugHandler::InstantiateModel(const char* a_modelPath, const RE::NiPoint3& a_location, const RE::NiPoint3& a_rotation, RE::TESObjectCELL* a_cell)
{
	// apparently, the doorTeleportMarker (0x1) niAVObject does not have a model attached
	RE::TESObjectSTAT* emptyMarkerStatic = RE::TESForm::LookupByID(0x1)->As<RE::TESObjectSTAT>(); 
	auto worldspace = a_cell->GetRuntimeData().worldSpace;
	if (!worldspace) return nullptr;
	RE::ObjectRefHandle newDoorRef;
	newDoorRef = RE::TESDataHandler::GetSingleton()->CreateReferenceAtLocation(emptyMarkerStatic, a_location, a_rotation, a_cell, worldspace, nullptr, nullptr, RE::ObjectRefHandle(), false, true);

	if (newDoorRef && newDoorRef.get())
	{
		// remove when unloading cell
		newDoorRef.get()->SetTemporary();

		RE::NiPointer<RE::NiNode> markerModel_;
		RE::BSModelDB::DBTraits::ArgsType args;
		RE::BSResource::ErrorCode errorCode = RE::BSModelDB::Demand(a_modelPath, markerModel_, args);

		if (errorCode != RE::BSResource::ErrorCode::kNone) return nullptr;

		if (!markerModel_) return nullptr;

		newDoorRef.get()->Set3D(markerModel_->Clone(), true);

		return newDoorRef.get().get();
	}
	return nullptr;
}

std::string DebugHandler::GetCellInfo(const RE::TESObjectCELL* a_cell, RE::TESObjectREFR* a_ref)
{
	std::string infoStr{ "CELL INFO" };

	if (!a_cell || a_cell->GetFormID() >> 24 == 0xFF)
	{
		if (a_ref)
		{
			if (const auto* TES = RE::TES::GetSingleton(); TES)
			{
				a_cell = TES->GetCell(a_ref->GetPosition());
			}
		}
		if (a_cell && a_cell->GetFormID() >> 24 == 0xFF)
		{
			infoStr += "\nNot available";
		}

	}
	if (a_cell)
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
	return infoStr;
}

std::string DebugHandler::GetNavmeshInfo(const RE::TESObjectCELL* a_cell, RE::FormID a_formID)
{
	std::string infoStr = GetCellInfo(a_cell, nullptr);

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

	std::string infoStr = GetCellInfo(a_cell, a_ref);

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
	std::string infoStr = GetCellInfo(a_cell, nullptr);

	auto& cellLand = a_cell->GetRuntimeData().cellLand;
	
	infoStr += "\n\nLANDSCAPE INFO:"s;
	infoStr += fmt::format("\nForm ID {:08X}", cellLand->formID);
	infoStr += "\nReferenced by:"s;

	for (const auto& fileName : GetSouceFiles(cellLand))
	{
		infoStr += "\nMod: "s;
		infoStr += std::string(fileName);
	}
	

	infoStr += "\n\nQUAD INFO:"s;
	infoStr += fmt::format("\nQuad nummber: {}", a_quad + 1);
	
	int i = 0;
	for (const auto texture : cellLand->loadedData->quadTextures[a_quad])
	{
		i++;
		if (texture)
		{
			if (i > 1) infoStr += "\n"s;
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

std::string DebugHandler::GetLightMarkerInfo(const RE::TESObjectCELL* a_cell, RE::TESObjectREFR* a_ref)
{
	std::string infoStr = GetCellInfo(a_cell, a_ref);
	
	infoStr += "\n\nLIGHT INFO:";

	RE::TESObjectLIGH* light = a_ref->GetBaseObject()->As<RE::TESObjectLIGH>();
	
	auto type = "Omnidirectional"s;

	if (light)
	{
		if (light->data.flags.any(RE::TES_LIGHT_FLAGS::kHemiShadow)) // WhiterunDragonreachBasement
		{
			type = "Shadow Hemisphere"s;
		}
		else if (light->data.flags.any(RE::TES_LIGHT_FLAGS::kSpotShadow)) // PotemasCatacombs02
		{
			type = "Shadow Spotlight"s;
		}
		else if (light->data.flags.any(RE::TES_LIGHT_FLAGS::kOmniShadow))
		{
			type = "Shadow Omnidirectional"s;
		}

		auto FOV = light->data.fov;
		auto fade = light->fade;

		auto falloffExponent = light->data.fallofExponent;
		auto radius = light->data.radius;
		auto nearDistance = light->data.nearDistance;
		auto color = light->data.color;
		auto flickerEffect = "None"s;
		if (light->data.flags.any(RE::TES_LIGHT_FLAGS::kFlicker)) flickerEffect = "Flicker"s;
		else if (light->data.flags.any(RE::TES_LIGHT_FLAGS::kPulse)) flickerEffect = "Pulse"s;

		auto flickerPeriod = light->data.flickerPeriodRecip;
		auto flickerIntensityAmplitude = light->data.flickerIntensityAmplitude;
		auto flicekerMovementAmplitude = light->data.flickerMovementAmplitude;
		auto portalStrict = light->data.flags.any(RE::TES_LIGHT_FLAGS::kPortalStrict);



		infoStr += fmt::format("\nForm ID: {:08X}", a_ref->formID);
		infoStr += fmt::format("\nPosition: {:.0f}, {:.0f}, {:.0f}", a_ref->GetPositionX(), a_ref->GetPositionY(), a_ref->GetPositionZ());
		infoStr += fmt::format("\nType: {}", type);


		infoStr += fmt::format("\nFOV: {:.2f}", FOV);
		infoStr += fmt::format("\nFade: {:.2f}", fade);
		infoStr += fmt::format("\nFalloff Exponent: {:.2f}", falloffExponent);
		infoStr += fmt::format("\nRadius: {}", radius);
		infoStr += fmt::format("\nNear Clip: {:.2f}", nearDistance);
		infoStr += fmt::format("\nColor: {}, {}, {}", color.red, color.green, color.blue);
		infoStr += fmt::format("\nFlicker Effect: {}", flickerEffect);
		if (flickerEffect != "None"s)
		{
			infoStr += fmt::format("\n Period: {:.2f}", 1 / flickerPeriod);
			infoStr += fmt::format("\n Intensity Amplitude: {:.2f}", flickerIntensityAmplitude);
			infoStr += fmt::format("\n Movement Amplitude: {:.2f}", flicekerMovementAmplitude);
		}
		infoStr += fmt::format("\nPortal Strict: {}", portalStrict);
	}
	else
	{
		infoStr += "\nNo light info available"s;
	}

	infoStr += std::string("\nReferenced by:");
	for (const auto& fileName : GetSouceFiles(a_ref))
	{
		infoStr += "\nMod: ";
		infoStr += std::string(fileName);
	}

	return infoStr;
}

std::string DebugHandler::GetSoundMarkerInfo(const RE::TESObjectCELL* a_cell, RE::TESObjectREFR* a_ref)
{
	std::string infoStr = GetCellInfo(a_cell, a_ref);

	infoStr += "\n\nSOUND INFO:"s;
	auto sound = a_ref->GetBaseObject()->As<RE::TESSound>();

	if (sound && sound->descriptor && sound->descriptor->soundDescriptor)
	{
		auto soundDescriptor = sound->descriptor;
		// for more sound info:
		//auto soundDefinition = reinterpret_cast<RE::BGSStandardSoundDef*>(sound->descriptor->soundDescriptor);

		if (!soundEditorIDs.empty())
			infoStr += fmt::format("\nEditor ID: {}", soundEditorIDs[sound->GetFormID()]);
		else 
			infoStr += "Restart game with 'Mod Active = True' for editorID";

		infoStr += fmt::format("\nBase ID: {:08X}", sound->GetFormID());
		infoStr += fmt::format("\nForm ID: {:08X}", a_ref->GetFormID());

		infoStr += "\n\nSOUND DESCRIPTOR INFO:"s;

		if (!soundEditorIDs.empty())
			infoStr += fmt::format("\nEditor ID: {}", soundDescriptorEditorIDs[soundDescriptor->GetFormID()]);
		else
			infoStr += "Restart game with 'Mod Active = True' for editorID";

		infoStr += fmt::format("\nForm ID: {:08X}", soundDescriptor->GetFormID());

	}
	else
	{
		infoStr += "\nNo sound info available"s;
	}

	infoStr += std::string("\nReferenced by:");
	for (const auto& fileName : GetSouceFiles(a_ref))
	{
		infoStr += "\nMod: ";
		infoStr += std::string(fileName);
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

bool DebugHandler::ShouldMarkerBeDrawnWhenFar(RE::TESObjectREFR* a_ref)
{
	if (auto baseObject = a_ref->GetBaseObject())
	{
		bool drawWhenFar = false;
		ShouldMarkerBeDrawn(a_ref, drawWhenFar);
		return drawWhenFar;
	}
	return false;
	
}

bool DebugHandler::ShouldMarkerBeDrawn(RE::TESObjectREFR* a_ref)
{
	bool dummy = false;
	return ShouldMarkerBeDrawn(a_ref, dummy);
}

bool DebugHandler::ShouldMarkerBeDrawn(RE::TESObjectREFR* a_ref, bool& a_showWhenFar)
{
	//logger::info("ID: {:X}", a_baseObject->formID);
	auto baseObject = a_ref->GetBaseObject();
	switch (baseObject->formID)
	{
		case 0x02 : // TravelMarker
		case 0x03 : // NorthMarker
		case 0x05 : // DivineMarker
		case 0x06 : // TempleMarker
		case 0x10 :	// MapMarker
		case 0x12 : // HorseMarker
		case 0x15 : // MultiBoundMárker
		case 0x1F : // RoomMarker
		case 0x20 : // PortalMarker
		case 0x21 : // CollisionMarker
		case 0x33 : // Radiationmarker
		case 0x61 : // CellWaterCurrentMarker
		case 0x62 : // WaterCurrentMarker
		case 0xA0 : // AnimInteractionMarker
		case 0xC4 : // WaterCurrentZoneMarker
		case 0x1C035 : // ComplexSceneMARKER
		case 0xF077B : // RoadMarker
			return MCM::settings::GetShowOtherStaticMarkers();
		case 0x32 : // COCHeadingMarker
		case 0x34 : // HeadingMarker
			return MCM::settings::GetShowHeadingMarkers();
		case 0x3B : // XMarker
			return MCM::settings::GetShowXMarkers();
		case 0x1E595 :  // DragonPerchTower
		case 0x7A37D :	// MQ104FortMeetSoldier4Marker
		case 0xAA934 :  // DragonPerchRockL02
		case 0xBFB04 :  // SoldierWallIdle
		case 0x103442 : // CartFurnitureDriver
		case 0x105D4D : // CartFurnitureHorse
			return MCM::settings::GetShowSitMarkers(); // || MCM::settings::GetShowDragonMarkers() // complicated, sinces the dragons are furniture
		case 0x52FF5 : // WallLeanMarker
			return MCM::settings::GetShowLeanMarkers();
		case 0x138C0 : // DragonMarker
			return MCM::settings::GetShowDragonMarkers();
		case 0x3DF55 : // DragonMarkerCrashStrip
			a_showWhenFar = true;
			return MCM::settings::GetShowDragonMarkers();
		case 0x68E43: // DragonMoundBaseAlt
			return MCM::settings::GetShowOtherActivatorMarkers() || MCM::settings::GetShowDragonMarkers(); 
		case 0x22201 : // CritterLandingMarker_Small
		case 0x6B2FF : // critterSpawnPond_Shallow
		case 0x6B44D : // critterSpawnPond_Deep
		case 0x6B45B : // critterSpawnPond_Small
		case 0x6B45C : // critterSpawnDragonflies24x7
		case 0xC2D47 : // DoNotPlaceSmallCritterLandingMarkerHelper
		case 0xC51FF : // critterSpawnInsects_Few
		case 0xC5208 : // critterSpawnInsects_Single
		case 0xC5209 : // critterSpawnInsects_Many
			return MCM::settings::GetShowCritterMarkers();
		case 0x7776E : // FXMistLow01LongHalfVis
		case 0x7776F : // FXMistLow01Long
		case 0x77770 : // FXMistLow02Rnd
		case 0x77771 : // FXMistLow02RndHalfVis
		case 0x77772 : // FXMistLow01
		case 0x77773 : // FXMistLow01HalfVis
			return MCM::settings::GetShowMistMarkers();
		case 0x75DCB: // FXAmbBeamXbDustBig02
		case 0xAA8F5: // FXAmbBeamSlowFogBrt02
		case 0xAA8F6: // FXAmbBeamSlowFogBrt01
			return MCM::settings::GetShowLightBeamMarkers();
		case 0x16D4B : // FXSteamCrack
		case 0x1ACAC : // FXWaterFallSkirtSlope
		case 0x2BCA5 : // FXSplashSmallParticles
		case 0x2BCA7 : // FXSplashSmallParticlesLong
		case 0x2BCAB : // FXWaterfallThin512x128
		case 0x2BCC8 : // FXWaterfallThin2048x128
		case 0x2EB0D : // FXGlowFillRoundDim
		case 0x2EB0E : // FXGlowFillRoundMid
		case 0x2EB0F : // FXGlowFillRoundXBrt
		case 0x2EB10 : // FXGlowFillRoundXDim
		case 0x2EB11 : // FXGlowFillRoundBrt
		case 0x30B39 : // FXSteamBillow
		case 0x3103C : // FXSteamJetFast
		case 0x3103F : // FXSteamJet
		case 0x77775 : // FXSmokeWispsLg1x1
		case 0x77776 : // FXSmokeWispsLg2x1
		case 0x77777 : // FXSmokeWispsLgVol01
		case 0xB09E4 : // FXSmokeChimney01
		case 0xB09ED : // FXSmokeChimney02
		case 0xE4E22 : // FXDweSteam01
		case 0x108E37 : // FXAmbWaterSalmon018
			return MCM::settings::GetShowOtherMSTTMarkers();
		case 0x57A8C : // FireLgPlacedHazard
			return MCM::settings::GetShowHazardMarkers();
		case 0x80C32: // ImpactMarker01
			return MCM::settings::GetShowImpactMarkers();
		//case 0x1B37D : // FXRapids
		case 0x21513 : // NorLever01
		case 0x27EF9 : // GuardMarker
		case 0x812C6 : // FireWeaponMarker
		case 0x10B035 : // FCAmbWaterfallSalmon01
			return MCM::settings::GetShowOtherActivatorMarkers();
		case 0x2630D : // SkyrimCloudDistant01
		case 0x274B7 : // SkyrimCloudDistant01_25
		case 0x274B8 : // SkyrimCloudDistant01_50
		case 0x2754F : // SkyrimCloudDistant01_O
		case 0x274A6 : // SkyrimCloudDistant01_O_25
		case 0x274B5 : // SkyrimCloudDistant01_O_50
		case 0x2630E : // SkyrimCloudDistant02
		case 0x27499 : // SkyrimCloudDistant02_25
		case 0x27483 : // SkyrimCloudDistant02_50
		case 0x27481 : // SkyrimCloudDistant02_O
		case 0x2747E : // SkyrimCloudDistant02_O_25
		case 0x2747D : // SkyrimCloudDistant02_O_50
		case 0x17521 : // SkyrimCloudDistant03
		case 0x2747C : // SkyrimCloudDistant03_25
		case 0x2747A : // SkyrimCloudDistant03_50
		case 0x27472 : // SkyrimCloudDistant03_O
		case 0x2746E : // SkyrimCloudDistant03_O_25
		case 0x2745A : // SkyrimCloudDistant03_O_50
		case 0x17520 : // SkyrimCloudDistant04
		case 0x2744C : // SkyrimCloudDistant04_25
		case 0x27447 : // SkyrimCloudDistant04_50
		case 0x2743B : // SkyrimCloudDistant04_O
		case 0x2743A : // SkyrimCloudDistant04_O_25
		case 0x27437 : // SkyrimCloudDistant04_O_50
		case 0x1751F : // SkyrimCloudDistant05
		case 0x27432 : // SkyrimCloudDistant05_25
		case 0x27430 : // SkyrimCloudDistant05_50
		case 0x27426 : // SkyrimCloudDistant05_O
		case 0x27425 : // SkyrimCloudDistant05_O_25
		case 0x2740F : // SkyrimCloudDistant05_O_50
		case 0x1751E : // SkyrimCloudDistant06
		case 0x2740C : // SkyrimCloudDistant06_25
		case 0x27408 : // SkyrimCloudDistant06_50
		case 0x273FC : // SkyrimCloudDistant06_O
		case 0x273FA : // SkyrimCloudDistant06_O_25
		case 0x273F5 : // SkyrimCloudDistant06_O_50
		case 0x273F3 : // SkyrimCloudShape01
		case 0x273E9 : // SkyrimCloudShape01_25
		case 0x273E8 : // SkyrimCloudShape01_50
		case 0x273E6 : // SkyrimCloudShape01_O
		case 0x273E0 : // SkyrimCloudShape01_O_25
		case 0x273DC : // SkyrimCloudShape01_O_50
		case 0x273D9 : // SkyrimCloudShape02
		case 0x273D4 : // SkyrimCloudShape02_25
		case 0x273C1 : // SkyrimCloudShape02_50
		case 0x273BF : // SkyrimCloudShape02_O
		case 0x273B8 : // SkyrimCloudShape02_O_25
		case 0x273B7 : // SkyrimCloudShape02_O_50
		case 0x273B0 : // SkyrimCloudShape03
		case 0x273AC : // SkyrimCloudShape03_25
		case 0x273AB : // SkyrimCloudShape03_50
		case 0x2738B : // SkyrimCloudShape03_O
		case 0x27349 : // SkyrimCloudShape03_O_25
		case 0x27338 : // SkyrimCloudShape03_O_50
		case 0x27323 : // SkyrimCloudShape04
		case 0x2730F : // SkyrimCloudShape04_25
		case 0x272E5 : // SkyrimCloudShape04_50
		case 0x272DB : // SkyrimCloudShape04_O
		case 0x272D3 : // SkyrimCloudShape04_O_25
		case 0x272CC : // SkyrimCloudShape04_O_50
		case 0x272C4 : // SkyrimCloudShape05
		case 0x272B3 : // SkyrimCloudShape05_25
		case 0x272B2 : // SkyrimCloudShape05_50
		case 0x272B1 : // SkyrimCloudShape05_O
		case 0x272B0 : // SkyrimCloudShape05_O_25
		case 0x272AF : // SkyrimCloudShape05_O_50
		case 0x272AE : // SkyrimCloudShape06
		case 0x272AD : // SkyrimCloudShape06_25
		case 0x272A8 : // SkyrimCloudShape06_50
		case 0x272A6 : // SkyrimCloudShape06_O
		case 0x27299 : // SkyrimCloudShape06_O_25
		case 0x27289 : // SkyrimCloudShape06_O_50
		case 0x10C4D1: // INV_SkyrimCloudDistant01
		case 0x10C4D0: // INV_SkyrimCloudDistant01_25
		case 0x10C4CF: // INV_SkyrimCloudDistant01_50
		case 0x10C4CE: // INV_SkyrimCloudDistant01_O
		case 0x10C4CD: // INV_SkyrimCloudDistant01_O_25
		case 0x10C4CC: // INV_SkyrimCloudDistant01_O_50
		case 0x10C4CB: // INV_SkyrimCloudDistant02
		case 0x10C4CA: // INV_SkyrimCloudDistant02_25
		case 0x10C4C9: // INV_SkyrimCloudDistant02_50
		case 0x10C4C8: // INV_SkyrimCloudDistant02_O
		case 0x10C4C7: // INV_SkyrimCloudDistant02_O_25
		case 0x10C4C6: // INV_SkyrimCloudDistant02_O_50
		case 0x10C4C5: // INV_SkyrimCloudDistant03
		case 0x10C4C4: // INV_SkyrimCloudDistant03_25
		case 0x10C4C3: // INV_SkyrimCloudDistant03_50
		case 0x10C4C2: // INV_SkyrimCloudDistant03_O
		case 0x10C4C1: // INV_SkyrimCloudDistant03_O_25
		case 0x10C4C0: // INV_SkyrimCloudDistant03_O_50
		case 0x10C4BF: // INV_SkyrimCloudDistant04
		case 0x10C4BE: // INV_SkyrimCloudDistant04_25
		case 0x10C4BD: // INV_SkyrimCloudDistant04_50
		case 0x10C4BC: // INV_SkyrimCloudDistant04_O
		case 0x10C4BB: // INV_SkyrimCloudDistant04_O_25
		case 0x10C4BA: // INV_SkyrimCloudDistant04_O_50
		case 0x10C4B9: // INV_SkyrimCloudDistant05
		case 0x10C4B8: // INV_SkyrimCloudDistant05_25
		case 0x10C4B7: // INV_SkyrimCloudDistant05_50
		case 0x10C4B6: // INV_SkyrimCloudDistant05_O
		case 0x10C4B5: // INV_SkyrimCloudDistant05_O_25
		case 0x10C4B4: // INV_SkyrimCloudDistant05_O_50
		case 0x10C4B3: // INV_SkyrimCloudDistant06
		case 0x10C4B2: // INV_SkyrimCloudDistant06_25
		case 0x10C4B1: // INV_SkyrimCloudDistant06_50
		case 0x10C4B0: // INV_SkyrimCloudDistant06_O
		case 0x10C4AF: // INV_SkyrimCloudDistant06_O_25
		case 0x10C4AE: // INV_SkyrimCloudDistant06_O_50
		case 0x10C4AD: // INV_SkyrimCloudShape01
		case 0x10C4AC: // INV_SkyrimCloudShape01_25
		case 0x10C4AB: // INV_SkyrimCloudShape01_50
		case 0x10C4AA: // INV_SkyrimCloudShape01_O
		case 0x10C4A9: // INV_SkyrimCloudShape01_O_25
		case 0x10C4A8: // INV_SkyrimCloudShape01_O_50
		case 0x10C4A7: // INV_SkyrimCloudShape02
		case 0x10C4A6: // INV_SkyrimCloudShape02_25
		case 0x10C4A5: // INV_SkyrimCloudShape02_50
		case 0x10C4A4: // INV_SkyrimCloudShape02_O
		case 0x10C4A3: // INV_SkyrimCloudShape02_O_25
		case 0x10C4A2: // INV_SkyrimCloudShape02_O_50
		case 0x10C4A1: // INV_SkyrimCloudShape03
		case 0x10C4A0: // INV_SkyrimCloudShape03_25
		case 0x10C49F: // INV_SkyrimCloudShape03_50
		case 0x10C49E: // INV_SkyrimCloudShape03_O
		case 0x10C49D: // INV_SkyrimCloudShape03_O_25
		case 0x10C49C: // INV_SkyrimCloudShape03_O_50
		case 0x10C49B: // INV_SkyrimCloudShape04
		case 0x10C49A: // INV_SkyrimCloudShape04_25
		case 0x10C499: // INV_SkyrimCloudShape04_50
		case 0x10C498: // INV_SkyrimCloudShape04_O
		case 0x10C497: // INV_SkyrimCloudShape04_O_25
		case 0x10C496: // INV_SkyrimCloudShape04_O_50
		case 0x10C495: // INV_SkyrimCloudShape05
		case 0x10C494: // INV_SkyrimCloudShape05_25
		case 0x10C493: // INV_SkyrimCloudShape05_50
		case 0x10C492: // INV_SkyrimCloudShape05_O
		case 0x10C491: // INV_SkyrimCloudShape05_O_25
		case 0x10C490: // INV_SkyrimCloudShape05_O_50
		case 0x10C48F: // INV_SkyrimCloudShape06
		case 0x10C48E: // INV_SkyrimCloudShape06_25
		case 0x10C48D: // INV_SkyrimCloudShape06_50
		case 0x10C48C: // INV_SkyrimCloudShape06_O
		case 0x10C48B: // INV_SkyrimCloudShape06_O_25
		case 0x10C4D2: // INV_SkyrimCloudShape06_O_50
			a_showWhenFar = true;
			return MCM::settings::GetShowCloudMarkers();
		case 0x3040C : // CW1MeleeCloseAttacker
		case 0x3040E : // CW1MeleeWideAttacker
		case 0x3041E : // CW1MissileCloseAttacker
		case 0x3041F : // CW1MissileWideAttacker
		case 0x30404 : // CW1SpawnAttacker
		case 0x3040F : // CW2MeleeCloseAttacker
		case 0x30413 : // CW2MeleeWideAttacker
		case 0x30420 : // CW2MissileCloseAttacker
		case 0x30421 : // CW2MissileWideAttacker
		case 0x30405 : // CW2SpawnAttacker
		case 0x30414 : // CW3MeleeCloseAttacker
		case 0x30415 : // CW3MeleeWideAttacker
		case 0x30422 : // CW3MissileCloseAttacker
		case 0x30423 : // CW3MissileWideAttacker
		case 0x30406 : // CW3SpawnAttacker
		case 0x30416 : // CW4MeleeCloseAttacker
		case 0x30417 : // CW4MeleeWideAttacker
		case 0x30424 : // CW4MissileCloseAttacker
		case 0x30425 : // CW4MissileWideAttacker
		case 0x30407 : // CW4SpawnAttacker
		case 0x30418 : // CW5MeleeCloseAttacker
		case 0x30419 : // CW5MeleeWideAttacker
		case 0x30426 : // CW5MissileCloseAttacker
		case 0x30427 : // CW5MissileWideAttacker
		case 0x30408 : // CW5SpawnAttacker
			return MCM::settings::GetShowCWAttackerMarkers();
		case 0x30428 : // CW1MeleeCloseDefender
		case 0x30429 : // CW1MeleeWideDefender
		case 0x30432 : // CW1MissileCloseDefender
		case 0x30433 : // CW1MissileWideDefender
		case 0x30409 : // CW1SpawnDefender
		case 0x3042A : // CW2MeleeCloseDefender
		case 0x3042B : // CW2MeleeWideDefender
		case 0x30434 : // CW2MissileCloseDefender
		case 0x30435 : // CW2MissileWideDefender
		case 0x3041A : // CW2SpawnDefender
		case 0x30431 : // CW3MeleeCloseDefender
		case 0x3042C : // CW3MeleeWideDefender
		case 0x3043C : // CW3MissileCloseDefender
		case 0x30441 : // CW3MissileWideDefender
		case 0x3041B : // CW3SpawnDefender
		case 0x3042D : // CW4MeleeCloseDefender
		case 0x3042E : // CW4MeleeWideDefender
		case 0x3043D : // CW4MissileCloseDefender
		case 0x3043E : // CW4MissileWideDefender
		case 0x3041C : // CW4SpawnDefender		<------------------
		case 0x30430 : // CW5MeleeCloseDefender
		case 0x3042F : // CW5MeleeWideDefender
		case 0x3043F : // CW5MissileCloseDefender
		case 0x30440 : // CW5MissileWideDefender
		case 0x3041D : // CW5SpawnDefender
			return MCM::settings::GetShowCWDefenderMarkers();
		case 0x41B2D : // CWSiegeSonsSoldier
		case 0x41B2E : // CWSiegeImperialSoldier
		case 0x4B77C : // CWTower01
		case 0xDEEC6 : // CWTowerWall01
		case 0xE108E : // CWSoldierImperialUseCatapult (NPC, so doesnt work yet)
			return MCM::settings::GetShowOtherCWMarkers();
		case 0x69811 : // ShadowMarkFenceWood01
		case 0x69813 : // ShadowMarkLootWood01 // debug this
			return MCM::settings::GetShowTextureSetMarkers();
	}

	switch (baseObject->GetFormType())
	{
		case RE::FormType::IdleMarker:
			return MCM::settings::GetShowIdleMarkers();
		
	}



	return false;
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

	logger::debug("Cache details:");
	logger::debug("  Used Memory size (approximately):      {:>10.3f} MB ", size/1000000.f);
	logger::debug("  Allocated Memory size (approximately): {:>10.3f} MB", allocSize/1000000.f);
	logger::debug("  # of cells:      {:>10}", cellCount);
	logger::debug("  # of trianlges:  {:>10}", triangleCount);
	logger::debug("  # of vertices:   {:>10}", vertexCount);
	logger::debug("  # of extra info: {:>10}", extraEdgeInfoCount);

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


/*
RE::BSTArray<RE::BSTArray<RE::NiPointer<RE::NiNode>>> 

RE::ShadowSceneNode::GetRuntimeData().lightQueueAdd; (or AddLight(RE::NiLight* a_light))


at SkyrimSE.exe+338C870 we have a pointer to an array of arrays of NiNodes
*/
