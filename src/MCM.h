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

	struct settings
	{
		struct LightMode
		{
			const static uint32_t day = 0;
			const static uint32_t night = 1;
			const static uint32_t autoMode = 2;
		};

		struct CollisionDisplayMode
		{
			const static uint32_t multipleSelected = 0;
			const static uint32_t consoleSelected = 1;
			const static uint32_t inRange = 2;
		};

		struct NavmeshMode
		{
			const static uint32_t runtime = 0;
			const static uint32_t creationKit = 1;
		};

		// Main
		static inline uint32_t openMenuHotkey;
		static inline uint32_t updateRate;
		static inline uint32_t dayNightIndex;
		static inline uint32_t collisionDisplayIndex;
		static inline uint32_t navmeshModeIndex;
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
		static inline bool showNavmeshTriangles;
		static inline bool showNavmeshCover;
		static inline bool showBoxes;
		static inline bool showOcclusion;
		static inline bool showCollisionMarkers;
		static inline bool showCoordinates;
		static inline bool showCoverBeams;
		static inline bool showMarkers;
		static inline bool showCollision;
		static inline bool collisionOcclude;
		static inline bool cleanCollisions;
		static inline bool showCharController;
		static inline bool useD3DMCMSetting; // Setting read from MCM
		static inline bool useD3D; // setting which actually enableds funcitonality; This is sat in main after the first read of MCM settings, and will not be changed elsewhere; a restart is required to change it
		static inline bool showToolTips;

		static inline float infoRange;
		static inline float canvasScale;
		static inline float cellWallsHeight;
		static inline float rangeStep;
		static inline float navmeshRange;
		static inline float boxesRange;
		static inline float markersRange;
		static inline float collisionRange;

		// FreeCam
		static inline uint32_t	ascendHotkey;
		static inline uint32_t	descendHotkey;
		static inline bool		enableFreeCamOnOpen;
		static inline bool		useDoubleAscendToFly;
		static inline bool		lockFreeCamToZPlane;
		static inline bool		playerFollowsCamera;
		static inline bool		useTFC;

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
		static inline uint32_t collisionMarkerColor;
		static inline uint32_t collisionMarkerBorderColor;
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
		static inline uint32_t collisionMarkerAlpha;
		static inline uint32_t collisionMarkerBorderAlpha;
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


		static inline bool logUI;
		static inline float uiScale;

	};

	class MarkerSettings
	{
		public:
			static MarkerSettings* GetSingleton()
			{
				static MarkerSettings singleton;
				return std::addressof(singleton);
			}

			class ShowMarkerSetting
			{
				public:
					ShowMarkerSetting(std::string a_label, std::string a_instanceName) : label(a_label), instanceName(a_instanceName) {}
				
					bool& GetRef() { return enabled; }
					bool* GetPtr() { return &enabled; }
					bool  IsEnabled()
					{
						if (parentSetting)
						{
							if (parentSetting->enabled) return enabled;
							else return false;
						}
						else return enabled;
					}
					void AddChildSetting(std::string a_label, std::string a_instanceName, ShowMarkerSetting*& a_referencePtr)
					{
						auto newSetting = std::make_unique<ShowMarkerSetting>(a_label, a_instanceName);
						newSetting->parentSetting = this;
						a_referencePtr = newSetting.get();
						subSettings.emplace_back(std::move(newSetting));
					}
					std::string GetLabel() const { return label; }
					std::string GetInstanceName() const { return instanceName; }
					std::vector<std::unique_ptr<ShowMarkerSetting>>& GetSubSettings() { return subSettings; }
					
					bool				HasParent() const { return parentSetting ? true : false; }
					ShowMarkerSetting*	GetParent() const { return parentSetting; }

				//private:
					std::string										label = ""s;
					std::string										instanceName = ""s;
					bool											enabled = false;
					ShowMarkerSetting*								parentSetting = nullptr;
					std::vector<std::unique_ptr<ShowMarkerSetting>> subSettings{}; 

			};

			void InitShowMarkerSettings();

			bool areMarkerSettingsInitialized = false;

			ShowMarkerSetting* showFurnitureMarkers = nullptr;
			ShowMarkerSetting* showSitMarkers = nullptr;
			ShowMarkerSetting* showLeanMarkers = nullptr;
			ShowMarkerSetting* showSleepMarkers = nullptr;

			ShowMarkerSetting* showLightMarkers = nullptr;
			ShowMarkerSetting* showOmniMarkers = nullptr;
			ShowMarkerSetting* showShadowOmniMarkers = nullptr;
			ShowMarkerSetting* showShadowSpotMarkers = nullptr;
			ShowMarkerSetting* showShadowHemiMarkers = nullptr;

			ShowMarkerSetting* showMiscMarkers = nullptr;
			ShowMarkerSetting* showIdleMarkers = nullptr;
			ShowMarkerSetting* showSoundMarkers = nullptr;
			ShowMarkerSetting* showDragonMarkers = nullptr;
			ShowMarkerSetting* showCloudMarkers = nullptr;
			ShowMarkerSetting* showCritterMarkers = nullptr;
			ShowMarkerSetting* showFloraMarkers = nullptr;
			ShowMarkerSetting* showHazardMarkers = nullptr;

			ShowMarkerSetting* showMovableStaticMarkers = nullptr;
			ShowMarkerSetting* showMistMarkers = nullptr;
			ShowMarkerSetting* showLightBeamMarkers = nullptr;
			ShowMarkerSetting* showOtherMSTTMarkers = nullptr;

			ShowMarkerSetting* showStaticMarkers = nullptr;
			ShowMarkerSetting* showXMarkers = nullptr;
			ShowMarkerSetting* showHeadingMarkers = nullptr;
			ShowMarkerSetting* showDoorTeleportMarkers = nullptr;
			ShowMarkerSetting* showOtherStaticMarkers = nullptr;

			ShowMarkerSetting* showActivatorMarkers = nullptr;
			ShowMarkerSetting* showImpactMarkers = nullptr;
			ShowMarkerSetting* showOtherActivatorMarkers = nullptr;

			ShowMarkerSetting* showCWMarkers = nullptr;
			ShowMarkerSetting* showCWAttackerMarkers = nullptr;
			ShowMarkerSetting* showCWDefenderMarkers = nullptr;
			ShowMarkerSetting* showOtherCWMarkers = nullptr;
		

			std::vector<std::unique_ptr<ShowMarkerSetting>> markerGroupSettings;

		private:

			void AddMarkerGroupSetting(std::string a_label, std::string a_instanceName, ShowMarkerSetting*& a_referencePtr)
			{
				markerGroupSettings.push_back(std::make_unique<ShowMarkerSetting>(a_label, a_instanceName));
				a_referencePtr = markerGroupSettings.back().get();
			}


		//static inline bool GetShowFurnitureMarkers()		{ return showFurnitureMarkers; }
		//static inline bool GetShowSitMarkers()				{ return showFurnitureMarkers && showSitMarkers; }
		//static inline bool GetShowLeanMarkers()				{ return showFurnitureMarkers && showLeanMarkers; }
		//static inline bool GetShowSleepMarkers()			{ return showFurnitureMarkers && showSleepMarkers; }

		//static inline bool GetShowLightMarkers()			{ return showLightMarkers; }
		//static inline bool GetShowOmniMarkers()				{ return showLightMarkers && showOmniMarkers;}
		//static inline bool GetShowShadowOmniMarkers()		{ return showLightMarkers && showShadowOmniMarkers;}
		//static inline bool GetShowShadowSpotMarkers()		{ return showLightMarkers && showShadowSpotMarkers;}
		//static inline bool GetShowShadowHemiMarkers()		{ return showLightMarkers && showShadowHemiMarkers;}


		//static inline bool GetShowMiscMarkers()				{ return showMiscMarkers; }
		//static inline bool GetShowIdleMarkers()				{ return showMiscMarkers && showIdleMarkers; }
		//static inline bool GetShowSoundMarkers()			{ return showMiscMarkers && showSoundMarkers; }
		//static inline bool GetShowDragonMarkers()			{ return showMiscMarkers && showDragonMarkers; }
		//static inline bool GetShowCloudMarkers()			{ return showMiscMarkers && showCloudMarkers; }
		//static inline bool GetShowCritterMarkers()			{ return showMiscMarkers && showCritterMarkers; }
		//static inline bool GetShowFloraMarkers()			{ return showMiscMarkers && showFloraMarkers; }
		//static inline bool GetShowHazardMarkers()			{ return showMiscMarkers && showHazardMarkers; }
		//static inline bool GetShowTextureSetMarkers()		{ return showMiscMarkers && showTextureSetMarkers; }

		//static inline bool GetShowMovableStaticMarkers()	{ return showMovableStaticMarkers; }
		//static inline bool GetShowMistMarkers()				{ return showMovableStaticMarkers && showMistMarkers; }
		//static inline bool GetShowLightBeamMarkers()		{ return showMovableStaticMarkers && showLightBeamMarkers; }
		//static inline bool GetShowOtherMSTTMarkers()		{ return showMovableStaticMarkers && showOtherMSTTMarkers; }

		//static inline bool GetShowStaticMarkers()			{ return showStaticMarkers; }
		//static inline bool GetShowXMarkers()				{ return showStaticMarkers && showXMarkers; }
		//static inline bool GetShowHeadingMarkers()			{ return showStaticMarkers && showHeadingMarkers; }
		//static inline bool GetShowDoorTeleportMarkers()		{ return showStaticMarkers && showDoorTeleportMarkers; }
		//static inline bool GetShowOtherStaticMarkers()		{ return showStaticMarkers && showOtherStaticMarkers; }

		//static inline bool GetShowActivatorMarkers()		{ return showActivatorMarkers; }
		//static inline bool GetShowImpactMarkers()			{ return showActivatorMarkers && showImpactMarkers; }
		//static inline bool GetShowOtherActivatorMarkers()	{ return showActivatorMarkers && showOtherActivatorMarkers; }

		//static inline bool GetShowCWMarkers()				{ return showCWMarkers; }
		//static inline bool GetShowCWAttackerMarkers()		{ return showCWMarkers && showCWAttackerMarkers; }
		//static inline bool GetShowCWDefenderMarkers()		{ return showCWMarkers && showCWDefenderMarkers; }
		//static inline bool GetShowOtherCWMarkers()			{ return showCWMarkers && showOtherCWMarkers; }

	};

	static inline MarkerSettings* MSettings()
	{
		return MarkerSettings::GetSingleton();
	}
}
