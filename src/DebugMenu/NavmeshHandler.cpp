#include "NavmeshHandler.h"
#include "DebugMenu.h"


namespace DebugMenu
{
	NavmeshHandler::NavmeshHandler()
	{
		logger::debug("Initialized NavmeshHandler");
	}

	float NavmeshHandler::GetRange()
	{
		return MCM::settings::navmeshRange;
	}

	RE::BSEventNotifyControl NavmeshHandler::ProcessEvent(const RE::TESCellFullyLoadedEvent* a_event, RE::BSTEventSource<RE::TESCellFullyLoadedEvent>*)
	{
		if (MCM::settings::modActive && a_event)
		{
			OnCellFullyLoaded(a_event->cell);
		}
		return RE::BSEventNotifyControl::kContinue;
	}

	void NavmeshHandler::Draw()
	{
		if (!MCM::settings::showNavmesh) return;
		
		RE::NiPoint3 origin = GetCenter();
		float range = GetRange();

		Utils::ForEachCellInRange(origin, range, [&](const RE::TESObjectCELL* a_cell)
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
				auto metaData = CreateMetaData();
				metaData->formID = navmesh.formID;
				metaData->cell = a_cell;
				metaData->infoType = InfoType::kNavmesh;

				auto& vertices = navmesh.vertices;
				auto& triangles = navmesh.triangles;


				for (const auto vertex : vertices)
				{
					break;
					// dont draw vertices
					/*auto dx = origin.x - vertex.location.x;
					auto dy = origin.y - vertex.location.y;
					if (dx * dx + dy * dy < range * range)
						GetDrawHandler()->DrawPoint(vertex.location, 4, 0xFFFFFF, 90);*/
				}

				for (int i = 0; i < triangles.size(); i++)
				{
					const auto& triangle = triangles[i];
					uint16_t triangleFlag = triangle.triangleFlags.underlying();


					if (!MCM::settings::useRuntimeNavmesh && !(triangleFlag & inFileFlag))
					{
						if (i + 2 < triangles.size())
						{
							bool isNextTriangleInFile = triangles[i + 1].triangleFlags.underlying() & inFileFlag;
							bool isNextNextTriangleInFile = triangles[i + 2].triangleFlags.underlying() & inFileFlag;
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
						auto dx = origin.x - vertices[i].location.x;
						auto dy = origin.y - vertices[i].location.y;
						if (sqrtf(dx * dx + dy * dy) > range)
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
						GetDrawHandler()->DrawPolygon({ vertex0, vertex1, vertex2 }, borderThickness, triangleColor, MCM::settings::navmeshAlpha, MCM::settings::navmeshBorderAlpha, triangleColor, false, metaData);

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
										edgeLinkAlpha = static_cast<uint32_t>(100 - (100 - edgeLinkAlpha) * (100 - edgeLinkAlpha) / 100.0f); // cell border edgelinks usually overlap almost entirely, so their combined opacity is probably (1-(1-opacity)^2), ie. if they are at 20% opcaity, combined they are probably at 7% opacity
										edgeLinkPosition = EdgeLinkPosition::kAbove;
									}
									else if (navmesh.extraEdgeInfo.data()[edgeInfoIndex].type.any(RE::EDGE_EXTRA_INFO_TYPE::kLedgeDown))
									{
										edgeLinkColor = MCM::settings::navmeshLedgeEdgeLinkColor;
										edgeLinkAlpha = static_cast<uint32_t>(100 - (100 - edgeLinkAlpha) * (100 - edgeLinkAlpha) / 100.0f);
										edgeLinkPosition = EdgeLinkPosition::kBelow;
									}

									auto edgeLinkPolygon = GetEdgeLinkPolygon(vertices[triangle.vertices[edge]].location, vertices[triangle.vertices[edge == 2 ? 0 : edge + 1]].location, edgeLinkPosition);

									GetDrawHandler()->DrawPolygon(edgeLinkPolygon, 0, edgeLinkColor, edgeLinkAlpha, 0);
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
						// first four bits describe the height
						bool edge0HasCover = Utils::GetNavmeshCoverHeight(triangle.traversalFlags.underlying(), 0) != 0;
						bool edge1HasCover = Utils::GetNavmeshCoverHeight(triangle.traversalFlags.underlying(), 1) != 0;

						if (edge0HasCover) DrawCover(navmesh.formID, vertex0, vertex1, triangle.traversalFlags.underlying(), 0);
						if (edge1HasCover) DrawCover(navmesh.formID, vertex1, vertex2, triangle.traversalFlags.underlying(), 1);

					}
				}
			}
		});
	}

	std::vector<RE::NiPoint3> NavmeshHandler::GetEdgeLinkPolygon(const RE::NiPoint3& a_point1, const RE::NiPoint3& a_point2, EdgeLinkPosition a_position)
	{
		RE::NiPoint3 directionAlongLine = a_point2 - a_point1;
		RE::NiPoint3 center = (a_point1 + a_point2) / 2;

		float boxHeight = 24.0f;
		float heightAbove = 0.0f;
		float heightBelow = 0.0f;
		directionAlongLine /= 5;

		switch (a_position)
		{
			case EdgeLinkPosition::kAbove:
			{
				heightAbove = boxHeight * 5 / 6.0f;
				heightBelow = boxHeight * 1 / 6.0f;
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
				heightAbove = boxHeight * 1 / 6.0f;
				heightBelow = boxHeight * 5 / 6.0f;
				break;
			}
		}

		RE::NiPoint3 corner1 = center + directionAlongLine; corner1.z += heightAbove;
		RE::NiPoint3 corner2 = center + directionAlongLine; corner2.z -= heightBelow;
		RE::NiPoint3 corner3 = center - directionAlongLine; corner3.z -= heightBelow;
		RE::NiPoint3 corner4 = center - directionAlongLine; corner4.z += heightAbove;

		std::vector<RE::NiPoint3> polygon{ corner1, corner2, corner3, corner4 };
		
		return polygon;
	}

	void NavmeshHandler::DrawCover(RE::FormID a_formID, const RE::NiPoint3& a_rightPoint, const RE::NiPoint3& a_leftPoint, uint16_t a_traversalFlags, uint8_t a_edge)
	{
		bool left = false;
		bool right = false;

		// first four bits describe the height
		int32_t iHeight = Utils::GetNavmeshCoverHeight(a_traversalFlags, a_edge);


		if (MCM::settings::showCoverBeams)
		{
			left = Utils::GetNavmeshCoverLeft(a_traversalFlags, a_edge);
			right = Utils::GetNavmeshCoverRight(a_traversalFlags, a_edge);
		}
		// maybe horizontal line every 16 or 32 height?
		uint32_t color = MCM::settings::navmeshCoverColor;
		uint32_t alpha = MCM::settings::navmeshCoverAlpha;
		uint32_t borderColor = MCM::settings::navmeshCoverBorderColor;
		uint32_t borderAlpha = MCM::settings::navmeshCoverBorderAlpha;

		float height = static_cast<float>(iHeight);

		if (iHeight == 240)
		{
			color = MCM::settings::navmeshMaxCoverColor;
			alpha = MCM::settings::navmeshMaxCoverAlpha;
			borderColor = MCM::settings::navmeshMaxCoverBorderColor;
			borderAlpha = MCM::settings::navmeshMaxCoverBorderAlpha;
		}
		else if (iHeight < 64) // cover is ledge cover
		{
			height = -height;
		}

		RE::NiPoint3 directionAlongLine = a_rightPoint - a_leftPoint;
		float directionAlongLineLength = directionAlongLine.Length();
		RE::NiPoint3 unitDirectionAlongLine = directionAlongLine / directionAlongLineLength;

		float borderThickness = 3.0f;
		float coverBeamWidth = 20.0f;
		if (directionAlongLineLength < coverBeamWidth) coverBeamWidth = directionAlongLineLength * 0.4f;

		RE::NiPoint3 leftOffset = unitDirectionAlongLine * coverBeamWidth;
		RE::NiPoint3 rightOffset = -leftOffset;

		if (!left) leftOffset = RE::NiPoint3(0.0f, 0.0f, 0.0f);
		if (!right) rightOffset = RE::NiPoint3(0.0f, 0.0f, 0.0f);

		// Draw cover
		RE::NiPoint3 corner1 = a_leftPoint + leftOffset * 1.02;
		RE::NiPoint3 corner2 = a_leftPoint + leftOffset * 1.02;   corner2.z += height;
		RE::NiPoint3 corner3 = a_rightPoint + rightOffset * 1.02; corner3.z += height;
		RE::NiPoint3 corner4 = a_rightPoint + rightOffset * 1.02;

		std::vector<RE::NiPoint3> polygon{ corner1, corner2, corner3, corner4 };

		GetDrawHandler()->DrawPolygon(polygon, borderThickness, color, alpha, borderAlpha, borderColor);

		// Draw horizontal lines on cover to visualize height
		if (MCM::settings::showNavmeshCoverLines)
		{
			uint32_t stepSize = MCM::settings::linesHeight;
			for (int step = stepSize; step < iHeight; step += stepSize)
			{
				int8_t multiplier = iHeight < 64 ? -1 : 1;
				RE::NiPoint3 point1 = corner1; point1.z += multiplier * step;
				RE::NiPoint3 point2 = corner4; point2.z += multiplier * step;
				GetDrawHandler()->DrawLine(point1, point2, borderThickness, borderColor, borderAlpha);
			}
		}


		// Draw dot in the middle of the triangle edge to contain info:

		if (MCM::settings::showNavmeshCoverInfo)
		{
			RE::NiPoint3 middle = a_leftPoint + directionAlongLine / 2;

			auto metaData = CreateMetaData();
			metaData->navmeshTraversalFlags = a_traversalFlags;
			metaData->coverEdge = a_edge;
			metaData->infoType = InfoType::kNavmeshCover;
			GetDrawHandler()->DrawPoint(middle, 10, borderColor, borderAlpha, metaData);
		}


		// Draw end beams
		alpha *= 2;
		if (left)
		{
			corner1 = a_leftPoint;
			corner2 = a_leftPoint;				 corner2.z += height;
			corner3 = a_leftPoint + leftOffset;  corner3.z += height;
			corner4 = a_leftPoint + leftOffset;

			std::vector<RE::NiPoint3> beamPolygon{ corner1, corner2, corner3, corner4 };
			GetDrawHandler()->DrawPolygon(beamPolygon, 0.0f, 0x000000, alpha, 0);
		}

		if (right)
		{
			corner1 = a_rightPoint;
			corner2 = a_rightPoint;				  corner2.z += height;
			corner3 = a_rightPoint + rightOffset; corner3.z += height;
			corner4 = a_rightPoint + rightOffset;

			std::vector<RE::NiPoint3> beamPolygon{ corner1, corner2, corner3, corner4 };
			GetDrawHandler()->DrawPolygon(beamPolygon, 0.0f, 0x000000, alpha, 0);
		}
	}

	std::vector<std::string_view>& NavmeshHandler::GetNavmeshSourceFiles(RE::FormID a_navmeshFormID)
	{
		return sourceFilesOrdered[a_navmeshFormID];
	}


	void NavmeshHandler::OnCellFullyLoaded(RE::TESObjectCELL* a_cell)
	{
		if (!a_cell) return;

		CacheCellNavmeshes(a_cell);
	}

	// if a navmesh is not referenced by a new mod, sometimes a_cell->GetRuntimeData().navMeshes will not be defined,
	// but it can still be found in the nav mesh load
	// 
	// on the other hand, on navmesh load, the source array will only display one of the base game (+dlc) .esps (i think), 
	// but on cell load, new mods can also be found, though it wont always find eg. heathfires.esm when the navmesh is defined in
	// skyrim.esm, but these are somethimes caught on navmesh load

	// usually <1 µs per file, sometimes ~10 and very rarely 20.
	void NavmeshHandler::OnNavMeshLoad(RE::NavMesh* const& a_navmesh)
	{
		if (!a_navmesh) return;

		RE::FormID formID = a_navmesh->GetFormID();

		if (auto cell = a_navmesh->GetSaveParentCell(); cell)
		{
			CacheNavmesh(a_navmesh, cell->GetFormID());
		}

		if (!a_navmesh->sourceFiles.array) return;

		RE::TESFile** files = a_navmesh->sourceFiles.array->data();
		int numberOfFiles = a_navmesh->sourceFiles.array->size();

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

	void NavmeshHandler::OnCellLoad(RE::TESObjectCELL* const& a_cell)
	{
		// the sourceFiles array of the navmeshes only contain the first (when not bugged) and last file that edits it. the cell load is called by all mods editing the cell, so we just get the sourceFiles on all these loads
		if (!a_cell || !a_cell->GetRuntimeData().navMeshes)
		{
			return;
		}

		auto& navmeshes = a_cell->GetRuntimeData().navMeshes->navMeshes;
		for (auto& navmesh : navmeshes)
		{
			if (navmesh) OnNavMeshLoad(navmesh.get());
		}
	}

	// Sometimes, the navmesh only exists in memory when being loaded (seems to only happen to navmeshes not edited by new mods)
	// so they are cached.
	void NavmeshHandler::CacheNavmesh(RE::NavMesh* a_navmesh, RE::FormID a_cellID)
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

	void NavmeshHandler::CacheCellNavmeshes(const RE::TESObjectCELL* a_cell) // call on cell fully loaded
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

	void NavmeshHandler::SizeofCache()
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
			cellCount++;

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

		size += triangleCount * sizeof(RE::BSNavmeshTriangle);
		size += vertexCount * sizeof(RE::BSNavmeshVertex);
		size += extraEdgeInfoCount * sizeof(RE::BSNavmeshEdgeExtraInfo);

		allocSize += triangleCapacity * sizeof(RE::BSNavmeshTriangle);
		allocSize += vertexCapacity * sizeof(RE::BSNavmeshVertex);
		allocSize += extraEdgeInfoCapacity * sizeof(RE::BSNavmeshEdgeExtraInfo);

		logger::debug("Cache details:");
		logger::debug("  Used Memory size (approximately):      {:>10.3f} MB ", size / 1000000.f);
		logger::debug("  Allocated Memory size (approximately): {:>10.3f} MB", allocSize / 1000000.f);
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