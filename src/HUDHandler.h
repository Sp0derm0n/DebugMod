# pragma once

#include "DrawMenu.h"
#include "Utils.h"

class HUDHandler
{
	public:
		static HUDHandler* GetSingleton()
		{
			static HUDHandler singleton;
			return std::addressof(singleton);
		}

		struct PointData
		{
			RE::NiPoint3 position;
			float radius;
			uint32_t color;
			uint32_t alpha;
		};

		struct LineData
		{
			RE::NiPoint3 start;
			RE::NiPoint3 end;
			float thickness;
			uint32_t color;
			uint32_t alpha;
		};
		
		RE::GPtr<DrawMenu> g_DrawMenu = nullptr;

		static RE::GPtr<DrawMenu> GetDrawMenuHud();

		std::vector<std::unique_ptr<PointData>> pointsToDraw;
		std::vector<std::unique_ptr<LineData>> linesToDraw;


		RE::NiPoint3 cameraPosition = RE::NiPoint3(1,1,1);
		RE::NiPoint3 cameraDirectionUnitVector;
		RE::NiPoint3 cameraHeightUnitVector;
		RE::NiPoint3 cameraWidthUnitVector;
		float cameraFOV;

		float canvasWidth;
		float canvasHeight;

		void init();
		void updateHUD();
		void updateCameraPosition();
		void updatePoints();

		Utils::ScreenPositionData worldPositionToScreenPosition(RE::NiPoint3 a_position);

		Utils::ScreenPositionData DrawLineWithOnePointBehindCamera(Utils::ScreenPositionData a_behindData, Utils::ScreenPositionData a_inFrontData, RE::NiPoint3 a_behindPoint, RE::NiPoint3 a_inFrontPoint);


		bool isOnScreen(RE::NiPoint2 a_point, float a_radius); // for points
		bool isOnScreen(RE::NiPoint2 a_start, RE::NiPoint2 a_end, float a_radius); // for lines

		void DrawPoint(RE::NiPoint3 a_position, float a_scale, uint32_t a_color = 0xFFFFFF, uint32_t a_alpha = 100);
		void DrawLine(RE::NiPoint3 a_start, RE::NiPoint3 a_end, float a_thickness, uint32_t a_color = 0xFFFFFF, uint32_t a_alpha = 100);
		

		


};