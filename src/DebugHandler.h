#pragma once

#include "DrawMenu.h"
#include "HUDHandler.h"

class DebugHandler
{
	public:
		HUDHandler* g_HUD = nullptr;

		static DebugHandler* GetSingleton()
		{
			static DebugHandler singleton;
			return std::addressof(singleton);
		}


		bool showNavmesh = false;
		bool showOcclusion = false;
		bool showCellBorders = false;
		float navmeshRange = 3000.0f;
		float occlusionRange = 1000.0f;
		uint32_t updateRate = 30;



		void init();
		void update();
		uint32_t updateCount = 0;

		void DrawAll();
		void ClearAll();
		void DrawNavmesh(float a_range);
		void DrawOcclusion(float a_range);
		void DrawTest();

	private:
		void DrawNavmesh(RE::TESObjectCELL* a_cell, RE::NiPoint3 a_origin, float a_range);

};