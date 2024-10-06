#pragma once

#include "DrawMenu.h"
#include "HUDHandler.h"
#include "DebugHandler.h"


static void FPSGetCurrentPosition(RE::FirstPersonState* a_fps, RE::NiPoint3& a_position)
	{
		using func_t = decltype(&FPSGetCurrentPosition);
		REL::Relocation<func_t> func{ RELOCATION_ID(51246, 50726) }; //SE ID NOT CORRECT
		return func(a_fps, a_position);
	}

namespace Hooks
{
	class Hook_PlayerUpdate 
	{
		public:
			static void install()
			{
				REL::Relocation<std::uintptr_t> PlayerCharacterVtbl{ RE::VTABLE_PlayerCharacter[0] }; 
				_Update = PlayerCharacterVtbl.write_vfunc(0xAD, Update);
				logger::info("hook: PlayerUpdate");
			}

		private:
			static void Update(RE::PlayerCharacter* player, float a_delta)
			{
				_Update(player, a_delta); // without this, the original function is not called by the game
				DebugHandler::GetSingleton()->update();
			}
			static inline REL::Relocation<decltype(Update)> _Update;
	};

	class MainUpdateHook
	{
	public:
	static void Install()
	{
		logger::info("MainUpdateHook");
			
		auto& trampoline = SKSE::GetTrampoline();
		REL::Relocation<uintptr_t> hook{ RELOCATION_ID(35551, 36544), REL::VariantOffset(0x11F, 0x160, 0x160) };  // main loop
		_Update = trampoline.write_call<5>(hook.address(), Update);

	}

	private:
		static void Update(RE::Main* a_this, float a2)
		{
			_Update(a_this, a2);
			DebugHandler::GetSingleton()->update();
			
		}
		static inline REL::Relocation<decltype(Update)> _Update;
	};



	class OnUpdateHook 
	{
		public:
			static void Install() 
			{
				auto address = REL::VariantID(35565, 36564, 0x5BAB10).address(); //69510 (140CFD280) for random func called at the end of main__update (140645EA0)
				auto offset = REL::VariantOffset(0x748, 0x2a7, 0x7EE).offset(); //0xC26
				OnUpdate = SKSE::GetTrampoline().write_call<5>(address + offset, OnUpdateMod);
			}
		private:
			static void OnUpdateMod() {
				// call original function
				OnUpdate();
				//logger::info("hmmm");
				HUDHandler::GetSingleton()->updateHUD();
			}
			static inline REL::Relocation<decltype(OnUpdateMod)> OnUpdate;
	};
}