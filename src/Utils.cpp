#include "Utils.h"
#include "math.h"

namespace Utils
{
	void ForEachCellInRange(RE::NiPoint3 a_origin, float a_range, std::function<void(const RE::TESObjectCELL* a_cell)> a_callback)
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

	bool IsPlayerLoaded()
	{
		return RE::PlayerCharacter::GetSingleton()->Is3DLoaded();
	}

	bool IsRefInLoadedCell(const RE::TESObjectREFR* a_ref)
	{
		if (a_ref->IsPlayerRef()) return true;

		if (const auto cell = a_ref->GetSaveParentCell())
		{
			return cell->IsAttached() || a_ref->IsPersistent();
		}
		return false;
	}


	int32_t GetNavmeshCoverHeight(uint16_t a_navmeshTraversalFlags, uint8_t a_navmeshEdge)
	{
		if (a_navmeshEdge == 0) return (a_navmeshTraversalFlags & 0b1111) * 16;
		return ((a_navmeshTraversalFlags >> 6) & 0b1111) * 16;
	}

	bool GetNavmeshCoverLeft(uint16_t a_navmeshTraversalFlags, uint8_t a_navmeshEdge)
	{
		if (a_navmeshEdge == 0) return a_navmeshTraversalFlags & (1 << 4);
		return a_navmeshTraversalFlags & (1 << 10);
	}

	bool GetNavmeshCoverRight(uint16_t a_navmeshTraversalFlags, uint8_t a_navmeshEdge)
	{
		if (a_navmeshEdge == 0) return a_navmeshTraversalFlags & (1 << 5);
		return a_navmeshTraversalFlags & (1 << 11);
	}

	void AttachChildNode(RE::NiNode* a_parent, RE::NiAVObject* a_child)
	{
		if (RE::TaskQueueInterface::ShouldUseTaskQueue())
		{
			RE::TaskQueueInterface::GetSingleton()->QueueNodeAttach(a_child, a_parent);
		}
		else
		{
			a_parent->AttachChild(a_child, true);
		}
	}

	void DetachChildrenByName(RE::NiNode* a_node, const RE::BSFixedString a_childName)
	{
		for (int i = a_node->GetChildren().size() - 1; i > -1; i--) // loop in reverse order, so indexing isn't messed up when detaching children
		{
			auto& child = a_node->GetChildren()[i];
			if (!child.get()) continue;
			if (child->name == a_childName)
			{
				a_node->DetachChildAt(i);
			}
		}
	}

	bool TryAttachChildByName(RE::NiNode* a_node, const char* a_modelName, const RE::BSFixedString a_markerName, RE::NiAVObject*& a_objectOut)
	{
		RE::NiPointer<RE::NiNode> markerModel;
		RE::BSModelDB::DBTraits::ArgsType args;
		RE::BSResource::ErrorCode errorCode = RE::BSModelDB::Demand(a_modelName, markerModel, args);

		if (errorCode != RE::BSResource::ErrorCode::kNone) return false;

		if (!markerModel) return false;

		a_objectOut = markerModel->Clone()->AsNode();

		if (!a_objectOut) return false;

		a_objectOut->IncRefCount();
		a_objectOut->name = a_markerName;
		AttachChildNode(a_node, a_objectOut);
		a_objectOut->DecRefCount();

		return true;
	}

	bool IsNodeTreeVisible(RE::NiAVObject* a_obj, uint32_t a_depth) // returns true if all children are visible, else false
	{
		if (a_obj->GetAppCulled()) return false;

		if (auto node = a_obj->AsNode())
		{
			for (auto& child : node->GetChildren())
			{
				if (child && child.get())
				{
					if (!IsNodeTreeVisible(child.get(), a_depth++)) return false;
				}
			}
		}
		return true;
	}

	void CullNode(RE::NiAVObject* a_obj, bool a_cull)
	{
		CullNode(a_obj, a_cull, a_cull);
	}

	void CullNode(RE::NiAVObject* a_obj, bool a_cullNode, bool a_cullEditorMarker)
	{
		if (auto node = a_obj->AsNode())
		{
			if (auto editorMarker = node->GetObjectByName("EditorMarker"))
			{
				editorMarker->SetAppCulled(a_cullEditorMarker);
				// if setting marker visible, the parent node must be visible.
				// if setting marker invisible, if the editorMarker is the only child, the parent should be sat invisible
				if (!a_cullEditorMarker) 
				{
					node->SetAppCulled(false); 
				}
				else if(node->GetChildren().size() == 1)
				{
					node->SetAppCulled(true);
				}
			}
			else node->CullNode(a_cullNode);
		}
	}

	void PrintNodeTree(RE::NiAVObject* a_obj, std::string a_indent)
	{
		logger::debug("{}{}: {}", a_indent, a_obj->name, a_obj->GetAppCulled());

		if (auto node = a_obj->AsNode())
		{
			for (auto& child : node->GetChildren())
			{
				if (!child.get()) continue;
				PrintNodeTree(child.get(), a_indent + "|-"s);
			}
		}
	}

	bool IsMarkerVisible(RE::NiAVObject* a_obj)
	{
		if (a_obj->GetAppCulled()) return false;

		if (auto node = a_obj->AsNode())
		{
			if (auto editorMarker = node->GetObjectByName("EditorMarker"))
			{
				if (editorMarker->GetAppCulled()) return false;
			}
		}
		return true;
	}


	bool HasChildrenOfName(RE::NiNode* a_node, const RE::BSFixedString a_childName)
	{
		for (int i = a_node->GetChildren().size() - 1; i > -1; i--) // loop in reverse order, since marker nodes are added during runtime, so will usually be last in the children list
		{
			const auto& child = a_node->GetChildren()[i];
			if (!child.get()) continue;
			if (child->name == a_childName)
			{
				return true;
			}
		}
		return false;
	}

	uint32_t HowManyChildrenOfName(RE::NiNode* a_node, const RE::BSFixedString a_markerName)
	{
		uint32_t numberOfChildren = 0;
		for (const auto& child : a_node->GetChildren()) 
		{
			if (!child.get()) continue;
			if (child->name == a_markerName)
			{
				numberOfChildren++;
			}
		}
		return numberOfChildren;
	}


	float GetLandscapeHeightAtLocation(RE::NiPoint3 a_loc, RE::TESObjectCELL* a_cell)
	{
		auto cellHeights = a_cell->GetRuntimeData().cellLand->loadedData->heights;
		uintptr_t loadedData_addr = reinterpret_cast<uintptr_t>(a_cell->GetRuntimeData().cellLand->loadedData);
		float baseHeight = *reinterpret_cast<float*>(loadedData_addr + 0x49C0);

		float cellSize = 4096.0f;
		float halfCellSize = cellSize / 2;

		// Cell structure:
		//  
		//  3 | 4
		// ---|---
		// .1 | 2
		// ^ is cellCoords
		//

		if (const auto cellCoords = a_cell->GetCoordinates())
		{
			int quadIndex = 0;

			// x and y values of the nearest quad "wall"
			float quadXloc = cellCoords->worldX;
			float quadYloc = cellCoords->worldY;

			if (a_loc.x > cellCoords->worldX + halfCellSize)
			{
				quadIndex++;
				quadXloc = cellCoords->worldX + halfCellSize;
			}

			if (a_loc.y > cellCoords->worldY + halfCellSize)
			{
				quadIndex += 2;
				quadYloc = cellCoords->worldY + halfCellSize;
			}

			float deltaX = a_loc.x - quadXloc;
			float deltaY = a_loc.y - quadYloc;

			float deltaXRatio = deltaX / halfCellSize;
			float deltaYRatio = deltaY / halfCellSize;

			int gridSize = 17;

			int gridIndexX = static_cast<int>(deltaXRatio * gridSize);
			int gridIndexY = static_cast<int>(deltaYRatio * gridSize);

			if (gridIndexX == gridSize) gridIndexX--;
			if (gridIndexY == gridSize) gridIndexY--;

			float height = baseHeight + cellHeights[quadIndex][gridIndexX + gridSize * gridIndexY];
			return height;
		}
		return zFloor;

	}

	RE::NiNode* GetCellStaticNode(RE::TESObjectCELL* a_cell)
	{
		auto cell3D = a_cell->GetRuntimeData().loadedData->cell3D;
		auto node = cell3D && cell3D.get() && cell3D->GetChildren().size() > 3 ? cell3D->GetChildren()[3] : nullptr;
		if (node && node.get())
		{
			return node.get()->AsNode();
		}
		return nullptr;
	}

	std::string GethkpShapeTypeName(const RE::hkpShape* a_shape)
	{
		switch (a_shape->type)
		{
			case RE::hkpShapeType::kInvalid:
				return "kInvalid"s;
			case RE::hkpShapeType::kSphere:
				return "kSphere"s;
			case RE::hkpShapeType::kCylinder:
				return "kCylinder"s;
			case RE::hkpShapeType::kTriangle:
				return "kTriangle"s;
			case RE::hkpShapeType::kBox:
				return "kBox"s;
			case RE::hkpShapeType::kCapsule:
				return "kCapsule"s;
			case RE::hkpShapeType::kConvexVertices:
				return "kConvexVertices"s;
			case RE::hkpShapeType::kCollection:
				return "kCollection"s;
			case RE::hkpShapeType::kBVTree:
				return "kBVTree"s;
			case RE::hkpShapeType::kList:
				return "kList"s;
			case RE::hkpShapeType::kMOPP:
				return "kMOPP"s;
			case RE::hkpShapeType::kConvexTranslate:
				return "kConvexTranslate"s;
			case RE::hkpShapeType::kConvexTransform:
				return "kConvexTransform"s;
			case RE::hkpShapeType::kSampledHeightField:
				return "kSampledHeightField"s;
			case RE::hkpShapeType::kExtendedMesh:
				return "kExtendedMesh"s;
			case RE::hkpShapeType::kTransform:
				return "kTransform"s;
			case RE::hkpShapeType::kCompressedMesh:
				return "kCompressedMesh"s;
			case RE::hkpShapeType::kCompound:
				return "kCompound"s;
			case RE::hkpShapeType::kTotalSPU:
				return "kTotalSPU"s;
			case RE::hkpShapeType::kConvex:
				return "kConvex"s;
			case RE::hkpShapeType::kMOPPEmbedded:
				return "kMOPPEmbedded"s;
			case RE::hkpShapeType::kConvexPiece:
				return "kConvexPiece"s;
			case RE::hkpShapeType::kMultiSphere:
				return "kMultiSphere"s;
			case RE::hkpShapeType::kConvexList:
				return "kConvexList"s;
			case RE::hkpShapeType::kTriangleCollection:
				return "kTriangleCollection"s;
			case RE::hkpShapeType::kMultiRay:
				return "kMultiRay"s;
			case RE::hkpShapeType::kHeightField:
				return "kHeightField"s;
			case RE::hkpShapeType::kSphereRep:
				return "kSphereRep"s;
			case RE::hkpShapeType::kBV:
				return "kBV"s;
			case RE::hkpShapeType::kPlane:
				return "kPlane"s;
			case RE::hkpShapeType::kPhantomCallback:
				return "kPhantomCallback"s;
			case RE::hkpShapeType::kUser0:
				return "kUser0"s;
			case RE::hkpShapeType::kUser1:
				return "kUser1"s;
			case RE::hkpShapeType::kUser2:
				return "kUser2"s;
			case RE::hkpShapeType::kTotal:
				return "kTotal"s;
			case RE::hkpShapeType::kAll:
				return "kAll"s;
			default:
				return "default";
		}
	}

	RE::NiPoint3 RotateNiPoint3(const RE::NiPoint3& a_vector, const RE::hkQuaternion& a_quaternion)
	{
		if (a_quaternion.vec.IsEqual(identityQuat)) return a_vector;

		/*RE::hkVector4 vec{ a_vector.x, a_vector.y, a_vector.z, 0 };
		RE::hkVector4 inverseQuaternion = a_quaternion.vec * -1; 

		auto rotatedVector = a_quaternion.vec * vec * inverseQuaternion;*/

		auto quatVec = hkvec4toNiVec3(a_quaternion.vec);
		float w = a_quaternion.vec.quad.m128_f32[3];

		// Reduced quaternion rotation formula https://gamedev.stackexchange.com/questions/28395/rotating-vector3-by-a-quaternion
		return quatVec * 2*(quatVec.Dot(a_vector)) + a_vector * (w*w - quatVec.Dot(quatVec)) + quatVec.Cross(a_vector) * 2 * w;
	}


	RE::NiPoint3 hkvec4toNiVec3(const RE::hkVector4& a_vector)
	{
		return RE::NiPoint3{ a_vector.quad.m128_f32[0], a_vector.quad.m128_f32[1], a_vector.quad.m128_f32[2] };
	}
	
	RE::NiMatrix3 hkMat3toNiMat3(const RE::hkMatrix3& a_matrix)
	{
		return RE::NiMatrix3(hkvec4toNiVec3(a_matrix.col0), hkvec4toNiVec3(a_matrix.col1), hkvec4toNiVec3(a_matrix.col2)).Transpose();
	}

	void printVec4(const char* a_msg, const RE::hkVector4& a_vector)
	{
		logger::debug("{} {} {} {} {}",a_msg, a_vector.quad.m128_f32[0], a_vector.quad.m128_f32[1], a_vector.quad.m128_f32[2], a_vector.quad.m128_f32[3]);
	}

	RE::NiMatrix3 GetRotationMatrixFromAxis(const RE::NiPoint3& a_axis, float a_angle)
	{
		float cos = cosf(a_angle);
		float sin = sinf(a_angle);
		float x = a_axis.x;
		float y = a_axis.y;
		float z = a_axis.z;

		RE::NiPoint3 row0{ x*x * (1-cos) + cos,   x*y * (1-cos) - z*sin, x*z * (1-cos) + y*sin};
		RE::NiPoint3 row1{ x*y * (1-cos) + z*sin, y*y * (1-cos) + cos,   y*z * (1-cos) - x*sin};
		RE::NiPoint3 row2{ x*z * (1-cos) - y*sin, y*z * (1-cos) + x*sin, z*z * (1-cos) + cos};
		
		return RE::NiMatrix3(row0, row1, row2);
	}

	RE::NiMatrix3 GetRotationMatrixZ(float a_angle)
	{
		float cos = cosf(a_angle);
		float sin = sinf(a_angle);
		RE::NiPoint3 row0{ cos, -sin, 0 };
		RE::NiPoint3 row1{ sin,  cos, 0 };
		RE::NiPoint3 row2{   0,    0, 1 };
		return RE::NiMatrix3(row0, row1, row2);
	}

	vec3u Utils::NiToGLMVec3(RE::NiPoint3& a_point)
	{
		return vec3u{ a_point.x, a_point.y, a_point.z };
	}

	vec3u Utils::NiToGLMVec3(RE::NiPoint3&& a_point)
	{
		return vec3u{ a_point.x, a_point.y, a_point.z };
	}

	ConnectivityData Utils::FindConvexHull(RE::hkArray<RE::hkVector4> a_vertices, RE::hkArray<RE::hkVector4>& a_planeEquations)
	{
		RE::hkArray<uint16_t> vertexIndices = GetEmptyHkArray<uint16_t>();
		RE::hkArray<uint8_t> verticesPerFace = GetEmptyHkArray<uint8_t>();

		auto numberOfPoints = a_vertices.size();

		for (int i = 0; i < numberOfPoints - 2; i++)
		{
			auto& pt1 = a_vertices[i];
			for (int j = i + 1; j < numberOfPoints - 1; j++)
			{
				auto& pt2 = a_vertices[j];
				auto vec21 = pt2 - pt1;
				for (int k = j + 1; k < numberOfPoints; k++)
				{
					auto& pt3 = a_vertices[k];
					auto vec31 = pt3 - pt1;
					auto normal = vec21.Cross(vec31);

					int8_t prev = 0;
					bool keep = true;

					for (int l = 0; l < numberOfPoints; l++)
					{
						if (l == i || l == j || l == k) continue;

						auto& otherPoint = a_vertices[l];
						auto direction = normal.Dot3(otherPoint - pt1);

						if (direction == 0) continue;

						int8_t eps = direction > 0 ? 1 : -1;

						// True whenever a set of points have been found where one is above and one is below the triangle surface
						if (eps + prev == 0)
						{
							keep = false;
							break;
						}

						prev = eps;
					}
					if (keep)
					{
						vertexIndices.push_back(i);
						vertexIndices.push_back(j);
						vertexIndices.push_back(k);
						verticesPerFace.push_back(3);
					}
				}
			}
		}

		//for (const auto& planeEquation : a_planeEquations)
		//{
		//	uint8_t numVertices = 0;
		//	for (int i = 0; i < a_vertices.size(); i++)
		//	{
		//		if (numVertices == 255) break;
		//		// plane equation ax + by + cz - d = 0
		//		// lhs is planeEquation.Dot3(vertex)
		//		float result = std::abs(planeEquation.Dot3(a_vertices[i]) + planeEquation.quad.m128_f32[3]);
		//		float eps = 1e-3f;
		//		if (result < eps)
		//		{
		//			vertexIndices.push_back(i);
		//			numVertices++;
		//		}
		//	}
		//	verticesPerFace.push_back(numVertices);
		//}
		return ConnectivityData{ vertexIndices, verticesPerFace };
	}


	std::vector<TriangleIndices> Utils::PlaneConvexHullIndicesToTriangleIndices(const std::vector<uint16_t>& a_hull)
	{
		// Assume clockwise convex hull
		// for n indices, order the them like this:
		//    123, 134, 145, 156, ... , 1(n-1)n

		std::vector<TriangleIndices> triangleIndices;
		uint16_t firstIndex = a_hull[0];
		for (int i = 1; i < a_hull.size() - 1; i++)
		{
			triangleIndices.push_back(TriangleIndices{ firstIndex, a_hull[i], a_hull[i+1]});
		}

		return triangleIndices;
	}

	std::vector<TriangleIndices> Utils::ConvexHullPlanesIndicesToTriangleIndices(const std::vector<std::vector<uint16_t>>& a_hull)
	{
		std::vector<TriangleIndices> triangleIndices;
		for (const auto& planeHull : a_hull)
		{
			uint16_t firstIndex = planeHull[0];
			for (int i = 1; i < planeHull.size() - 1; i++)
			{
				triangleIndices.push_back(TriangleIndices{ firstIndex, planeHull[i], planeHull[i + 1] });
			}
		}
		return triangleIndices;
	}


	std::string GetFormEditorID(const RE::TESForm* a_form)
	{
		static auto tweaks = GetModuleHandle(L"po3_Tweaks");
		static auto func = reinterpret_cast<_GetFormEditorID>(GetProcAddress(tweaks, "GetFormEditorID"));
		if (func) {
			return func(a_form->formID);
		}
		return "";
	}
}