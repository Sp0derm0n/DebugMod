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

		struct NavmeshInfo
		{
			RE::FormID formID;
			bool hasTriangleInfo = false;
			RE::BSTArray<RE::BSNavmeshVertex> vertices;
			RE::BSTArray<RE::BSNavmeshTriangle> triangles;
			RE::BSTArray<RE::BSNavmeshEdgeExtraInfo> extraEdgeInfo;
		};
		

		bool isDrawMenuOpen = false;
		bool isCoordinatesBoxVisible = false;
		bool hasDebugMenuBeenOpenedBefore = false;

		float timeSinceLastUpdate = 2.0f; // updates at least once per second

		std::map<RE::FormID, std::vector<std::string_view>> sourceFilesOrdered;
		std::map<RE::FormID, std::set<std::string_view>> sourceFiles;
		std::map<RE::FormID, std::vector<NavmeshInfo>> cachedNavmeshes;
		std::map<RE::FormID, bool> isCellsCacheFinalized;

		void Init();
		void Update(float a_delta);
		void DrawAll();
		void DrawCellBorders();
		void ShowCoordinates();
		void DrawTest();

		void OpenDrawMenu();
		void CloseDrawMenu();
		bool isAnyDebugON();

		void OnCellLoad(RE::TESObjectCELL* const& a_cell);
		void OnNavMeshLoad(RE::NavMesh* const& a_navmesh);
		void CacheNavmesh(RE::NavMesh* a_navmesh, RE::FormID a_cellID); // caches a navmesh beloning to the cell with id a_cellID
		void CacheCellNavmeshes(const RE::TESObjectCELL* a_cell); // caches navmeshes of a cell

		void SizeofCache();

	private:

		enum class EdgeLinkPosition
		{
			kAbove = 0,
			kCenter,
			kBelow
		};

		// navmersh triangleFlags
		uint16_t waterFlag  = 1 << 9;  // 0000 0010 0000 0000
		uint16_t doorFlag   = 1 << 10; // 0000 0100 0000 0000
		uint16_t inFileFlag = 1 << 11; // 0000 1000 0000 0000

		float GetLightLevel();

		void ForEachCellInRange(RE::NiPoint3 a_origin, float a_range, std::function<void(const RE::TESObjectCELL*)> a_callback);
		void DrawNavmesh(RE::NiPoint3 a_origin, float a_range);
		void DrawNavmeshEdgeLink(const RE::NiPoint3& a_point1, const RE::NiPoint3& a_point2, uint32_t a_color, uint32_t a_alpha, EdgeLinkPosition a_position);
		void DrawNavmeshCover(const RE::NiPoint3& a_rightPoint, const RE::NiPoint3& a_leftPoint, int32_t a_height, bool a_left, bool a_right);
		void DrawOcclusion(RE::NiPoint3 a_origin, float a_range);


		std::string GetNavmeshInfo(RE::FormID a_formID, const RE::TESObjectCELL* a_cell);
		std::string GetOcclusionInfo(const RE::TESObjectCELL* a_cell, RE::TESObjectREFR* a_ref, RE::NiPoint3 a_bounds);
		std::string GetQuadInfo(const RE::TESObjectCELL* a_cell, uint8_t a_quad);

		
		std::vector<std::string_view> GetSouceFiles(RE::TESForm* a_form);
		

		
};