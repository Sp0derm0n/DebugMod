#include "logger.h"
#include "EventSink.h"
#include "DrawMenu.h"
#include "Hooks.h"
#include "DebugMenu/DebugMenu.h"
#include "DebugUIMenu.h"
#include "MCM.h"
#include "Renderer/Renderer.h"

void MessageHandler(SKSE::MessagingInterface::Message* a_message) {
    switch (a_message->type) 
	{
		case SKSE::MessagingInterface::kPostPostLoad:
		{
			MCM::Register();
			MCM::InitNonMCMSettings();
			MCM::DebugMenuPresets::Init();

			DrawMenu::Register();
			DebugMenuUI::Register();
			DebugMenu::GetDebugMenuHandler()->Init();

			Hooks::Hook_SoundMarkersSetFormEditorID::install();

			break;
		}
        case SKSE::MessagingInterface::kDataLoaded: 
		{
			RE::BSInputDeviceManager::GetSingleton()->AddEventSink(EventSink::GetSingleton());
			RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink<RE::TESCellFullyLoadedEvent>(DebugMenu::GetNavmeshHandler().get());
			RE::UI::GetSingleton()->AddEventSink<RE::MenuOpenCloseEvent>(DebugMenu::GetDebugMenuHandler().get());

			DebugMenu::GetMarkerHandler()->InitPostDataLoaded();

			SKSE::AllocTrampoline(14);
			Hooks::Hook_MainUpdate_sub_140437D40::Install();
			Hooks::Hook_CellLoad::install();
			Hooks::Hook_NavMeshLoad::install();

			MCM::settings::logUI = false;
			MCM::settings::uiScale = 1.0f;
			
			//bool* bShowMarkers = (bool*)RELOCATION_ID(508943, 381019).address();
			//*bShowMarkers = true;

			if (!MCM::settings::modActive)
			{
				logger::debug("Plugin currently disabled in MCM");
			}
			break;
        }
    }
}

extern "C" __declspec( dllexport ) bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
    SKSE::Init(a_skse);
    SetupLog();
	
	logger::debug("plugin loaded");

	//// Use to attach debugger to skyrim very early.
	//// When attaching CE to skyrim, use the processes tab instead of the applications tab
	/*auto time = std::chrono::system_clock::now();
	int waitSeconds = 10;*/

	/*while (true)
	{
		auto newTime = std::chrono::system_clock::now();
		auto passedTime = std::chrono::duration_cast<std::chrono::seconds>(newTime-time).count();
		if (passedTime > waitSeconds)
		{
			break;
		}
	}*/

	MCM::DebugMenuMCM::ReadSettings(true);
	MCM::settings::useD3D = MCM::settings::useD3DMCMSetting;

	if (!MCM::settings::useD3D)
	{
		logger::debug("D3D11 is disabled in MCM");
	}


    const auto messaging = SKSE::GetMessagingInterface();
    messaging->RegisterListener(MessageHandler);

	if (MCM::settings::useD3D)
	{
		logger::debug("D3D11 is enabled in MCM");

		if (!Renderer::AttachD3D())
		{
			logger::debug("AttachD3D failed");
			WarningPopup(L"DebugMenu: AttachD3D failed. D3D11 rendering will be disabled");
			MCM::settings::useD3D = false;
		}
	}
	
    return true;
}

extern "C" __declspec( dllexport ) constinit auto SKSEPlugin_Version = []() {
	SKSE::PluginVersionData v;
	v.PluginVersion(Version::VERSION);
	v.PluginName(Version::NAME);
	v.AuthorName("Spodermon");
	v.UsesAddressLibrary();
	v.CompatibleVersions({ SKSE::RUNTIME_SSE_LATEST });
	v.UsesNoStructs();

	return v;
}();

extern "C" __declspec( dllexport ) bool SKSEPlugin_Query(const SKSE::QueryInterface*, SKSE::PluginInfo* pluginInfo)
{
	pluginInfo->name = SKSEPlugin_Version.pluginName;
	pluginInfo->infoVersion = SKSE::PluginInfo::kVersion;
	pluginInfo->version = SKSEPlugin_Version.pluginVersion;
	return true;
}
