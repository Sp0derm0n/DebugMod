#pragma once

#include "Linalg.h"
#include "Linalg.h"
#include "DrawMenu.h"

class DrawHandler
{
	public:

		RE::GPtr<DrawMenu> g_DrawMenu = nullptr;

		static DrawHandler* GetSingleton()
		{
			static DrawHandler singleton;
			return std::addressof(singleton);
		}

		void GetDrawMenu();

		struct ScreenspacePoint
		{
			RE::NiPoint2 point;
			float scale;
		};

		struct ScreenspacePolygon
		{
			std::vector<RE::NiPoint2> points;
			std::vector<float> scales;
			float avgScale;
		};

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
			bool isSimpleLine;
		};

		struct PolygonData
		{
			std::vector<RE::NiPoint3> positions;
			float borderThickness;
			uint32_t color;
			uint32_t baseAlpha;
			uint32_t borderColor;
			uint32_t borderAlpha;
			std::string info;
		};

		bool isMenuOpen;

		bool isPolygonHighlighted; // controls wheter a polygon is being looked at (reset every fram)
		bool isInfoBoxVisible;

		float alphaMultiplier = 1;
		float bufferScale = 0.0f; // If some objects are blinking, and this is > 0, it might be the cause. Has not been properly implemented yet. extension of canvas that still allows object to be drawn
		float infoRadius = 16.0f; // distance from the canvas center that a point can be and still be selected to show info
		float infoDelay = 0.25f; // time before the infobox opens
		float timeHovering = 0.0f;

		std::vector<RE::NiPoint3> highlightPolygon; // controls what polygon the the 

		float canvasWidth;
		float canvasHeight;
		float left;
		float right;
		float top;
		float bottom;

		uint32_t linesScrolledPerClick = 8;
		int32_t infoBoxScroll = 1;

		RE::NiPoint3 cameraFacingDirection;
		Linalg::Matrix4 projectionMatrix;

		RE::NiPointer<RE::NiCamera> niCamera;
		RE::PlayerCamera* playerCamera;

		std::vector<std::unique_ptr<PointData>> pointsToDraw;
		std::vector<std::unique_ptr<LineData>> linesToDraw;
		std::vector<std::unique_ptr<PolygonData>> polygonsToDraw;

		void Init();
		void Update(float a_delta);
		void ClearAll();
		void UpdateCanvasScale();

		void DrawPoint(RE::NiPoint3 a_position, float a_scale, uint32_t a_color = 0xFFFFFF, uint32_t a_alpha = 100);
		void DrawLine(RE::NiPoint3 a_start, RE::NiPoint3 a_end, float a_thickness, uint32_t a_color = 0xFFFFFF, uint32_t a_alpha = 100, bool a_isSimpleLine = true);
		void DrawPolygon(std::vector<RE::NiPoint3> a_positions, float a_borderThickness = 2, uint32_t a_color = 0xFFFFFF, uint32_t a_baseAlpha = 50, uint32_t a_borderAlpha = 0, std::string a_info = "", uint32_t a_borderColor = 0xFFFFFF, bool a_useCustomBorderColor = false);

		void InfoBoxScrollUp();
		void InfoBoxScrollDown();

	private:
		void DrawPoints();
		void DrawLines();
		void DrawPolygons(float a_delta);
		void DrawCrosshair();
		void DrawCanvasBorders();
		void BuildProjectionMatrix();

		RE::NiPointer<RE::NiCamera> GetNiCamera(RE::PlayerCamera* a_camera);

		Linalg::Vector4 worldToClipPoint(const RE::NiPoint3& a_position);

		bool isPointOnScreen(const Linalg::Vector4& a_clipPoint);
		bool ClipLine(Linalg::Vector4& a_point1, Linalg::Vector4& a_point2);
		bool ClipPolygon(std::vector<Linalg::Vector4>& a_points);



		ScreenspacePoint PointToScreenspace(const Linalg::Vector4& a_point);
		ScreenspacePolygon PolygonToScreenspace(const std::vector<Linalg::Vector4>& a_points);


};