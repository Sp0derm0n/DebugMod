#include "logger.h"
#include "EventSink.h"
#include "DrawMenu.h"
#include "hooks.h"
#include "DebugHandler.h"
#include "DebugUIMenu.h"
#include "MCM.h"
#include "Linalg.h"

void MessageHandler(SKSE::MessagingInterface::Message* a_message) {
    switch (a_message->type) 
	{
        case SKSE::MessagingInterface::kDataLoaded: 
		{
			MCM::Register();
			MCM::InitNonMCMSettings();
			MCM::DebugMenuMCM::ReadSettings(true);			

			RE::BSInputDeviceManager::GetSingleton()->AddEventSink(EventSink::GetSingleton());
			RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink<RE::TESCellFullyLoadedEvent>(EventSink::GetSingleton());

			DrawMenu::Register();
			DebugMenu::Register();
			DebugHandler::GetSingleton()->Init();

			Hooks::Hook_PlayerUpdate::install();
			Hooks::Hook_CellLoad::install();
			Hooks::Hook_NavMeshLoad::install();

			if (!MCM::settings::modActive)
			{
				logger::info("Plugin currently disabled in MCM");
			}
        }
		break;

    }
}

extern "C" __declspec( dllexport ) bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
    SKSE::Init(a_skse);
    SetupLog();
	logger::info("plugin loaded");

    const auto messaging = SKSE::GetMessagingInterface();
    messaging->RegisterListener(MessageHandler);

    return true;
}

extern "C" __declspec( dllexport ) constinit auto SKSEPlugin_Version = []() {
	SKSE::PluginVersionData v;
	v.PluginVersion(Version::VERSION);
	v.PluginName(Version::NAME);
	v.AuthorName("Spodermon");
	v.UsesAddressLibrary(true);
	v.CompatibleVersions({ SKSE::RUNTIME_SSE_LATEST });
	v.HasNoStructUse(true);

	return v;
}();


extern "C" __declspec( dllexport ) bool SKSEPlugin_Query(const SKSE::QueryInterface*, SKSE::PluginInfo* pluginInfo)
{
	pluginInfo->name = SKSEPlugin_Version.pluginName;
	pluginInfo->infoVersion = SKSE::PluginInfo::kVersion;
	pluginInfo->version = SKSEPlugin_Version.pluginVersion;
	return true;
}
