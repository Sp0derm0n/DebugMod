#include "Utils.h"

namespace Utils
{
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
}