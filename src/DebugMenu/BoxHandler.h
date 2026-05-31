#pragma once

#include "DebugItem.h"

namespace DebugMenu
{
	class BoxHandler : public DebugItem
	{
		public:
			BoxHandler();

			void Draw() override;

		private:
			
			const static uint32_t planeMarkerID = 0x17;
			const static uint32_t collisionMarkerID = 0x21;

			float	GetRange() override;
			void    DrawBoxes();
			void	DrawOcclusion(RE::TESObjectREFR* a_ref, const RE::TESObjectCELL* a_cell);
			void	DrawCollisionBox(RE::TESObjectREFR* a_ref, const RE::TESObjectCELL* a_cell);

			bool	IsRefWithinRange(RE::TESObjectREFR* a_ref);
			void	DrawBox(const RE::NiPoint3& a_center, const RE::NiPoint3& a_halfExtents,
							const RE::NiMatrix3 a_rotation, uint32_t a_baseColor, uint32_t a_edgeColor,
							uint32_t a_baseAlpha, uint32_t a_edgeAlpha, MetaData& a_shapeMetaData);
	};
}