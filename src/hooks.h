#pragma once

#include "DebugMenu/DebugMenu.h"
#include "MCM.h"

namespace Hooks
{
	// not in use
	class Hook_PlayerUpdate 
	{
		public:
			static void install()
			{
				REL::Relocation<std::uintptr_t> PlayerCharacterVtbl{ RE::VTABLE_PlayerCharacter[0] };
				_Update = PlayerCharacterVtbl.write_vfunc(0xAD, Update);
				logger::debug("hook: PlayerUpdate");

			}
		private:
			static void Update(RE::PlayerCharacter* player, float a_delta)
			{
				_Update(player, a_delta);
			}
			static inline REL::Relocation<decltype(Update)> _Update;
	};

	// not in use
	class Hook_MainUpdate
	{
		public:
			static void Install()
			{
				logger::debug("Hook: MainUpdate");

				auto& trampoline = SKSE::GetTrampoline();
				REL::Relocation<uintptr_t> hook{ RELOCATION_ID(35551, 36544), REL::VariantOffset(0x11F, 0x160, 0x160) };  // main loop
				_Update = trampoline.write_call<5>(hook.address(), Update);

			}

		private:
			static void Update(RE::Main* a_this, float a_2)
			{
				_Update(a_this, a_2);

				if (MCM::settings::modActive)
				{
					DebugMenu::GetDebugMenuHandler()->Update();
				}
			}
			static inline REL::Relocation<decltype(Update)> _Update;
	};

	// post player_update update
	class Hook_MainUpdate_sub_140437D40
	{
		public:
			static void Install()
			{
				logger::debug("Hook: MainUpdate sub_140437D40");

				auto& trampoline = SKSE::GetTrampoline();
				REL::Relocation<uintptr_t> hook{ RELOCATION_ID(35565, 36564), REL::VariantOffset(0x2BB, 0x2D6, 0x2D6) };
				_Update = trampoline.write_call<5>(hook.address(), Update);
			}
		private:
			static void Update(RE::BSTreeManager* a_treeManager, RE::BSGeometryListCullingProcess* a_cullingProcess, bool a_a3)
			{
				_Update(a_treeManager, a_cullingProcess, a_a3);

				if (MCM::settings::modActive)
				{
					DebugMenu::GetDebugMenuHandler()->Update();
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
				logger::debug("hook: CellLoad");

			}
		private:
			static void Load(RE::TESObjectCELL* a_cell, RE::TESFile* a_mod)
			{
				_Load(a_cell, a_mod);

				if (MCM::settings::modActive)
				{
					DebugMenu::GetNavmeshHandler()->OnCellLoad(a_cell);
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
				logger::debug("hook: NavMeshLoad");

			}
		private:
			static bool Load(RE::NavMesh* a_navmesh, RE::TESFile* a_mod)
			{
				bool result = _Load(a_navmesh, a_mod);

				if (MCM::settings::modActive && MCM::settings::showMoreNavmeshSourcefiles)
				{
					DebugMenu::GetNavmeshHandler()->OnNavMeshLoad(a_navmesh);
				}
				return result;
			}
			static inline REL::Relocation<decltype(Load)> _Load;
	};

	class Hook_SoundMarkersSetFormEditorID
	{
		public:
			static void install()
			{
				//REL::Relocation<std::uintptr_t> BGSSoundDescriptorFormVtbl{ RE::VTABLE_BGSSoundDescriptorForm[0] };
				REL::Relocation<std::uintptr_t> TESSoundVtbl{ RE::VTABLE_TESSound[0] };
				//_Descriptor_SetFormEditorID = BGSSoundDescriptorFormVtbl.write_vfunc(0x33, Descriptor_SetFormEditorID);
				_Sound_SetFormEditorID = TESSoundVtbl.write_vfunc(0x33, Sound_SetFormEditorID);

				logger::debug("hook: SoundMarkersSetFormEditorID");
			}
		private:
			//static bool Descriptor_SetFormEditorID(RE::TESForm* a_form, const char* a_editorID)
			//{
			//	if (MCM::settings::modActive)
			//	{
			//		DebugMenu::GetMarkerHandler()->soundDescriptorEditorIDs[a_form->GetFormID()] = a_editorID;
			//	}
			//
			//	return _Descriptor_SetFormEditorID(a_form, a_editorID);
			//}

			static bool Sound_SetFormEditorID(RE::TESForm* a_form, const char* a_editorID)
			{
				if (MCM::settings::modActive)
				{
					DebugMenu::InfoHandler::soundEditorIDs[a_form->GetFormID()] = a_editorID;
				}

				return _Sound_SetFormEditorID(a_form, a_editorID);
			}
			//static inline REL::Relocation<decltype(Descriptor_SetFormEditorID)> _Descriptor_SetFormEditorID;
			static inline REL::Relocation<decltype(Sound_SetFormEditorID)> _Sound_SetFormEditorID;
			
	};
}