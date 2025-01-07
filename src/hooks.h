#pragma once

#include "DrawMenu.h"
#include "DebugHandler.h"
#include "MCM.h"

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
				_Update(player, a_delta);

				if (MCM::settings::modActive)
				{
					DebugHandler::GetSingleton()->Update(a_delta);
				}
			}
			static inline REL::Relocation<decltype(Update)> _Update;
	};

	class Hook_CellLoad 
	{
		public:
		static void install()
		{
			REL::Relocation<std::uintptr_t> TESObjectCELLVtbl{ RE::VTABLE_TESObjectCELL[0] }; 
			_Load = TESObjectCELLVtbl.write_vfunc(0x6, Load);
			logger::info("hook: CellLoad");

		}
		private:
		static void Load(RE::TESObjectCELL* a_cell, RE::TESFile* a_mod)
		{
			_Load(a_cell, a_mod);

			if (MCM::settings::modActive)
			{
				DebugHandler::GetSingleton()->OnCellLoad(a_cell);
			}
		}
		static inline REL::Relocation<decltype(Load)> _Load;
	};

	class Hook_NavMeshLoad 
	{
		public:
		static void install()
		{
			REL::Relocation<std::uintptr_t> NavMeshVtbl{ RE::VTABLE_NavMesh[0] }; 
			_Load = NavMeshVtbl.write_vfunc(0x6, Load);
			logger::info("hook: NavMeshLoad");

		}
		private:
		static void Load(RE::NavMesh* a_navmesh, RE::TESFile* a_mod)
		{
			_Load(a_navmesh, a_mod);

			if (MCM::settings::modActive && MCM::settings::showMoreNavmeshSourcefiles)
			{
				DebugHandler::GetSingleton()->OnNavMeshLoad(a_navmesh);
			}
		}
		static inline REL::Relocation<decltype(Load)> _Load;
	};
}