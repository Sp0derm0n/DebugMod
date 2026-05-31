#pragma once

#include "DebugItem.h"

namespace DebugMenu
{
	class CellHandler : public DebugItem
	{
		public:
			CellHandler();
			void Draw() override;
			void OnCellLoad(const RE::TESObjectCELL* a_cell, RE::TESFile* a_mod);
			void OnLandLoad(const RE::TESObjectLAND* a_land, RE::TESFile* a_mod);
			void Test();

		private:
			struct VertexHeightMap
			{
				float offset;
				int8_t heightMap[33][33];
				uint8_t pad[3];
			};
			static_assert(sizeof(VertexHeightMap)==0x448);

			struct LandscapeHeightMap
			{
				std::vector<std::vector<vec3u>> vertices{};

				LandscapeHeightMap(VertexHeightMap& a_heightMap, RE::TESObjectCELL* a_cell);
				operator bool() const { return !vertices.empty(); }
			};

			struct ModLandscape
			{
				//std::vector<QuadLandscape> quads;
				std::string_view modName;
				bool hasLandData = false;
				bool hasCellLand = false;
				
			};

			struct CellLandscape
			{
				std::vector<ModLandscape> modLandscapes;
				std::vector<std::string_view> landLoadMods;
				RE::FormID cellID;
			};

			std::vector<CellLandscape> cellLandscapes;
	};
}