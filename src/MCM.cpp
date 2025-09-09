#include "MCM.h"
#include <SimpleIni.h>
#include "DrawHandler.h"
#include "DebugHandler.h"
#include "UIHandler.h"

namespace MCM
{
	void DebugMenuMCM::OnConfigClose(RE::TESQuest*)
	{
		ReadSettings();
		DrawHandler::GetSingleton()->UpdateCanvasScale();
		DebugHandler::GetSingleton()->HideAllMarkers();
		if (!settings::modActive)
		{
			DebugHandler::GetSingleton()->CloseDrawMenu();
			UIHandler::GetSingleton()->Unload();
			MCM::settings::showCellBorders = false;
			MCM::settings::showNavmesh = false;
			MCM::settings::showOcclusion = false;
			MCM::settings::showMarkers = false;
		}
	}

	void DebugMenuMCM::ReadSettingsFromPath(std::filesystem::path a_path, bool a_firstRead)
	{
		CSimpleIniA ini;
		ini.SetUnicode();
		ini.LoadFile(a_path.string().c_str());

		// Main
		ReadUInt32Setting(ini, "Main", "uOpenMenuHotkey",	settings::openMenuHotkey);
		ReadUInt32Setting(ini, "Main", "uScrollUpHotkey",	settings::scrollUpHotkey);
		ReadUInt32Setting(ini, "Main", "uScrollDownHotkey",	settings::scrollDownHotkey);
		ReadUInt32Setting(ini, "Main", "uUpdateRate",		settings::updateRate);

		ReadBoolSetting(ini, "Main", "bModActive",			settings::modActive);
		ReadBoolSetting(ini, "Main", "bShowInfoOnHover",	settings::showInfoOnHover);
		ReadBoolSetting(ini, "Main", "bShowCrosshair",		settings::showCrosshair);
		ReadBoolSetting(ini, "Main", "bShowCanvasBorder",	settings::showCanvasBorder);
		ReadBoolSetting(ini, "Main", "bShowCoverBeams",		settings::showCoverBeams);


		if (a_firstRead) // only change these settings when the menu is opened for the first time
		{
			ReadUInt32Setting(ini, "Main", "uDayNightMode",		settings::dayNightIndex);

			ReadBoolSetting(ini, "Main", "bShowCellBorderOnMenuOpen",		settings::showCellBorders);
			ReadBoolSetting(ini, "Main", "bShowCellBorderWallsOnMenuOpen",	settings::showCellWalls);
			ReadBoolSetting(ini, "Main", "bShowCellQuadsOnMenuOpen",		settings::showCellQuads);
			ReadBoolSetting(ini, "Main", "bShowNavmeshOnMenuOpen",			settings::showNavmesh);
			ReadBoolSetting(ini, "Main", "bShowNavmeshTrianglesOnMenuOpen",	settings::showNavmeshTriangles);
			ReadBoolSetting(ini, "Main", "bShowNavmeshCoverOnMenuOpen",		settings::showNavmeshCover);
			ReadBoolSetting(ini, "Main", "bShowOcclusionOnMenuOpen",		settings::showOcclusion);
			ReadBoolSetting(ini, "Main", "bShowCoordinatesOnMenuOpen",		settings::showCoordinates);
			ReadBoolSetting(ini, "Main", "bShowMarkersOnMenuOpen",			settings::showMarkers);

			ReadFloatSetting(ini, "Main", "fNavmeshRange",			settings::navmeshRange);
			ReadFloatSetting(ini, "Main", "fOcclusionRange",		settings::occlusionRange);
			ReadFloatSetting(ini, "Main", "fMarkersRange",			settings::markersRange);
		}
		
		ReadFloatSetting(ini, "Main", "fInfoRange",				settings::infoRange);
		ReadFloatSetting(ini, "Main", "fCanvasScale",			settings::canvasScale);
		ReadFloatSetting(ini, "Main", "fCellBorderWallsHeight", settings::cellWallsHeight);
		ReadFloatSetting(ini, "Main", "fRangeStep",				settings::rangeStep);
		

		// Colors
		ReadUInt32Setting(ini, "Colors", "uCellBorderColor",				settings::cellBorderColor);
		ReadUInt32Setting(ini, "Colors", "uCellBorderWallsColor",			settings::cellWallsColor);
		ReadUInt32Setting(ini, "Colors", "uCellQuadsColor",					settings::cellQuadsColor);
		ReadUInt32Setting(ini, "Colors", "uNavmeshColor",					settings::navmeshColor);
		ReadUInt32Setting(ini, "Colors", "uNavmeshDoorColor",				settings::navmeshDoorColor);
		ReadUInt32Setting(ini, "Colors", "uNavmeshWaterColor",				settings::navmeshWaterColor);
		ReadUInt32Setting(ini, "Colors", "uNavmeshPrefferedPathingColor",	settings::navmeshPrefferedColor);
		ReadUInt32Setting(ini, "Colors", "uNavmeshCellEdgeLinkColor",		settings::navmeshCellEdgeLinkColor);
		ReadUInt32Setting(ini, "Colors", "uNavmeshLedgeEdgeLinkColor",		settings::navmeshLedgeEdgeLinkColor);
		ReadUInt32Setting(ini, "Colors", "uNavmeshCoverColor",				settings::navmeshCoverColor);
		ReadUInt32Setting(ini, "Colors", "uNavmeshCoverBorderColor",		settings::navmeshCoverBorderColor);
		ReadUInt32Setting(ini, "Colors", "uNavmeshMaxCoverColor",			settings::navmeshMaxCoverColor);
		ReadUInt32Setting(ini, "Colors", "uNavmeshMaxCoverBorderColor",		settings::navmeshMaxCoverBorderColor);
		ReadUInt32Setting(ini, "Colors", "uOcclusionColor",					settings::occlusionColor);
		ReadUInt32Setting(ini, "Colors", "uOcclusionBorderColor",			settings::occlusionBorderColor);
		ReadUInt32Setting(ini, "Colors", "uLightBulbInfoColor",				settings::lightBulbInfoColor);
		ReadUInt32Setting(ini, "Colors", "uSoundMarkerInfoColor",			settings::soundMarkerInfoColor);
		

		// Alpha
		ReadUInt32Setting(ini, "Alpha", "uCellBorderAlpha",				settings::cellBorderAlpha);
		ReadUInt32Setting(ini, "Alpha", "uCellBorderWallsAlpha",		settings::cellWallsAlpha);
		ReadUInt32Setting(ini, "Alpha", "uCellQuadsAlpha",				settings::cellQuadsAlpha);
		ReadUInt32Setting(ini, "Alpha", "uNavmeshTrianglesAlpha",		settings::navmeshAlpha);
		ReadUInt32Setting(ini, "Alpha", "uNavmeshBordersAlpha",			settings::navmeshBorderAlpha);
		ReadUInt32Setting(ini, "Alpha", "uNavmeshEdgeLinkAlpha",		settings::navmeshEdgeLinkAlpha);
		ReadUInt32Setting(ini, "Alpha", "uNavmeshCoverAlpha",			settings::navmeshCoverAlpha);
		ReadUInt32Setting(ini, "Alpha", "uNavmeshCoverBorderAlpha",		settings::navmeshCoverBorderAlpha);
		ReadUInt32Setting(ini, "Alpha", "uNavmeshMaxCoverAlpha",		settings::navmeshMaxCoverAlpha);
		ReadUInt32Setting(ini, "Alpha", "uNavmeshMaxCoverBorderAlpha",	settings::navmeshMaxCoverBorderAlpha);
		ReadUInt32Setting(ini, "Alpha", "uOcclusionAlpha",				settings::occlusionAlpha);
		ReadUInt32Setting(ini, "Alpha", "uOcclusionBorderAlpha",		settings::occlusionBorderAlpha);
		ReadUInt32Setting(ini, "Alpha", "uMarkerInfoAlpha",				settings::markerInfoAlpha);

		ReadFloatSetting(ini, "Alpha", "fLightBulbAlpha",				settings::lightBulbAplha);

		// Advanced
		ReadBoolSetting(ini, "Advanced", "bShowMoreNavmeshSourcefiles",	settings::showMoreNavmeshSourcefiles);
		ReadBoolSetting(ini, "Advanced", "bShowNavmeshCoverInfo",		settings::showNavmeshCoverInfo);
		ReadBoolSetting(ini, "Advanced", "bShowNavmeshCoverLines",		settings::showNavmeshCoverLines);
		ReadBoolSetting(ini, "Advanced", "bShowMarkerInfo",				settings::showMarkerInfo);

		ReadUInt32Setting(ini, "Advanced", "uLinesHeight",		settings::linesHeight);

	}

	void DebugMenuMCM::ReadSettings(bool a_firstRead)
	{
		constexpr auto defaultSettingsPath = L"Data/MCM/Config/DebugMenu/settings.ini";
		constexpr auto mcmPath = L"Data/MCM/Settings/DebugMenu.ini";

		ReadSettingsFromPath(defaultSettingsPath, a_firstRead);
		ReadSettingsFromPath(mcmPath, a_firstRead);
	}

	void DebugMenuMCM::ReadBoolSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_varName, bool& a_var) 
	{
		const char* settingExists = nullptr;
		settingExists = a_ini.GetValue(a_sectionName, a_varName);
		if (settingExists)
		{
			a_var = a_ini.GetBoolValue(a_sectionName, a_varName);
		}
	}

	void DebugMenuMCM::ReadFloatSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_varName, float& a_var) 
	{
		const char* settingExists = nullptr;
		settingExists = a_ini.GetValue(a_sectionName, a_varName);
		if (settingExists)
		{
			a_var = static_cast<float>(a_ini.GetDoubleValue(a_sectionName, a_varName));
		}
	}

	void DebugMenuMCM::ReadUInt32Setting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_varName, uint32_t& a_var) 
	{
		const char* settingExists = nullptr;
		settingExists = a_ini.GetValue(a_sectionName, a_varName);
		if (settingExists)
		{
			a_var = static_cast<uint32_t>(a_ini.GetLongValue(a_sectionName, a_varName));
		}
	}

	bool DebugMenuMCM::Register(RE::BSScript::IVirtualMachine* a_vm)
	{
		a_vm->RegisterFunction("OnConfigClose", "DebugMenuMCM", OnConfigClose);

		logger::debug("Registered DebugMenuMCM class");
		return true;
	}

	void Register()
	{
		auto papyrus = SKSE::GetPapyrusInterface();
		papyrus->Register(DebugMenuMCM::Register);
		logger::debug("Registered papyrus functions");
	}

	void InitNonMCMSettings()
	{
		settings::useRuntimeNavmesh = false;
		settings::minRange = 500.0f;
		settings::maxRange = 15000.0f;
	}

	void DebugMenuPresets::Init()
	{
		LoadMarkerSettings(0); // preset 0 = last session
	}

	void DebugMenuPresets::ReadWriteSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_varName, bool a_write, bool& a_variable)
	{
		if (a_write)
		{
			a_ini.SetBoolValue(a_sectionName, a_varName, a_variable);
		}
		else
		{
			DebugMenuMCM::ReadBoolSetting(a_ini, a_sectionName, a_varName, a_variable);
		}
	}

	void DebugMenuPresets::LoadMarkerSettings(uint32_t a_presetIndex)
	{
		SaveLoadSettings(a_presetIndex, false);

	}
	void DebugMenuPresets::SaveMarkerSettings(uint32_t a_presetIndex)
	{
		SaveLoadSettings(a_presetIndex, true);
	}

	void DebugMenuPresets::SaveLoadSettings(uint32_t a_presetIndex, bool a_save)
	{
		std::wstring fileName = L"Data/SKSE/plugins/DebugMenu/DM_Preset_" + std::to_wstring(a_presetIndex) + L".ini"s;
		CSimpleIniA ini;
		ini.SetUnicode();

		if (!a_save)
		{
			SI_Error rc = ini.LoadFile(fileName.c_str());
			if (rc < 0)
			{
				if (a_presetIndex == 0) logger::debug("Failed to load previous session's marker settings");
				return;
			}
		}

		ReadWriteSetting(ini, "Markers", "bFurniture",		a_save, MCM::settings::showFurnitureMarkers);
		ReadWriteSetting(ini, "Markers", "bSit",			a_save, MCM::settings::showSitMarkers);
		ReadWriteSetting(ini, "Markers", "bLean",			a_save, MCM::settings::showLeanMarkers);
		ReadWriteSetting(ini, "Markers", "bSleep",			a_save, MCM::settings::showSleepMarkers);

		ReadWriteSetting(ini, "Markers", "bMisc",			a_save, MCM::settings::showMiscMarkers);
		ReadWriteSetting(ini, "Markers", "bLight",			a_save, MCM::settings::showLightMarkers);
		ReadWriteSetting(ini, "Markers", "bIdle",			a_save, MCM::settings::showIdleMarkers);
		ReadWriteSetting(ini, "Markers", "bSound",			a_save, MCM::settings::showSoundMarkers);
		ReadWriteSetting(ini, "Markers", "bDragon",			a_save, MCM::settings::showDragonMarkers);
		ReadWriteSetting(ini, "Markers", "bCloud",			a_save, MCM::settings::showCloudMarkers);
		ReadWriteSetting(ini, "Markers", "bCritter",		a_save, MCM::settings::showCritterMarkers);
		ReadWriteSetting(ini, "Markers", "bFlora",			a_save, MCM::settings::showFloraMarkers);
		ReadWriteSetting(ini, "Markers", "bHazard",			a_save, MCM::settings::showHazardMarkers);
		//ReadWriteSetting(ini, "Markers", "bTextureSet",		a_save, MCM::settings::showTextureSetMarkers);

		ReadWriteSetting(ini, "Markers", "bMovableStatic",	a_save, MCM::settings::showMovableStaticMarkers);
		ReadWriteSetting(ini, "Markers", "bMist",			a_save, MCM::settings::showMistMarkers);
		ReadWriteSetting(ini, "Markers", "bLightBeam",		a_save, MCM::settings::showLightBeamMarkers);
		ReadWriteSetting(ini, "Markers", "bOtherMSTT",		a_save, MCM::settings::showOtherMSTTMarkers);

		ReadWriteSetting(ini, "Markers", "bStatic",			a_save, MCM::settings::showStaticMarkers);
		ReadWriteSetting(ini, "Markers", "bX",				a_save, MCM::settings::showXMarkers);
		ReadWriteSetting(ini, "Markers", "bHeading",		a_save, MCM::settings::showHeadingMarkers);
		ReadWriteSetting(ini, "Markers", "bDoorTeleport",	a_save, MCM::settings::showDoorTeleportMarkers);
		ReadWriteSetting(ini, "Markers", "bOtherStatic",	a_save, MCM::settings::showOtherStaticMarkers);

		ReadWriteSetting(ini, "Markers", "bActivator",		a_save, MCM::settings::showActivatorMarkers);
		ReadWriteSetting(ini, "Markers", "bImpact",			a_save, MCM::settings::showImpactMarkers);
		ReadWriteSetting(ini, "Markers", "bOtherACTI",		a_save, MCM::settings::showOtherActivatorMarkers);

		ReadWriteSetting(ini, "Markers", "bCivilWar",		a_save, MCM::settings::showCWMarkers);
		ReadWriteSetting(ini, "Markers", "bCWAttacker",		a_save, MCM::settings::showCWAttackerMarkers);
		ReadWriteSetting(ini, "Markers", "bCWDefender",		a_save, MCM::settings::showCWDefenderMarkers);
		ReadWriteSetting(ini, "Markers", "bOtherCW",		a_save, MCM::settings::showOtherCWMarkers);

		if (!a_save && a_presetIndex == 0)
		{
			logger::debug("Loaded previous session's marker settings");
		}

		if (a_save)
		{
			ini.SaveFile(fileName.c_str());
		}
	}
}
