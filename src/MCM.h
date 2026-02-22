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
		
			static void UpdateCollisionColor();
	};

	class DebugMenuPresets
	{
		public:
			static void Init();
			static void LoadMarkerSettings(uint32_t a_presetIndex);
			static void SaveMarkerSettings(uint32_t a_presetIndex);

		private:
			static void ReadWriteSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_varName, bool a_write, bool& a_variable);
			static void SaveLoadSettings(uint32_t a_presetIndex, bool a_save);
	};

	void Register();
	void InitNonMCMSettings();
	
	// not in use yet
	struct MarkerSetting
	{
		MarkerSetting(const std::string a_uiName, std::string a_identifier, std::string a_iniName, MarkerSetting* a_parent = nullptr) : 
			uiDisplayName(a_uiName),
			identifier(a_identifier),
			iniName(a_iniName),
			parent(a_parent),
			settingValue(false)
		{}

		std::string uiDisplayName;
		std::string identifier;
		std::string iniName;
		MarkerSetting* parent;
		bool settingValue;
		
	};

	struct settings
	{
		// Main
		static inline uint32_t openMenuHotkey;
		static inline uint32_t updateRate;
		static inline uint32_t dayNightIndex;
		static inline uint32_t collisionDisplayIndex;
		static inline uint32_t scrollUpHotkey;
		static inline uint32_t scrollDownHotkey;
		static inline uint32_t maxCollisions;

		static inline bool modActive;
		static inline bool showInfoOnHover;
		static inline bool showCrosshair;
		static inline bool showCanvasBorder;
		static inline bool updateCollisionsEveryFrame;
		static inline bool showCellBorders;
		static inline bool showCellWalls;
		static inline bool showCellQuads;
		static inline bool showNavmesh;
		static inline bool useRuntimeNavmesh;
		static inline bool showNavmeshTriangles;
		static inline bool showNavmeshCover;
		static inline bool showOcclusion;
		static inline bool showCoordinates;
		static inline bool showCoverBeams;
		static inline bool showMarkers;
		static inline bool showCollision;
		static inline bool collisionOcclude;
		static inline bool cleanCollisions;
		static inline bool showCharController;
		static inline bool useD3DMCMSetting; // Setting read from MCM
		static inline bool useD3D; // setting which actually enableds funcitonality; This is sat in main after the first read of MCM settings, and will not be changed elsewhere; a restart is required to change it


		static inline float infoRange;
		static inline float canvasScale;
		static inline float cellWallsHeight;
		static inline float rangeStep;
		static inline float navmeshRange;
		static inline float occlusionRange;
		static inline float markersRange;
		static inline float collisionRange;

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
		static inline uint32_t disabledOcclusionColor;
		static inline uint32_t occlusionBorderColor;
		static inline uint32_t lightBulbInfoColor;
		static inline uint32_t soundMarkerInfoColor;
		static inline uint32_t collisionColorInt;

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
		static inline uint32_t markerInfoAlpha;
		static inline uint32_t collisionAlpha;

		static inline float lightBulbAplha;

		// Advanced
		static inline bool showMoreNavmeshSourcefiles;
		static inline bool showNavmeshCoverInfo;
		static inline bool showNavmeshCoverLines;
		static inline bool showMarkerInfo;
		
		static inline uint32_t linesHeight;
		static inline uint32_t capsuleCylinderSegments;
		static inline uint32_t capsuleSphereSegments;

		// Non MCM settings
		static inline float minRange;
		static inline float maxRange;

		static inline bool updateVisibleMarkers;

		static inline vec4u collisionColor;

		//static MarkerSetting showFurnitureMarkers;
		//static MarkerSetting showSitMarkers;

		// to add new marker (streamline this): 
		//		make setting, 
		//		make function to get value of setting, 
		//		make it save to the inis
		//		add it in DebugUIMenu.cpp

		static inline bool showFurnitureMarkers;
		static inline bool showSitMarkers;
		static inline bool showLeanMarkers;
		static inline bool showSleepMarkers;

		static inline bool showLightMarkers;
		static inline bool showOmniMarkers;
		static inline bool showShadowOmniMarkers;
		static inline bool showShadowSpotMarkers;
		static inline bool showShadowHemiMarkers;

		static inline bool showMiscMarkers;
		static inline bool showIdleMarkers;
		static inline bool showSoundMarkers;
		static inline bool showDragonMarkers;
		static inline bool showCloudMarkers;
		static inline bool showCritterMarkers;
		static inline bool showFloraMarkers;
		static inline bool showHazardMarkers;
		static inline bool showTextureSetMarkers;

		static inline bool showMovableStaticMarkers;
		static inline bool showMistMarkers;
		static inline bool showLightBeamMarkers;
		static inline bool showOtherMSTTMarkers;

		static inline bool showStaticMarkers;
		static inline bool showXMarkers;
		static inline bool showHeadingMarkers;
		static inline bool showDoorTeleportMarkers;
		static inline bool showOtherStaticMarkers;

		static inline bool showActivatorMarkers;
		static inline bool showImpactMarkers;
		static inline bool showOtherActivatorMarkers;

		static inline bool showCWMarkers;
		static inline bool showCWAttackerMarkers;
		static inline bool showCWDefenderMarkers;
		static inline bool showOtherCWMarkers;


		static inline bool GetShowFurnitureMarkers()		{ return showFurnitureMarkers; }
		static inline bool GetShowSitMarkers()				{ return showFurnitureMarkers && showSitMarkers; }
		static inline bool GetShowLeanMarkers()				{ return showFurnitureMarkers && showLeanMarkers; }
		static inline bool GetShowSleepMarkers()			{ return showFurnitureMarkers && showSleepMarkers; }

		static inline bool GetShowLightMarkers()			{ return showLightMarkers; }
		static inline bool GetShowOmniMarkers()				{ return showLightMarkers && showOmniMarkers;}
		static inline bool GetShowShadowOmniMarkers()		{ return showLightMarkers && showShadowOmniMarkers;}
		static inline bool GetShowShadowSpotMarkers()		{ return showLightMarkers && showShadowSpotMarkers;}
		static inline bool GetShowShadowHemiMarkers()		{ return showLightMarkers && showShadowHemiMarkers;}


		static inline bool GetShowMiscMarkers()				{ return showMiscMarkers; }
		static inline bool GetShowIdleMarkers()				{ return showMiscMarkers && showIdleMarkers; }
		static inline bool GetShowSoundMarkers()			{ return showMiscMarkers && showSoundMarkers; }
		static inline bool GetShowDragonMarkers()			{ return showMiscMarkers && showDragonMarkers; }
		static inline bool GetShowCloudMarkers()			{ return showMiscMarkers && showCloudMarkers; }
		static inline bool GetShowCritterMarkers()			{ return showMiscMarkers && showCritterMarkers; }
		static inline bool GetShowFloraMarkers()			{ return showMiscMarkers && showFloraMarkers; }
		static inline bool GetShowHazardMarkers()			{ return showMiscMarkers && showHazardMarkers; }
		static inline bool GetShowTextureSetMarkers()		{ return showMiscMarkers && showTextureSetMarkers; }

		static inline bool GetShowMovableStaticMarkers()	{ return showMovableStaticMarkers; }
		static inline bool GetShowMistMarkers()				{ return showMovableStaticMarkers && showMistMarkers; }
		static inline bool GetShowLightBeamMarkers()		{ return showMovableStaticMarkers && showLightBeamMarkers; }
		static inline bool GetShowOtherMSTTMarkers()		{ return showMovableStaticMarkers && showOtherMSTTMarkers; }

		static inline bool GetShowStaticMarkers()			{ return showStaticMarkers; }
		static inline bool GetShowXMarkers()				{ return showStaticMarkers && showXMarkers; }
		static inline bool GetShowHeadingMarkers()			{ return showStaticMarkers && showHeadingMarkers; }
		static inline bool GetShowDoorTeleportMarkers()		{ return showStaticMarkers && showDoorTeleportMarkers; }
		static inline bool GetShowOtherStaticMarkers()		{ return showStaticMarkers && showOtherStaticMarkers; }

		static inline bool GetShowActivatorMarkers()		{ return showActivatorMarkers; }
		static inline bool GetShowImpactMarkers()			{ return showActivatorMarkers && showImpactMarkers; }
		static inline bool GetShowOtherActivatorMarkers()	{ return showActivatorMarkers && showOtherActivatorMarkers; }

		static inline bool GetShowCWMarkers()				{ return showCWMarkers; }
		static inline bool GetShowCWAttackerMarkers()		{ return showCWMarkers && showCWAttackerMarkers; }
		static inline bool GetShowCWDefenderMarkers()		{ return showCWMarkers && showCWDefenderMarkers; }
		static inline bool GetShowOtherCWMarkers()			{ return showCWMarkers && showOtherCWMarkers; }

		static inline bool logUI;
		static inline float uiScale;
	};

	
}
