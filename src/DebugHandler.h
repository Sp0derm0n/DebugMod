#pragma once

#include "DrawMenu.h"
#include "DrawHandler.h"
#include "UIHandler.h"
#include "DebugUIMenu.h"

class DebugHandler
{
	public:

		DrawHandler* g_DrawHandler = nullptr;
		UIHandler* g_UI = nullptr;

		static DebugHandler* GetSingleton()
		{
			static DebugHandler singleton;
			return std::addressof(singleton);
		}

		struct TriangleInfo
		{
			bool cellBorderEdgeLink[3];
		};

		struct NavmeshInfo
		{
			RE::FormID formID;
			bool hasTriangleInfo = false;
			RE::BSTArray<RE::BSNavmeshVertex> vertices;
			RE::BSTArray<RE::BSNavmeshTriangle> triangles;
			RE::BSTArray<RE::BSNavmeshEdgeExtraInfo> extraEdgeInfo;
			std::vector<TriangleInfo> trianglesInfo; // triangle index, edge index

		};
		

		bool isDrawMenuOpen = false;
		bool hasDebugMenuBeenOpenedBefore = false;
		bool useRuntimeNavmesh = false;

		float minRange = 1000.0f;
		float maxRange = 15000.0f;
		float timeSinceLastUpdate = 2; // updates at least once per second

		std::map<RE::FormID, std::vector<std::string_view>> sourceFilesOrdered;
		std::map<RE::FormID, std::set<std::string_view>> sourceFiles;
		std::map<RE::FormID, std::vector<NavmeshInfo>> cachedNavmeshes;
		std::map<RE::FormID, bool> isCellsCacheFinalized;

		void Init();
		void Update(float a_delta);
		void DrawAll();
		void DrawCellBorders();
		void DrawNavmesh(float a_range);
		void DrawOcclusion(float a_range);
		void DrawTest();

		void OpenDrawMenu();
		void CloseDrawMenu();
		bool isAnyDebugON();

		void OnCellLoad(RE::TESObjectCELL* a_cell);
		void OnNavMeshLoad(RE::NavMesh* a_navmesh);
		void CacheNavmesh(RE::NavMesh* a_navmesh, RE::FormID a_cellID); // caches a navmesh beloning to the cell with id a_cellID
		void CacheCellNavmeshes(RE::TESObjectCELL* a_cell); // caches navmeshes of a cell

		void SizeofCache();

	private:

		// navmersh triangleFlags
		uint16_t waterFlag  = 1 << 9;  // 0000 0010 0000 0000
		uint16_t doorFlag   = 1 << 10; // 0000 0100 0000 0000
		uint16_t inFileFlag = 1 << 11; // 0000 1000 0000 0000

		struct PlaneMarker
		{
			RE::TESObjectREFR* reference;
			RE::NiPoint3 bounds;
		};

		float GetLightLevel();

		void GetTrianglesInfo(NavmeshInfo& a_navmeshInfo, RE::FormID a_cellID);
		void DrawNavmesh(RE::TESObjectCELL* a_cell, RE::NiPoint3 a_origin, float a_range);
		void DrawNavmeshEdgeLink(const RE::NiPoint3& a_point1, const RE::NiPoint3& a_point2, uint32_t a_color);

		std::string GetNavmeshInfo(RE::FormID a_formID, RE::TESObjectCELL* a_cell);
		std::string GetOcclusionInfo(PlaneMarker* a_planeMarker);
		std::string GetQuadInfo(RE::TESObjectCELL* a_cell, uint8_t a_quad);

		
		std::vector<std::string_view> GetSouceFiles(RE::TESForm* a_form);
		

		
};