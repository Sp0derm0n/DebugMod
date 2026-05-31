#pragma once

#include "DebugMenu/DebugMenu.h"
#include "MCM.h"
#include "FreeCamHandler.h"

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
	
	// move speed
	//RE::NiPoint3 prePos;
	//float* deltaTime = (float*)RELOCATION_ID(523660, 410199).address();
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

				// Move speed
				//if (auto playerCam = RE::PlayerCamera::GetSingleton())
				//{
				//	if (auto freeCamState = playerCam->GetRuntimeData().cameraStates[RE::CameraState::kFree])
				//	{
				//		if (auto freeCam = FreeCamHandler::GetSingleton()->GetFreeCamera())
				//		{
				//			auto pos = freeCam->translation;
				//			auto delta = pos - prePos;
				//			auto speed = delta / 70 / *deltaTime;
				//			prePos = pos;
				//			logger::debug("free cam speed: {}", speed.Length());
				//		}
				//	}
				//}

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
			static bool Load(RE::TESObjectCELL* a_cell, RE::TESFile* a_mod)
			{
				bool result = _Load(a_cell, a_mod);

				if (MCM::settings::modActive)
				{
					DebugMenu::GetNavmeshHandler()->OnCellLoad(a_cell);
				}
				return result;
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

	class Hook_LandLoad
	{
		public:
		static void install()
		{
			REL::Relocation<std::uintptr_t> LandVtbl{ RE::VTABLE_TESObjectLAND[0] };
			_Load = LandVtbl.write_vfunc(0x6, Load);
			logger::debug("hook: LandLoad");

		}
		private:
		static bool Load(RE::TESObjectLAND* a_land, RE::TESFile* a_mod)
		{
			bool result = _Load(a_land, a_mod);

			if (MCM::settings::modActive)
			{
				DebugMenu::GetCellHandler()->OnLandLoad(a_land, a_mod);
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
				REL::Relocation<std::uintptr_t> TESSoundVtbl{ RE::VTABLE_TESSound[0] };
				_Sound_SetFormEditorID = TESSoundVtbl.write_vfunc(0x33, Sound_SetFormEditorID);

				logger::debug("hook: SoundMarkersSetFormEditorID");
			}
		private:

			static bool Sound_SetFormEditorID(RE::TESForm* a_form, const char* a_editorID)
			{
				if (MCM::settings::modActive)
				{
					DebugMenu::InfoHandler::soundEditorIDs[a_form->GetFormID()] = a_editorID;
				}

				return _Sound_SetFormEditorID(a_form, a_editorID);
			}
			static inline REL::Relocation<decltype(Sound_SetFormEditorID)> _Sound_SetFormEditorID;
	};

	class Hook_FreeCameraState_UpdatePosition_1408E0640
	{
		public:
			static void Install()
			{
				logger::debug("Hook: FreeCameraState UpdatePosition_140437D40");

				auto& trampoline = SKSE::GetTrampoline();
				REL::Relocation<uintptr_t> hook{ RELOCATION_ID(49813, 50743), REL::VariantOffset(0x12, 0x12, 0x12) };
				_UpdatePosition = trampoline.write_call<5>(hook.address(), UpdatePosition);
			}

		private:
			static void UpdatePosition(RE::FreeCameraState* a_freeCameraState)
			{
				if (MCM::settings::modActive) 
				{
					FreeCamHandler::GetSingleton()->Update();
				}

				_UpdatePosition(a_freeCameraState);
				
				if (MCM::settings::modActive)
				{
					FreeCamHandler::GetSingleton()->PostUpdate();
				}

			}
			static inline REL::Relocation<decltype(UpdatePosition)> _UpdatePosition;
	};

	class Hook_FreeCameraBegin
	{
		public:
		static void install()
		{
			REL::Relocation<std::uintptr_t> vtbl{ RE::VTABLE_FreeCameraState[0] };
			_Begin = vtbl.write_vfunc(0x1, Begin);
			logger::debug("hook: CameraBegin");

		}
		private:
		static void Begin(RE::FreeCameraState* a_camera)
		{
			_Begin(a_camera);
			if (MCM::settings::modActive) FreeCamHandler::GetSingleton()->OnEnterFreeCam();


		}
		static inline REL::Relocation<decltype(Begin)> _Begin;
	};

	class Hook_FreeCameraEnd
	{
		public:
		static void install()
		{
			REL::Relocation<std::uintptr_t> vtbl{ RE::VTABLE_FreeCameraState[0] };
			_End = vtbl.write_vfunc(0x2, End);
			logger::debug("hook: CameraEnd");

		}
		private:
		static void End(RE::FreeCameraState* a_camera)
		{
			if (MCM::settings::modActive) FreeCamHandler::GetSingleton()->OnExitFreeCam();
			_End(a_camera);

		}
		static inline REL::Relocation<decltype(End)> _End;
	};
}