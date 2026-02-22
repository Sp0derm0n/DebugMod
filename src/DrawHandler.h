#pragma once

#include "Linalg.h"
#include "DrawMenu.h"

class DrawHandler
{
	public:
		RE::GPtr<DrawMenu> g_DrawMenu = nullptr;
	
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

		struct ShapeMetaData
		{
			enum class InfoType
			{
				kNoInfo,
				kQuad,
				kNavmesh,
				kNavmeshCover,
				kOcclusion,
				kRef,
				kLightMarker,
				kSoundMarker
			};
			InfoType infoType = InfoType::kNoInfo;
			RE::FormID formID = 0x0;
			const RE::TESObjectCELL* cell = nullptr;
			const RE::TESObjectREFR* ref = nullptr;
			RE::NiPoint3 occlusionBounds{ 0.0f, 0.0f, 0.0f };
			int8_t quad = -1;
			uint8_t coverEdge = 0;
			uint16_t navmeshTraversalFlags = 0;


		};

		using MetaDataPtr = std::shared_ptr<ShapeMetaData>;

		struct ShapeData
		{
			MetaDataPtr metaData = nullptr;
			ShapeData(MetaDataPtr a_metaData) : metaData(a_metaData) {}
			ShapeData() {}
		};

		struct PointData : ShapeData
		{
			RE::NiPoint3 position;
			float radius;
			uint32_t color;
			uint32_t alpha;
			PointData(RE::NiPoint3 a_pos, float a_radius, uint32_t a_color, uint32_t a_alpha, MetaDataPtr a_metaData) :
				ShapeData(a_metaData), position(a_pos), radius(a_radius), color(a_color), alpha(a_alpha)
			{}
		};

		struct LineData : ShapeData
		{
			RE::NiPoint3 start;
			RE::NiPoint3 end;
			float thickness;
			uint32_t color;
			uint32_t alpha;
			bool isSimpleLine;
			LineData(RE::NiPoint3 a_start, RE::NiPoint3 a_end, float a_thickness, uint32_t a_color, uint32_t a_alpha, bool a_isSimpleLine, MetaDataPtr a_metaData) :
				ShapeData(a_metaData),
				start(a_start), 
				end(a_end), 
				thickness(a_thickness), 
				color(a_color), 
				alpha(a_alpha), 
				isSimpleLine(a_isSimpleLine)
			{}
		};

		struct PolygonData : ShapeData
		{
			std::vector<RE::NiPoint3> positions;
			float borderThickness;
			uint32_t color;
			uint32_t baseAlpha;
			uint32_t borderColor;
			uint32_t borderAlpha;
			PolygonData(std::vector<RE::NiPoint3> a_positions, float a_thickness, uint32_t a_color, uint32_t a_baseAlpha, uint32_t a_borderColor, uint32_t a_borderAlpha, MetaDataPtr a_metaData) :
				ShapeData(a_metaData),
				positions(a_positions), 
				borderThickness(a_thickness), 
				color(a_color), baseAlpha(a_baseAlpha), 
				borderColor(a_borderColor), 
				borderAlpha(a_borderAlpha)
			{}
		};

		bool isMenuOpen = false;

		

		float alphaMultiplier = 1;
		float pointScaleMultiplier = 200; // Chosen arbitrarily
		float bufferScale = 0.0f; // If some objects are blinking, and this is > 0, it might be the cause. Has not been properly implemented yet. extension of canvas that still allows object to be drawn
		

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

		DrawHandler();
		void Init();
		void GetDrawMenu();
		void Update(float a_delta);
		void ClearScaleform();
		void ClearD3D11();
		void UpdateCanvasScale();
		void UpdateProjectionMatrix();
		Linalg::Matrix4& GetProjectionMatrix();

		void DrawPoint(RE::NiPoint3 a_position, float a_scale, uint32_t a_color = 0xFFFFFF, uint32_t a_alpha = 100, MetaDataPtr a_metaData = nullptr);
		void DrawLine(RE::NiPoint3 a_start, RE::NiPoint3 a_end, float a_thickness, uint32_t a_color = 0xFFFFFF, uint32_t a_alpha = 100, bool a_isSimpleLine = true, MetaDataPtr a_metaData = nullptr);
		void DrawPolygon(std::vector<RE::NiPoint3> a_positions, float a_borderThickness = 2, uint32_t a_color = 0xFFFFFF, uint32_t a_baseAlpha = 50, uint32_t a_borderAlpha = 0, uint32_t a_borderColor = 0xFFFFFF, bool a_useCustomBorderColor = false, MetaDataPtr a_metaData = nullptr);

		void InfoBoxScrollUp();
		void InfoBoxScrollDown();
		
	private:
		struct ShowInfoData
		{
			float depth = 0.0f;
			RE::NiPoint2 screenPoint{ 0.0f, 0.0f };
			MetaDataPtr shapeMetaData = nullptr;

			const bool operator==(ShowInfoData& a_other) const { return shapeMetaData == a_other.shapeMetaData; }
			const bool operator!=(ShowInfoData& a_other) const { return shapeMetaData != a_other.shapeMetaData; }
		};

		std::vector<ShowInfoData>	eligibleInfoPoints;
		bool						isInfoBoxVisible = false;
		const float					infoRadius = 16.0f; // distance from the canvas center that a point can be and still be selected to show info
		const float					infoDelay = 0.25f; // time before the infobox opens
		float						timeHovering = 0.0f; // in seconds
		ShowInfoData				currentInfoData;


		bool						IsShapeEligibleForInfo(const ShapeData* a_shape, const RE::NiPoint2& a_pointInShape, float a_depth);
		void						AddEligibleInfoPoint(float a_pointDepth, const RE::NiPoint2& a_screenPoint, MetaDataPtr& metaData);
		void						HandleInfo(float a_delta);


		RE::NiPointer<RE::NiCamera>		GetNiCamera(RE::PlayerCamera* a_camera);
		void							DrawPoints();
		void							DrawLines();
		void							DrawPolygons();
		void							DrawCrosshair();
		void							DrawCanvasBorders();
		void							BuildProjectionMatrix();
		Linalg::Vector4					worldToClipPoint(const RE::NiPoint3& a_position);
		bool							isPointOnScreen(const Linalg::Vector4& a_clipPoint);
		bool							ClipLine(Linalg::Vector4& a_point1, Linalg::Vector4& a_point2);
		bool							ClipPolygon(std::vector<Linalg::Vector4>& a_points);

		ScreenspacePoint				PointToScreenspace(const Linalg::Vector4& a_point);
		ScreenspacePolygon				PolygonToScreenspace(const std::vector<Linalg::Vector4>& a_points);


};