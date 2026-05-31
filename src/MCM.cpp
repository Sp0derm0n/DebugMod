#include "MCM.h"
#include <SimpleIni.h>
#include "DrawHandler.h"
#include "DebugMenu/DebugMenu.h"

namespace MCM
{
	void DebugMenuMCM::OnConfigClose(RE::TESQuest*)
	{
		ReadSettings();
		DebugMenu::GetDrawHandler()->UpdateCanvasScale();
		DebugMenu::GetMarkerHandler()->HideAllMarkers();
		DebugMenu::GetCollisionHandler()->HideAllCollisions();
		UpdateCollisionColor();
		if (!settings::modActive)
		{
			DebugMenu::GetDebugMenuHandler()->CloseDrawMenu();
			MCM::settings::showCellBorders = false;
			MCM::settings::showNavmesh = false;
			MCM::settings::showOcclusion = false;
			MCM::settings::showMarkers = false;
		}
	}

	void DebugMenuMCM::UpdateCollisionColor()
	{
		float red = (MCM::settings::collisionColorInt >> 16) / 255.0f;
		float green = ((MCM::settings::collisionColorInt >> 8) & 0xFF) / 255.0f;
		float blue = ((MCM::settings::collisionColorInt & 0xFF)) / 255.0f;
		float alpha = MCM::settings::collisionAlpha / 100.0f;
		MCM::settings::collisionColor = vec4u(red, green, blue, alpha);
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
		ReadBoolSetting(ini, "Main", "bCollisionRate",		settings::updateCollisionsEveryFrame);
		ReadBoolSetting(ini, "Main", "bShowCoverBeams",		settings::showCoverBeams);


		if (a_firstRead) // only change these settings when the menu is opened for the first time
		{
			ReadUInt32Setting(ini, "Main", "uDayNightMode",			settings::dayNightIndex);
			ReadUInt32Setting(ini, "Main", "uCollisionDisplayMode",	settings::collisionDisplayIndex);
			ReadUInt32Setting(ini, "Main", "uNavmeshDisplayMode",	settings::navmeshModeIndex);

			ReadBoolSetting(ini, "Main", "bShowCellBorderOnMenuOpen",		settings::showCellBorders);
			ReadBoolSetting(ini, "Main", "bShowCellBorderWallsOnMenuOpen",	settings::showCellWalls);
			ReadBoolSetting(ini, "Main", "bShowCellQuadsOnMenuOpen",		settings::showCellQuads);
			ReadBoolSetting(ini, "Main", "bShowNavmeshOnMenuOpen",			settings::showNavmesh);
			ReadBoolSetting(ini, "Main", "bShowNavmeshTrianglesOnMenuOpen",	settings::showNavmeshTriangles);
			ReadBoolSetting(ini, "Main", "bShowNavmeshCoverOnMenuOpen",		settings::showNavmeshCover);
			ReadBoolSetting(ini, "Main", "bShowBoxesOnMenuOpen",			settings::showBoxes);
			ReadBoolSetting(ini, "Main", "bShowOcclusionOnMenuOpen",		settings::showOcclusion);
			ReadBoolSetting(ini, "Main", "bShowCollisionMarkersOnMenuOpen",	settings::showCollisionMarkers);
			ReadBoolSetting(ini, "Main", "bShowCoordinatesOnMenuOpen",		settings::showCoordinates);
			ReadBoolSetting(ini, "Main", "bShowMarkersOnMenuOpen",			settings::showMarkers);
			ReadBoolSetting(ini, "Main", "bShowMarkersOnMenuOpen",			settings::showMarkers);
			ReadBoolSetting(ini, "Main", "bShowCollisionsOnMenuOpen",		settings::showCollision);
			ReadBoolSetting(ini, "Main", "bCollisionOcclude",				settings::collisionOcclude);
			ReadBoolSetting(ini, "Main", "bShowCharController",				settings::showCharController);
			ReadBoolSetting(ini, "Main", "bUseDirectX",						settings::useD3DMCMSetting);


			ReadFloatSetting(ini, "Main", "fNavmeshRange",				settings::navmeshRange);
			ReadFloatSetting(ini, "Main", "fOcclusionRange",			settings::boxesRange);
			ReadFloatSetting(ini, "Main", "fMarkersRange",				settings::markersRange);
			ReadFloatSetting(ini, "Main", "fCollisionRange",			settings::collisionRange);

			// Free Cam
			ReadBoolSetting(ini, "FreeCam", "bEnableOnFirstMenuOpen",	settings::enableFreeCamOnOpen);
			ReadBoolSetting(ini, "FreeCam", "bUseDoubleAscendToFly",	settings::useDoubleAscendToFly);
			ReadBoolSetting(ini, "FreeCam", "bLockFreeCamToZPlane",		settings::lockFreeCamToZPlane);
			ReadBoolSetting(ini, "FreeCam", "bPlayerFollowsCam",		settings::playerFollowsCamera);
		}

		ReadBoolSetting(ini, "Main", "bCleanCollisions", settings::cleanCollisions);
		

		ReadUInt32Setting(ini, "Main", "uMaxCollisions", settings::maxCollisions);

		ReadFloatSetting(ini, "Main", "fInfoRange",				settings::infoRange);
		ReadFloatSetting(ini, "Main", "fCanvasScale",			settings::canvasScale);
		ReadFloatSetting(ini, "Main", "fCellBorderWallsHeight", settings::cellWallsHeight);
		ReadFloatSetting(ini, "Main", "fRangeStep",				settings::rangeStep);
		
		// UI
		ReadBoolSetting(ini, "UI", "bShowToolTips", settings::showToolTips);


		// Free Cam
		ReadUInt32Setting(ini, "FreeCam", "uAscendHotkey",	settings::ascendHotkey);
		ReadUInt32Setting(ini, "FreeCam", "uDescendHotkey", settings::descendHotkey);

		ReadBoolSetting(ini, "FreeCam", "bUseTFC", settings::useTFC);

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
		ReadUInt32Setting(ini, "Colors", "uDisabledOcclusionColor",			settings::disabledOcclusionColor);
		ReadUInt32Setting(ini, "Colors", "uOcclusionBorderColor",			settings::occlusionBorderColor);
		ReadUInt32Setting(ini, "Colors", "uCollisionMarkerColor",			settings::collisionMarkerColor);
		ReadUInt32Setting(ini, "Colors", "uCollisionMarkerBorderColor",		settings::collisionMarkerBorderColor);
		ReadUInt32Setting(ini, "Colors", "uLightBulbInfoColor",				settings::lightBulbInfoColor);
		ReadUInt32Setting(ini, "Colors", "uSoundMarkerInfoColor",			settings::soundMarkerInfoColor);
		ReadUInt32Setting(ini, "Colors", "uCollisionColor",					settings::collisionColorInt);
		

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
		ReadUInt32Setting(ini, "Alpha", "uCollisionMarkerAlpha",		settings::collisionMarkerAlpha);
		ReadUInt32Setting(ini, "Alpha", "uCollisionMarkerBorderAlpha",	settings::collisionMarkerBorderAlpha);
		ReadUInt32Setting(ini, "Alpha", "uMarkerInfoAlpha",				settings::markerInfoAlpha);
		ReadUInt32Setting(ini, "Alpha", "uCollisionAlpha",				settings::collisionAlpha);

		ReadFloatSetting(ini, "Alpha", "fLightBulbAlpha",				settings::lightBulbAplha);

		// Advanced
		ReadBoolSetting(ini, "Advanced", "bShowMoreNavmeshSourcefiles",	settings::showMoreNavmeshSourcefiles);
		ReadBoolSetting(ini, "Advanced", "bShowNavmeshCoverInfo",		settings::showNavmeshCoverInfo);
		ReadBoolSetting(ini, "Advanced", "bShowNavmeshCoverLines",		settings::showNavmeshCoverLines);
		ReadBoolSetting(ini, "Advanced", "bShowMarkerInfo",				settings::showMarkerInfo);

		ReadUInt32Setting(ini, "Advanced", "uLinesHeight",				settings::linesHeight);
		ReadUInt32Setting(ini, "Advanced", "uCapsuleCylinderSegments",	settings::capsuleCylinderSegments);
		ReadUInt32Setting(ini, "Advanced", "uCapsuleSphereSegments",	settings::capsuleSphereSegments);

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
		settings::minRange = 500.0f;
		settings::maxRange = 15000.0f;
		MSettings()->InitShowMarkerSettings();
	}

	void DebugMenuPresets::Init()
	{
		if (!MSettings()->areMarkerSettingsInitialized)
		{
			logger::debug("ERROR: Tried to load marker settings before they were initialized");
			return;
		}
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

		ReadWriteSetting(ini, "Markers", "bFurniture",		a_save, MCM::MSettings()->showFurnitureMarkers->GetRef());
		ReadWriteSetting(ini, "Markers", "bSit",			a_save, MCM::MSettings()->showSitMarkers->GetRef());
		ReadWriteSetting(ini, "Markers", "bLean",			a_save, MCM::MSettings()->showLeanMarkers->GetRef());
		ReadWriteSetting(ini, "Markers", "bSleep",			a_save, MCM::MSettings()->showSleepMarkers->GetRef());

		ReadWriteSetting(ini, "Markers", "bLight",			a_save, MCM::MSettings()->showLightMarkers->GetRef());
		ReadWriteSetting(ini, "Markers", "bOmni",			a_save, MCM::MSettings()->showOmniMarkers->GetRef());
		ReadWriteSetting(ini, "Markers", "bShadowOmni",		a_save, MCM::MSettings()->showShadowOmniMarkers->GetRef());
		ReadWriteSetting(ini, "Markers", "bShadowSpot",		a_save, MCM::MSettings()->showShadowSpotMarkers->GetRef());
		ReadWriteSetting(ini, "Markers", "bShadowHemi",		a_save, MCM::MSettings()->showShadowHemiMarkers->GetRef());


		ReadWriteSetting(ini, "Markers", "bMisc",			a_save, MCM::MSettings()->showMiscMarkers->GetRef());
		ReadWriteSetting(ini, "Markers", "bIdle",			a_save, MCM::MSettings()->showIdleMarkers->GetRef());
		ReadWriteSetting(ini, "Markers", "bSound",			a_save, MCM::MSettings()->showSoundMarkers->GetRef());
		ReadWriteSetting(ini, "Markers", "bDragon",			a_save, MCM::MSettings()->showDragonMarkers->GetRef());
		ReadWriteSetting(ini, "Markers", "bCloud",			a_save, MCM::MSettings()->showCloudMarkers->GetRef());
		ReadWriteSetting(ini, "Markers", "bCritter",		a_save, MCM::MSettings()->showCritterMarkers->GetRef());
		ReadWriteSetting(ini, "Markers", "bFlora",			a_save, MCM::MSettings()->showFloraMarkers->GetRef());
		ReadWriteSetting(ini, "Markers", "bHazard",			a_save, MCM::MSettings()->showHazardMarkers->GetRef());
		//ReadWriteSetting(ini, "Markers", "bTextureSet",		a_save, MCM::MSettings()->showTextureSetMarkers);

		ReadWriteSetting(ini, "Markers", "bMovableStatic",	a_save, MCM::MSettings()->showMovableStaticMarkers->GetRef());
		ReadWriteSetting(ini, "Markers", "bMist",			a_save, MCM::MSettings()->showMistMarkers->GetRef());
		ReadWriteSetting(ini, "Markers", "bLightBeam",		a_save, MCM::MSettings()->showLightBeamMarkers->GetRef());
		ReadWriteSetting(ini, "Markers", "bOtherMSTT",		a_save, MCM::MSettings()->showOtherMSTTMarkers->GetRef());

		ReadWriteSetting(ini, "Markers", "bStatic",			a_save, MCM::MSettings()->showStaticMarkers->GetRef());
		ReadWriteSetting(ini, "Markers", "bX",				a_save, MCM::MSettings()->showXMarkers->GetRef());
		ReadWriteSetting(ini, "Markers", "bHeading",		a_save, MCM::MSettings()->showHeadingMarkers->GetRef());
		ReadWriteSetting(ini, "Markers", "bDoorTeleport",	a_save, MCM::MSettings()->showDoorTeleportMarkers->GetRef());
		ReadWriteSetting(ini, "Markers", "bOtherStatic",	a_save, MCM::MSettings()->showOtherStaticMarkers->GetRef());

		ReadWriteSetting(ini, "Markers", "bActivator",		a_save, MCM::MSettings()->showActivatorMarkers->GetRef());
		ReadWriteSetting(ini, "Markers", "bImpact",			a_save, MCM::MSettings()->showImpactMarkers->GetRef());
		ReadWriteSetting(ini, "Markers", "bOtherACTI",		a_save, MCM::MSettings()->showOtherActivatorMarkers->GetRef());

		ReadWriteSetting(ini, "Markers", "bCivilWar",		a_save, MCM::MSettings()->showCWMarkers->GetRef());
		ReadWriteSetting(ini, "Markers", "bCWAttacker",		a_save, MCM::MSettings()->showCWAttackerMarkers->GetRef());
		ReadWriteSetting(ini, "Markers", "bCWDefender",		a_save, MCM::MSettings()->showCWDefenderMarkers->GetRef());
		ReadWriteSetting(ini, "Markers", "bOtherCW",		a_save, MCM::MSettings()->showOtherCWMarkers->GetRef());

		if (!a_save && a_presetIndex == 0)
		{
			logger::debug("Loaded previous session's marker settings");
		}

		if (a_save)
		{
			ini.SaveFile(fileName.c_str());
		}
	}

	void MCM::MarkerSettings::InitShowMarkerSettings()
	{
		logger::debug("Initializing marker selection settings");

		AddMarkerGroupSetting("Furniture", "ShowFurnitureButton", showFurnitureMarkers);
		showFurnitureMarkers->AddChildSetting("Sit",	"ShowSitMarkersButton",		showSitMarkers);
		showFurnitureMarkers->AddChildSetting("Lean",	"ShowLeanMarkersButton",	showLeanMarkers);
		showFurnitureMarkers->AddChildSetting("Sleep",	"ShowSleepmarkersButton",	showSleepMarkers);

		AddMarkerGroupSetting("Light bulbs", "ShowLightMarkersButton", showLightMarkers);
		showLightMarkers->AddChildSetting("Omni",			"ShowOmniLightMarkersButton",		showOmniMarkers);
		showLightMarkers->AddChildSetting("Shadow omni",	"ShowShadowOmniLightMarkersButton", showShadowOmniMarkers);
		showLightMarkers->AddChildSetting("Shadow spot",	"ShowShadowSpotLightMarkersButton", showShadowSpotMarkers);
		showLightMarkers->AddChildSetting("Shadow hemi",	"ShowShadowHemiLightMarkersButton", showShadowHemiMarkers);

		AddMarkerGroupSetting("Misc", "ShowMiscMarkersButton", showMiscMarkers);
		showMiscMarkers->AddChildSetting("Idle markers",	"ShowIdleMarkersButton",	showIdleMarkers);
		showMiscMarkers->AddChildSetting("Sound markers",	"ShowSoundMarkersButton",	showSoundMarkers);
		showMiscMarkers->AddChildSetting("Dragon markers",	"ShowDragonMarkersButton",	showDragonMarkers);
		showMiscMarkers->AddChildSetting("Cloud markers",	"ShowCloudMarkersButton",	showCloudMarkers);
		showMiscMarkers->AddChildSetting("Critter markers",	"ShowCritterMarkersButton", showCritterMarkers);
		showMiscMarkers->AddChildSetting("Flora markers",	"ShowFloraMarkersButton",	showFloraMarkers);
		showMiscMarkers->AddChildSetting("Hazard markers",	"ShowHazardMarkersButton",	showHazardMarkers);

		AddMarkerGroupSetting("Movable Statics", "ShowMSTTMarkersButton", showMovableStaticMarkers);
		showMovableStaticMarkers->AddChildSetting("Mist",			"ShowMistMarkersButton",		showMistMarkers);
		showMovableStaticMarkers->AddChildSetting("Light beams",	"ShowLightBeamMarkersButton",	showLightBeamMarkers);
		showMovableStaticMarkers->AddChildSetting("Other MSTT",		"ShowOtherMSTTMarkersButton",	showOtherMSTTMarkers);

		AddMarkerGroupSetting("Statics", "ShowStaticMarkersButton", showStaticMarkers);
		showStaticMarkers->AddChildSetting("X marker",		"ShowXMarkersButton",				showXMarkers);
		showStaticMarkers->AddChildSetting("Heading",		"ShowHeadingMarkersButton",			showHeadingMarkers);
		showStaticMarkers->AddChildSetting("Door teleport", "ShowDoorTeleportMarkersButton",	showDoorTeleportMarkers);
		showStaticMarkers->AddChildSetting("Other statics", "ShowOtherStaticMarkersButton",		showOtherStaticMarkers);

		AddMarkerGroupSetting("Activators", "ShowActivatorMarkersButton", showActivatorMarkers);
		showActivatorMarkers->AddChildSetting("Impact markers",		"ShowImpactMarkersButton",			showImpactMarkers);
		showActivatorMarkers->AddChildSetting("Other activators",	"ShowOtherActivatorsMarkersButto",	showOtherActivatorMarkers);

		AddMarkerGroupSetting("Civli War", "ShowCivilWarMarkersButton", showCWMarkers);
		showCWMarkers->AddChildSetting("Attacker", "ShowCWAttackerMarkersButton",	showCWAttackerMarkers);
		showCWMarkers->AddChildSetting("Defender", "ShowCWDefenderMarkersButton",	showCWDefenderMarkers);
		showCWMarkers->AddChildSetting("Other CW", "ShowOtherCWMarkersButton",		showOtherCWMarkers);

		areMarkerSettingsInitialized = true;

		logger::debug("Finished initializing marker selection settings");

	}
}
