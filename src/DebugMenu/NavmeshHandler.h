#pragma once

#include "DebugItem.h"

namespace DebugMenu
{
	class NavmeshHandler : 
		public DebugItem,
		public RE::BSTEventSink<RE::TESCellFullyLoadedEvent>
	{
		public:
			NavmeshHandler();

			void							Draw() override;
			void							OnCellFullyLoaded(RE::TESObjectCELL* a_cell);
			void							OnNavMeshLoad(RE::NavMesh* const& a_navmesh);
			void							OnCellLoad(RE::TESObjectCELL* const& a_cell);
			std::vector<std::string_view>&	GetNavmeshSourceFiles(RE::FormID a_navmeshFormID); // not used

			RE::BSEventNotifyControl ProcessEvent(const RE::TESCellFullyLoadedEvent* a_event, RE::BSTEventSource<RE::TESCellFullyLoadedEvent>*);

		private:
			struct NavmeshInfo
			{
				RE::FormID formID = 0x0;
				bool hasTriangleInfo = false;
				RE::BSTArray<RE::BSNavmeshVertex> vertices;
				RE::BSTArray<RE::BSNavmeshTriangle> triangles;
				RE::BSTArray<RE::BSNavmeshEdgeExtraInfo> extraEdgeInfo;
			};

			enum TriangelFlag : uint32_t
			{
				waterFlag	= 1 << 9,
				doorFlag	= 1 << 10,
				inFileFlag	= 1 << 11,
			};

			enum class EdgeLinkPosition
			{
				kAbove = 0,
				kCenter,
				kBelow
			};

			std::map<RE::FormID, std::vector<std::string_view>> sourceFilesOrdered;
			std::map<RE::FormID, std::set<std::string_view>>	sourceFiles;
			std::map<RE::FormID, std::vector<NavmeshInfo>>		cachedNavmeshes;
			std::map<RE::FormID, bool>							isCellsCacheFinalized;
			
			float						GetRange() override;
			std::vector<RE::NiPoint3>	GetEdgeLinkPolygon(const RE::NiPoint3& a_point1, const RE::NiPoint3& a_point2, EdgeLinkPosition a_position);
			void						DrawCover(RE::FormID a_formID, const RE::NiPoint3& a_rightPoint, const RE::NiPoint3& a_leftPoint, uint16_t a_traversalFlags, uint8_t a_edge);
			void						CacheNavmesh(RE::NavMesh* a_navmesh, RE::FormID a_cellID); // caches a navmesh beloning to the cell with id a_cellID
			void						CacheCellNavmeshes(const RE::TESObjectCELL* a_cell); // caches navmeshes of a cell
			void						SizeofCache();

			

	};

}