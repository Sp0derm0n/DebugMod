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
			float baseHeight = *reinterpret_cast<float*>(loadedData_addr + 0x49C0); // also given as sum(heightExtents)/2

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

					DrawHandler::ShapeMetaData quadMetaData;
					quadMetaData.cell = cell;
					quadMetaData.infoType = InfoType::kQuad;
					quadMetaData.quad = i;
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

	CellHandler::LandscapeHeightMap::LandscapeHeightMap(VertexHeightMap& a_heightMap, RE::TESObjectCELL* a_cell)
	{
		// Generate heights /////////////////////////////////////////////////////////////////////////////
		float heights[33][33] = {};
		heights[0][0] = a_heightMap.offset * 8;

		// each value in the heightmap constitutes a height difference when multiplied by 8, compared to the previous point in the row
		// the first value in each row, however, is a difference compared to the first value in the previous row
		// except for the first row ofcourse, which simply starts out at offset * 8

		for (int i = 0; i < 33; i++)
		{
			for (int j = 0; j < 33; j++)
			{
				if (i == 0 && j == 0) continue;

				float heightDifference = a_heightMap.heightMap[i][j] * 8.0f;

				if (j == 0)
					heights[i][j] = heights[i - 1][0] + heightDifference;
				else
					heights[i][j] = heights[i][j - 1] + heightDifference;
				logger::info("h: {} {}", heights[i][j], a_heightMap.heightMap[i][j]);
			}
		}

		// Generate vertices

		float cellSize = 4096.0f;
		float cellInternalGridLength = cellSize / 32; // 33 heights = 32 differences
		if (const auto cellCoords = a_cell->GetCoordinates())
		{
			for (int i = 0; i < 33; i++)
			{
				std::vector<vec3u> row;
				for (int j = 0; j < 33; j++)
				{
					float x = cellCoords->worldX + cellInternalGridLength * j;
					float y = cellCoords->worldY + cellInternalGridLength * i;
					float z = heights[i][j];

					logger::info("xyz: {:.0f} {:.0f} {:.0f}", x, y, z);

					row.push_back(vec3u{ x, y, z });
				}
				vertices.emplace_back(std::move(row));
			}
		}

	}


	void CellHandler::Test()
	{
		return;
		//RE::PlayerCamera::GetSingleton()->ToggleFreeCameraMode(false);

		RE::ControlMap::GetSingleton()->ToggleControls(RE::UserEvents::USER_EVENT_FLAG::kMovement, true, false);


		return;
		logger::info("Test");
		/*for (const auto& cellLandscape : cellLandscapes)
		{
			logger::info("CELL: {:X}", cellLandscape.cellID);
			for (const auto& modLandscape : cellLandscape.modLandscapes)
			{
				logger::info(" mod: {}; cellLand: {}; landData? {}", modLandscape.modName, modLandscape.hasCellLand, modLandscape.hasLandData);
			}
			for (const auto& mod : cellLandscape.landLoadMods)
			{
				logger::info(" Loaded mod: {}", mod);
			}
		}*/
		auto pos = RE::PlayerCharacter::GetSingleton()->GetPosition();
		auto cell = RE::TES::GetSingleton()->GetCell(pos);
		RE::TESFile* file = RE::TESDataHandler::GetSingleton()->GetLoadedMods()[0];

		logger::info("cell? {}; file? {}", cell ? true : false, file ? true : false);

		if (file && cell)
		{
			file->OpenTES(RE::NiFile::OpenMode::kReadOnly, false);

			logger::info("file: {}", file->GetFilename());
			logger::info("offset: {}", file->fileOffset);

			auto res = file->SeekCell(cell->GetRuntimeData().worldSpace, cell->GetCoordinates()->cellX, cell->GetCoordinates()->cellY);
			logger::info("seekcell: {}", res);
			logger::info("offset: {}", file->fileOffset);

			res = file->SeekLandscapeForCurrentCell();
			logger::info("seeklandscapeForCurrentCell: {}", res);
			logger::info("offset: {}", file->fileOffset);
			logger::info("FORM: {:X}", file->currentform.formID);
			logger::info(" size: {}; actualChunkSize: {}", file->currentform.length, file->actualChunkSize);
			auto* heightMap = new VertexHeightMap();

			if (true/*file->currentform.flags & (1 << 0)*/) // has vertex heightmap / normal map flag
			{
				logger::info("flags {}", file->currentform.flags);
				do
				{
					if (file->GetCurrentSubRecordType() == 'TGHV')
					{
						logger::info("chunk size: {:X}", file->actualChunkSize);
						auto read = file->ReadData(heightMap, sizeof(VertexHeightMap));
						logger::info("read? {}; data: {:X} offset: {}", read, reinterpret_cast<uintptr_t>(heightMap), heightMap->offset);
					}
				} while (file->SeekNextSubrecord());

				uint32_t fileSize = file->currentform.length;

				auto loadedData = cell->GetRuntimeData().cellLand->loadedData;

				auto heights = cell->GetRuntimeData().cellLand->loadedData->heights;
				/*std::string heightsString = "heights = [[";
				for (int i = 0; i < 34; i++)
				{
					int index0 = i;
					if (i >= 17)
					{
						index0 -= 17;
					}
					if (i == 17) continue;

					for (int j = 0; j < 34; j++)
					{
						int quad = 0;
						int index1 = j;
						if (j >= 17)
						{
							index1 -= 17;
						}
						if (i >= 17) quad += 2;
						if (j >= 17) quad += 1;

						if (j == 17) continue;


						heightsString += fmt::format("{}", heights[quad][17 * index0 + index1]);
						if (j != 33) heightsString += ",";
					}
					heightsString += "]";
					if (i != 33) heightsString += ", [";
				}
				heightsString += "]";
				logger::info("{}", heightsString);*/

				auto heightsString = "heights = [["s;
				for (int i = 0; i < 33; i++)
				{
					for (int j = 0; j < 33; j++)
					{
						heightsString += fmt::format("{}", heightMap->heightMap[i][j]);
						if (j != 32) heightsString += ",";
					}
					heightsString += "]";
					if (i != 32) heightsString += ", [";
				}
				heightsString += "]";
				logger::info("{}", heightsString);
			}


			/*logger::info("heightExtents: {} {}", loadedData->heightExtents.x, loadedData->heightExtents.y);
			float baseHeight = *reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(loadedData) + 0x49C0);
			logger::info("baseHeight: {}", baseHeight);*/

			/*if (!loadedData->mesh[0]->GetAppCulled())
			{
				for (int i = 0; i < 4; i++) loadedData->mesh[i]->SetAppCulled(true);
			}
			else;
			{
				for (int i = 0; i < 4; i++) loadedData->mesh[i]->SetAppCulled(false);
			}*/


			LandscapeHeightMap landscape{ *heightMap, cell };

			std::vector<CollisionHandler::CollisionTriangle> triangles{};

			if (landscape)
			{
				auto& pos = landscape.vertices[0][0];
				logger::info("Created landscape; first vertex: {} {} {}", pos[0], pos[1], pos[2]);

				// 32x32 squares
				for (int i = 0; i < 32; i++)
				{
					for (int j = 0; j < 32; j++)
					{
						auto v1 = landscape.vertices[i][j];
						auto v2 = landscape.vertices[i + 1][j];
						auto v3 = landscape.vertices[i + 1][j + 1];
						auto v4 = landscape.vertices[i][j + 1];
						auto square = CollisionHandler::RefCollisionData::SquareToTriangles(v1, v2, v3, v4);
						triangles.push_back(square.first);
						triangles.push_back(square.second);
					}
				}
			}

			if (triangles.size() != 0)
			{
				CollisionHandler::CollisionMesh landscapeMesh{ triangles };
				Renderer::DrawMesh(landscapeMesh.meshDrawer);

			}
			else logger::info("no triangles");



			file->CloseTES(false);


		}
		else logger::info("no file");

	}

	void CellHandler::OnCellLoad(const RE::TESObjectCELL* a_cell, RE::TESFile* a_mod)
	{
		//	std::string_view modName = a_mod ? a_mod->GetFilename() : "no mod"s;

		//	if (!a_cell) 
		//	{
		//		logger::info("NO CELL");
		//		return;
		//	}


		//	ModLandscape landscape;
		//	landscape.modName = modName;

		//	RE::FormID cellID = a_cell->formID;


		//	const auto* cellLand = a_cell->GetRuntimeData().cellLand;
		//	landscape.hasCellLand = cellLand ? true : false;
		//	const auto* loadedLandData = cellLand ? cellLand->loadedData : nullptr;
		//	if (loadedLandData)
		//	{
		//		landscape.hasLandData = true;
		//		for (int i = 0; i < 4; i++)
		//		{
		//			QuadLandscape quad;
		//			for (int j = 0; j < 289; j++)
		//			{
		//				quad.heights.push_back(loadedLandData->heights[i][j]);
		//			}
		//			landscape.quads.push_back(quad);
		//		}
		//		logger::info("CELL LOAD: {:X} LAND? {} DATA? {} | {}", cellID, landscape.hasCellLand, landscape.hasLandData, modName);

		//	}
		//	

		//	bool isCellCached = false;
		//	for (auto& cellLandscape : cellLandscapes)
		//	{
		//		if (cellLandscape.cellID == cellID)
		//		{
		//			cellLandscape.modLandscapes.emplace_back(std::move(landscape));
		//			isCellCached = true;
		//			break;
		//		}
		//	}

		//	if (!isCellCached)
		//	{
		//		CellLandscape cellLandscape;
		//		cellLandscape.cellID = cellID;
		//		cellLandscape.modLandscapes.emplace_back(std::move(landscape));
		//		cellLandscapes.emplace_back(std::move(cellLandscape));
		//	}
	}

	void CellHandler::OnLandLoad(const RE::TESObjectLAND* a_land, RE::TESFile* a_mod)
	{
		//	std::string_view mod = a_mod ? a_mod->GetFilename() : "no mod"s;
		//	const auto cell = a_land->parentCell;


		//	RE::FormID cellID = cell ? cell->formID : 0x0;

		//	logger::info("cell: {:X} mod: {} file offset: {} formID: {}", cellID, mod, a_mod->fileOffset, a_mod->currentform.formID);


		//	if (!a_land) 
		//	{
		//		//logger::info("{} {:X} NO LAND", mod, cellID);
		//		return;
		//	}


		//	if (!a_land->loadedData)
		//	{
		//		//logger::info("{} {:X} NO LOADED DATA", mod, cellID);
		//		return;
		//	}

		//	//logger::info("MOD: {}; loadedData? {}", mod, a_land->loadedData ? true : false);


		//	if (!cell) 
		//	{
		//		logger::info("{} {:X} NO CELL", mod, cellID);
		//		return;
		//	}


		//	//logger::info("LOADED LAND IN CELL {:X} from {}", cellID, mod);

		//	bool isCellCached = false;
		//	for (auto& cellLandscape : cellLandscapes)
		//	{
		//		if (cellLandscape.cellID == cellID)
		//		{
		//			cellLandscape.landLoadMods.push_back(mod);
		//			isCellCached = true;
		//			break;
		//		}
		//	}

		//	if (!isCellCached)
		//	{
		//		CellLandscape cellLandscape;
		//		cellLandscape.cellID = cellID;
		//		cellLandscape.landLoadMods.push_back(mod);
		//		cellLandscapes.emplace_back(std::move(cellLandscape));
		//	}
	}
}

