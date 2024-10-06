#include "logger.h"
#include "EventSink.h"
#include "DrawMenu.h"
#include "hooks.h"
#include "HUDHandler.h"
#include "DebugHandler.h"

void MessageHandler(SKSE::MessagingInterface::Message* a_message) {
    switch (a_message->type) {
        case SKSE::MessagingInterface::kDataLoaded: {
			RE::BSInputDeviceManager::GetSingleton()->AddEventSink(EventSink::GetSingleton());
			DrawMenu::Register();
			DebugHandler::GetSingleton()->init();
			HUDHandler::GetSingleton()->init();
			//SKSE::AllocTrampoline(14);
			Hooks::Hook_PlayerUpdate::install();
			//Hooks::OnUpdateHook::Install();
			//Hooks::Hook_CameraUpdate::Install();
			//Hooks::MainUpdateHook::Install();
        }
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
