#pragma once

namespace Utils
{
	const float zFloor = -30000.0f;
	const uint32_t defaultCullFlag = 1 << 31;

	void AttachChildNode(RE::NiNode* a_parent, RE::NiAVObject* a_child);
	void DetachChildrenByName(RE::NiNode* a_node, const RE::BSFixedString a_childName);

	bool TryAttachChildByName(RE::NiNode* a_node, const char* a_modelName, const RE::BSFixedString a_markerName, RE::NiAVObject*& a_objectOut);
	bool IsNodeTreeVisible(RE::NiAVObject* a_obj, uint32_t a_depth = 0);
	bool IsMarkerVisible(RE::NiAVObject* a_obj);
	bool HasChildrenOfName(RE::NiNode* a_node, const RE::BSFixedString a_childName);
	uint32_t HowManyChildrenOfName(RE::NiNode* a_node, const RE::BSFixedString a_markerName);

	void CullNode(RE::NiAVObject* a_obj, bool a_cullNode, bool a_cullEditorMarker);
	void CullNode(RE::NiAVObject* a_obj, bool a_cull);
	void PrintNodeTree(RE::NiAVObject* a_obj, std::string a_indent = ""s);

	float GetLandscapeHeightAtLocation(RE::NiPoint3 a_loc, RE::TESObjectCELL* a_cell);

	RE::NiNode* GetCellStaticNode(RE::TESObjectCELL* a_cell);
}