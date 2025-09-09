#pragma once

#include <unordered_set>
#include "DrawMenu.h"
#include "DrawHandler.h"
#include "UIHandler.h"
#include "DebugUIMenu.h"

//#define TRACEOBJECTS

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
			RE::FormID formID = 0x0;
			bool hasTriangleInfo = false;
			RE::BSTArray<RE::BSNavmeshVertex> vertices;
			RE::BSTArray<RE::BSNavmeshTriangle> triangles;
			RE::BSTArray<RE::BSNavmeshEdgeExtraInfo> extraEdgeInfo;
		};
		
		bool isDrawMenuOpen = false;
		bool isCoordinatesBoxVisible = false;
		bool hasDebugMenuBeenOpenedBefore = false;

		float timeSinceLastUpdate = 2.0f; // updates at least once per second, so 2 seconds means it updates next frame

		std::map<RE::FormID, std::vector<std::string_view>> sourceFilesOrdered;
		std::map<RE::FormID, std::set<std::string_view>> sourceFiles;
		std::map<RE::FormID, std::vector<NavmeshInfo>> cachedNavmeshes;
		std::map<RE::FormID, bool> isCellsCacheFinalized;
		std::map<RE::FormID, std::string> soundEditorIDs;
		std::map<RE::FormID, std::string> soundDescriptorEditorIDs;

		void Init();
		void Update(float a_delta);
		void ResetUpdateTimer();
		void DrawAll();
		void DrawCellBorders();
		void ShowCoordinates();
		void DrawMarkers(RE::NiPoint3 a_origin, float a_range);
		void HideAllMarkers();
		void DrawTest();

		void OpenDrawMenu();
		void CloseDrawMenu();
		bool isAnyDebugON();

		void OnCellLoad(RE::TESObjectCELL* const& a_cell);
		void OnCellFullyLoaded(RE::TESObjectCELL* a_cell);
		void OnNavMeshLoad(RE::NavMesh* const& a_navmesh);
		void CacheNavmesh(RE::NavMesh* a_navmesh, RE::FormID a_cellID); // caches a navmesh beloning to the cell with id a_cellID
		void CacheCellNavmeshes(const RE::TESObjectCELL* a_cell); // caches navmeshes of a cell

		void SizeofCache();

	private:

		const RE::FormID debugFormID = 0x10FF45; // reference ID
		uint32_t debugMarkerIndex = -1; // sat by the code when the marker is added
		std::vector<RE::NiPoint3> debugPoints; 

		enum class EdgeLinkPosition
		{
			kAbove = 0,
			kCenter,
			kBelow
		};

		enum Visibility : bool
		{
			kHide = false,
			kShow = true
		};


		struct Marker
		{
			Marker(RE::TESObjectREFR* a_ref, RE::BSFixedString a_markerName = "") :
				ref(a_ref->GetHandle().get()),
				markerName(a_markerName),
				formID(a_ref->formID)
			{}

			RE::TESObjectREFRPtr ref;
			RE::BSFixedString markerName = ""s;
			RE::FormID formID = 0;
			bool drawWhenFar = false;
			bool cullWhenHiding = true;

			void SetDefaultState();
		};

		struct MarkerInfo
		{
			std::string path = ""s;
			RE::BSFixedString name = "";
		};

		

		// navmesh triangleFlags
		uint16_t waterFlag  = 1 << 9;  // 0000 0010 0000 0000
		uint16_t doorFlag   = 1 << 10; // 0000 0100 0000 0000
		uint16_t inFileFlag = 1 << 11; // 0000 1000 0000 0000

		const float lightBulbAlphaMax = 2.25;
		const float drawWhenFarRange = 30000.0f;

		RE::BSEffectShaderMaterial* lightBulbMaterial = nullptr;

		const RE::BSFixedString furnitureMarkerName = "FurnitureMarkerVis";
		const RE::BSFixedString doorTeleportMarkerName = "TeleportMarkerVis";
		const RE::BSFixedString lightMarkerName = "LightMarkerVis";
		const RE::BSFixedString locRefTypeName = "LocRefTypeMarkerVis";
		const RE::BSFixedString skyMarkerBeamName = "SkyMarkerVis";

		std::vector<std::unique_ptr<Marker>> visibleMarkers;
		std::unordered_set<RE::FormID> visibleMarkersIDs;

		void AddVisibleMarker(RE::TESObjectREFR* a_ref, RE::BSFixedString a_markerName = "", bool a_cullWhenHiding = true);
		void RemoveVisibleMarker(uint32_t a_markerIndex);

		void CopyEffectShaderMaterial(const std::string& a_filepath, const std::string& a_shapeName, RE::BSEffectShaderMaterial*& a_materialOut);
		void ForEachCellInRange(RE::NiPoint3 a_origin, float a_range, std::function<void(const RE::TESObjectCELL*)> a_callback);
		void DrawNavmesh(RE::NiPoint3 a_origin, float a_range);
		void DrawNavmeshEdgeLink(const RE::NiPoint3& a_point1, const RE::NiPoint3& a_point2, uint32_t a_color, uint32_t a_alpha, EdgeLinkPosition a_position);
		void DrawNavmeshCover(const RE::NiPoint3& a_rightPoint, const RE::NiPoint3& a_leftPoint, int32_t a_height, bool a_left, bool a_right);
		void DrawOcclusion(RE::NiPoint3 a_origin, float a_range);

		void ShowMarker(RE::TESObjectREFR* a_ref);
		void ShowDoorTeleportMarker(RE::TESObjectREFR* a_ref);
		void ShowFurnitureMarker(RE::TESObjectREFR* a_ref);
		void ShowStaticMarker(RE::TESObjectREFR* a_ref);
		void ShowLightMarker(RE::TESObjectREFR* a_ref);
		void ShowSoundMarker(RE::TESObjectREFR* a_ref);
		void ShowLocRefType(RE::TESObjectREFR* a_ref);
		void ShowSkyMarkerBeam(RE::TESObjectREFR* a_ref);
		[[nodiscard]] bool ShowNodeIfNeeded(RE::TESObjectREFR* a_ref);

		void HideMarker(uint32_t a_markerIndex);
		


		bool IsMarkerLoaded(const std::unique_ptr<Marker>& a_marker);
		bool ShouldMarkerBeDrawnWhenFar(RE::TESObjectREFR* a_ref);
		bool ShouldMarkerBeDrawn(RE::TESObjectREFR* a_ref);
		bool ShouldMarkerBeDrawn(RE::TESObjectREFR* a_ref, bool& a_showWhenFar);

		float GetLightLevel();
		
		RE::NiNode* GetNodeFromRef(RE::TESObjectREFR* a_ref);
		RE::NiNode* TrySet3DByName(RE::TESObjectREFR* a_ref, const char* a_modelName, const RE::BSFixedString a_markerName);
		
		MarkerInfo GetMarkerInfo(RE::TESObjectREFR* a_ref);

		RE::TESObjectCELL* GetCellFromCoordinates(RE::NiPoint3 a_location);
		RE::TESObjectREFR* InstantiateModel(const char* a_modelPath, const RE::NiPoint3& a_location, const RE::NiPoint3& a_rotation, RE::TESObjectCELL* a_cell);

		std::string GetCellInfo(const RE::TESObjectCELL* a_cell, RE::TESObjectREFR* a_ref);
		std::string GetNavmeshInfo(const RE::TESObjectCELL* a_cell, RE::FormID a_formID);
		std::string GetOcclusionInfo(const RE::TESObjectCELL* a_cell, RE::TESObjectREFR* a_ref, RE::NiPoint3 a_bounds, bool a_isDisabled);
		std::string GetQuadInfo(const RE::TESObjectCELL* a_cell, uint8_t a_quad);
		std::string GetLightMarkerInfo(const RE::TESObjectCELL* a_cell, RE::TESObjectREFR* a_ref);
		std::string GetSoundMarkerInfo(const RE::TESObjectCELL* a_cell, RE::TESObjectREFR* a_ref);

		
		std::vector<std::string_view> GetSouceFiles(RE::TESForm* a_form);
		

		
};