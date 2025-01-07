#pragma once

#include <SimpleIni.h>

namespace MCM
{
	class DebugMenuMCM 
	{
		public:
			static bool Register(RE::BSScript::IVirtualMachine* a_vm);
			static void OnConfigClose(RE::TESQuest*);

			static void ReadSettingsFromPath(std::filesystem::path a_path, bool a_firstRead);
			static void ReadSettings(bool a_firstRead = false);

			static void ReadBoolSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_varName, bool& a_var);
			static void ReadFloatSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_varName, float& a_var);
			static void ReadUInt32Setting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_varName, uint32_t& a_var);
		
	};

	void Register();
	void InitNonMCMSettings();
	
	struct settings
	{
		// Main
		static inline uint32_t openMenuHotkey;
		static inline uint32_t updateRate;
		static inline uint32_t dayNightIndex;
		static inline uint32_t scrollUpHotkey;
		static inline uint32_t scrollDownHotkey;

		static inline bool modActive;
		static inline bool showInfoOnHover;
		static inline bool showCrosshair;
		static inline bool showCanvasBorder;
		static inline bool showCellBorders;
		static inline bool showCellWalls;
		static inline bool showCellQuads;
		static inline bool showNavmesh;
		static inline bool showNavmeshTriangles;
		static inline bool showNavmeshCover;
		static inline bool showOcclusion;
		static inline bool showCoordinates;
		static inline bool showCoverBeams;

		static inline float infoRange;
		static inline float canvasScale;
		static inline float cellWallsHeight;
		static inline float rangeStep;
		static inline float navmeshRange;
		static inline float occlusionRange;

		// Colors
		static inline uint32_t cellBorderColor;
		static inline uint32_t cellWallsColor;
		static inline uint32_t cellQuadsColor;
		static inline uint32_t navmeshColor;
		static inline uint32_t navmeshDoorColor;
		static inline uint32_t navmeshWaterColor;
		static inline uint32_t navmeshPrefferedColor;
		static inline uint32_t navmeshCellEdgeLinkColor;
		static inline uint32_t navmeshLedgeEdgeLinkColor;
		static inline uint32_t navmeshCoverColor;
		static inline uint32_t navmeshCoverBorderColor;
		static inline uint32_t navmeshMaxCoverColor;
		static inline uint32_t navmeshMaxCoverBorderColor;
		static inline uint32_t occlusionColor;
		static inline uint32_t occlusionBorderColor;

		// Alpha
		static inline uint32_t cellBorderAlpha;
		static inline uint32_t cellWallsAlpha;
		static inline uint32_t cellQuadsAlpha;
		static inline uint32_t navmeshAlpha;
		static inline uint32_t navmeshBorderAlpha;
		static inline uint32_t navmeshEdgeLinkAlpha;
		static inline uint32_t navmeshCoverAlpha;
		static inline uint32_t navmeshCoverBorderAlpha;
		static inline uint32_t navmeshMaxCoverAlpha;
		static inline uint32_t navmeshMaxCoverBorderAlpha;
		static inline uint32_t occlusionAlpha;
		static inline uint32_t occlusionBorderAlpha;

		// Advanced
		static inline bool showMoreNavmeshSourcefiles;
		static inline bool showNavmeshCoverInfo;
		static inline bool showNavmeshCoverLines;
		
		static inline uint32_t linesHeight;

		// Non MCM settings
		static inline bool useRuntimeNavmesh;
		static inline float minRange;
		static inline float maxRange;
	};
}
