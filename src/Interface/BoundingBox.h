#pragma once

namespace ScaleformUI
{
	class BBox;
	using BBoxPTR = std::unique_ptr<BBox>;

	class BBox
	{
		public:
			struct AABB
			{
				float xmin = 0.0f;
				float ymin = 0.0f;
				float xmax = 0.0f;
				float ymax = 0.0f;
			};
			virtual ~BBox() = default;
		
			virtual bool IsPointInBBox(float a_x, float a_y) = 0;
			virtual void UpdateBounds(float a_xmin, float a_ymin, float a_xmax, float y_ymax, float a_rotation = 0.0f) = 0;
			AABB GetAABB() { return aabb; }

		protected:
			AABB aabb;
	};

	class SquareBBox : public BBox
	{
		public:
			SquareBBox() {}
			SquareBBox(float a_xmin, float a_ymin, float a_xmax, float a_ymax) : xmin(a_xmin), ymin(a_ymin), xmax(a_xmax), ymax(a_ymax) {}
			bool IsPointInBBox(float a_x, float a_y) override;
			void UpdateBounds(float a_xmin, float a_ymin, float a_xmax, float y_ymax, float a_rotation = 0.0f) override;
		
		private:
			float xmin = 0.0f;
			float ymin = 0.0f;
			float xmax = 0.0f;
			float ymax = 0.0f;
			float rotation = 0.0f;
	};

	class CircleBBox : public BBox
	{
		public:

			bool IsPointInBBox(float a_x, float a_y) override { return 0.0f; }

		private:
			float centerX = 0.0f;
			float centerY = 0.0f;
			float radius = 0.0f;
	};

	class CylinderBBox : public BBox
	{
		public:
			// shortest distance between linesegment and point? try differentiating to find analytical expression
			bool IsPointInBBox(float a_x, float a_y) override { return 0.0f; }
		private:
			float xA = 0.0f;
			float yA = 0.0f;
			float xB = 0.0f;
			float yB = 0.0f;
			float radius = 0.0f;
			float rotation = 0.0f;
	};

	class OvalBBox : public BBox
	{
		public:
			bool IsPointInBBox(float a_x, float a_y) override { return 0.0f; }
		private:
			float centerX = 0.0f;
			float centerY = 0.0f;
			float radiusX = 0.0f;
			float radiusY = 0.0f;
			float rotation = 0.0f;

	};
}