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
				logger::debug("hook: PlayerUpdate");
			
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
			logger::debug("hook: CellLoad");

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
			logger::debug("hook: NavMeshLoad");

		}
		private:
		static bool Load(RE::NavMesh* a_navmesh, RE::TESFile* a_mod)
		{
			bool result = _Load(a_navmesh, a_mod);

			if (MCM::settings::modActive && MCM::settings::showMoreNavmeshSourcefiles)
			{
				DebugHandler::GetSingleton()->OnNavMeshLoad(a_navmesh);
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
				REL::Relocation<std::uintptr_t> BGSSoundDescriptorFormVtbl{ RE::VTABLE_BGSSoundDescriptorForm[0] };
				REL::Relocation<std::uintptr_t> TESSoundVtbl{ RE::VTABLE_TESSound[0] };
				_Descriptor_SetFormEditorID = BGSSoundDescriptorFormVtbl.write_vfunc(0x33, Descriptor_SetFormEditorID);
				_Sound_SetFormEditorID = TESSoundVtbl.write_vfunc(0x33, Sound_SetFormEditorID);

				logger::debug("hook: SoundMarkersSetFormEditorID");
			}
		private:
				static bool Descriptor_SetFormEditorID(RE::TESForm* a_form, const char* a_editorID)
				{
					if (MCM::settings::modActive)
					{
						DebugHandler::GetSingleton()->soundDescriptorEditorIDs[a_form->GetFormID()] = a_editorID;
					}

					return _Descriptor_SetFormEditorID(a_form, a_editorID);
				}

				static bool Sound_SetFormEditorID(RE::TESForm* a_form, const char* a_editorID)
				{
					if (MCM::settings::modActive)
					{
						DebugHandler::GetSingleton()->soundEditorIDs[a_form->GetFormID()] = a_editorID;
					}

					return _Sound_SetFormEditorID(a_form, a_editorID);
				}
				static inline REL::Relocation<decltype(Descriptor_SetFormEditorID)> _Descriptor_SetFormEditorID;
				static inline REL::Relocation<decltype(Sound_SetFormEditorID)> _Sound_SetFormEditorID;
			
	};
}