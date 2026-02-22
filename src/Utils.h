#pragma once

namespace Utils
{

	struct TriangleIndices
	{
		uint16_t index1;
		uint16_t index2;
		uint16_t index3;
	};

	const float zFloor = -30000.0f;
	const uint32_t defaultCullFlag = 1 << 31;
	const RE::hkVector4 identityQuat{ 0.0f, 0.0f, 0.0f, 1.0f };

	void			ForEachCellInRange(RE::NiPoint3 a_origin, float a_range, std::function<void(const RE::TESObjectCELL*)> a_callback);
	bool			IsPlayerLoaded();
	bool			IsRefInLoadedCell(const RE::TESObjectREFR* a_ref);

	int32_t			GetNavmeshCoverHeight(uint16_t a_navmeshTraversalFlags, uint8_t a_navmeshEdge);
	bool			GetNavmeshCoverLeft(uint16_t a_navmeshTraversalFlags, uint8_t a_navmeshEdge);
	bool			GetNavmeshCoverRight(uint16_t a_navmeshTraversalFlags, uint8_t a_navmeshEdge);

	void			AttachChildNode(RE::NiNode* a_parent, RE::NiAVObject* a_child);
	void			DetachChildrenByName(RE::NiNode* a_node, const RE::BSFixedString a_childName);

	bool			TryAttachChildByName(RE::NiNode* a_node, const char* a_modelName, const RE::BSFixedString a_markerName, RE::NiAVObject*& a_objectOut);
	bool			IsNodeTreeVisible(RE::NiAVObject* a_obj, uint32_t a_depth = 0);
	bool			IsMarkerVisible(RE::NiAVObject* a_obj);
	bool			HasChildrenOfName(RE::NiNode* a_node, const RE::BSFixedString a_childName);
	uint32_t		HowManyChildrenOfName(RE::NiNode* a_node, const RE::BSFixedString a_markerName);

	void			CullNode(RE::NiAVObject* a_obj, bool a_cullNode, bool a_cullEditorMarker);
	void			CullNode(RE::NiAVObject* a_obj, bool a_cull);
	void			PrintNodeTree(RE::NiAVObject* a_obj, std::string a_indent = ""s);

	float			GetLandscapeHeightAtLocation(RE::NiPoint3 a_loc, RE::TESObjectCELL* a_cell);

	RE::NiNode*		GetCellStaticNode(RE::TESObjectCELL* a_cell);
	

	std::string		GethkpShapeTypeName(const RE::hkpShape* a_shape);
	RE::NiPoint3	RotateNiPoint3(const RE::NiPoint3& a_vector, const RE::hkQuaternion& a_quaternion);
	RE::NiMatrix3	GetRotationMatrixFromAxis(const RE::NiPoint3& a_axis, float a_angle);
	RE::NiMatrix3	GetRotationMatrixZ(float a_angle);
	RE::NiPoint3	hkvec4toNiVec3(const RE::hkVector4& a_vector);
	RE::NiMatrix3	hkMat3toNiMat3(const RE::hkMatrix3& a_matrix);
	void			printVec4(const char* a_msg, const RE::hkVector4& a_vector);

	vec3u			NiToGLMVec3(RE::NiPoint3& a_point);
	vec3u			NiToGLMVec3(RE::NiPoint3&& a_point);

	struct ConnectivityData
	{
		RE::hkArray<uint16_t> vertexIndices;
		RE::hkArray<uint8_t> verticesPerFace;
	};
	ConnectivityData FindConvexHull(RE::hkArray<RE::hkVector4> a_vertices, RE::hkArray<RE::hkVector4>& a_planeEquations);

	// Turn n points in a plane into list of triangles
	std::vector<TriangleIndices> PlaneConvexHullIndicesToTriangleIndices(const std::vector<uint16_t>& a_hull);

	// Turn a full convex hull consisting of m planes into a single list of triangles
	std::vector<TriangleIndices> ConvexHullPlanesIndicesToTriangleIndices(const std::vector<std::vector<uint16_t>>& a_hull);
	

	template <typename T>
	inline RE::hkArray<T> GetEmptyHkArray()
	{
		RE::hkArray<T> newArray;
		newArray._data = nullptr;
		newArray._size = 0;
		newArray._capacityAndFlags = 1 << 31;
		return newArray;
	}

	template <typename T>
    inline void HashCombine(size_t& seed, const T& v) noexcept 
	{
        std::hash<T> h;
        seed ^= h(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }	
	
	using _GetFormEditorID = const char* (*)(std::uint32_t);
	std::string	GetFormEditorID(const RE::TESForm* a_form);
}