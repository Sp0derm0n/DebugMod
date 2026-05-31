#include "BoxHandler.h"
#include "DebugMenu.h"

namespace DebugMenu
{
	BoxHandler::BoxHandler()
	{
		logger::debug("Initialized BoxHandler");
	}

	void BoxHandler::Draw()
	{
		if (MCM::settings::showBoxes) DrawBoxes();
	}

	float BoxHandler::GetRange()
	{
		return MCM::settings::boxesRange;
	}

	void BoxHandler::DrawBoxes()
	{
		RE::NiPoint3 origin = GetCenter();
		float range = GetRange();

		Utils::ForEachCellInRange(origin, range, [&](const RE::TESObjectCELL* a_cell)
		{
			a_cell->ForEachReference([&](RE::TESObjectREFR* a_ref)
			{

				switch (a_ref->GetBaseObject()->formID)
				{
					case planeMarkerID:
					{
						if (MCM::settings::showOcclusion && IsRefWithinRange(a_ref))
						{
							DrawOcclusion(a_ref, a_cell);
						}
						break;
					}
					case collisionMarkerID:
					{
						if (MCM::settings::showCollisionMarkers && IsRefWithinRange(a_ref))
						{
							DrawCollisionBox(a_ref, a_cell);
						}
						break;
					}
				}
				return RE::BSContainer::ForEachResult::kContinue;
			});
		});
	}

	void BoxHandler::DrawOcclusion(RE::TESObjectREFR* a_ref, const RE::TESObjectCELL* a_cell)
	{
		auto extra = &a_ref->extraList;
		for (auto& data : a_ref->extraList)
		{
			if (data.GetType() == RE::ExtraDataType::kPrimitive)
			{
				auto extraPrimitive = static_cast<RE::ExtraPrimitive*>(&data);
				auto primitive = extraPrimitive->primitive;

				RE::NiAVObject* object = a_ref->Get3D();

				RE::NiMatrix3 rotation;
				if (object) rotation = object->world.rotate;
				else rotation = RE::NiMatrix3{ a_ref->GetAngleX(), a_ref->GetAngleY(), a_ref->GetAngleZ() };

				uint32_t baseColor = a_ref->IsDisabled() ? MCM::settings::disabledOcclusionColor : MCM::settings::occlusionColor;

				RE::NiPoint3 center = a_ref->GetPosition();

				MetaData metaData;
				metaData.bounds = 2*primitive->halfExtents;
				metaData.cell = a_cell;
				metaData.ref = a_ref;
				metaData.infoType = InfoType::kOcclusion;

				DrawBox(center, primitive->halfExtents, rotation, baseColor,
						MCM::settings::occlusionBorderColor, MCM::settings::occlusionAlpha,
						MCM::settings::occlusionBorderAlpha, metaData);
			}
		}
	}

	void BoxHandler::DrawCollisionBox(RE::TESObjectREFR* a_ref, const RE::TESObjectCELL* a_cell)
	{
		RE::COL_LAYER collisionLayer = RE::COL_LAYER::kUnidentified;

		auto extra = &a_ref->extraList;
		for (auto& data : a_ref->extraList)
		{
			if (data.GetType() == RE::ExtraDataType::kCollisionData)
			{
				auto extraCollisionData = static_cast<RE::ExtraCollisionData*>(&data);
				if (extraCollisionData && extraCollisionData->collisionData)
					collisionLayer = extraCollisionData->collisionData->layer;
			}
		}
		for (auto& data : a_ref->extraList)
		{
			if (data.GetType() == RE::ExtraDataType::kPrimitive)
			{
				auto extraPrimitive = static_cast<RE::ExtraPrimitive*>(&data);
				auto primitive = extraPrimitive->primitive;

				RE::NiAVObject* object = a_ref->Get3D();

				RE::NiMatrix3 rotation;
				if (object) rotation = object->world.rotate;
				else rotation = RE::NiMatrix3{ a_ref->GetAngleX(), a_ref->GetAngleY(), a_ref->GetAngleZ() };


				RE::NiPoint3 center = a_ref->GetPosition();

				MetaData metaData;
				metaData.bounds = 2 * primitive->halfExtents;
				metaData.cell = a_cell;
				metaData.ref = a_ref;
				metaData.formID = a_ref->formID;
				metaData.colliisonLayer = collisionLayer;
				metaData.infoType = InfoType::kCollisionMarker;

				DrawBox(center, primitive->halfExtents, rotation, MCM::settings::collisionMarkerColor,
						MCM::settings::collisionMarkerBorderColor, MCM::settings::collisionMarkerAlpha,
						MCM::settings::collisionMarkerBorderAlpha, metaData);
			}
		}

	}

	bool BoxHandler::IsRefWithinRange(RE::TESObjectREFR* a_ref)
	{
		RE::NiPoint3 origin = GetCenter();

		float dx = a_ref->GetPositionX() - origin.x;
		float dy = a_ref->GetPositionY() - origin.y;

		float range = GetRange();

		return dx * dx + dy * dy < range * range;
	}

	void BoxHandler::DrawBox(const RE::NiPoint3& a_center, const RE::NiPoint3& a_halfExtents,
							 const RE::NiMatrix3 a_rotation, uint32_t a_baseColor, uint32_t a_edgeColor,
							 uint32_t a_baseAlpha, uint32_t a_edgeAlpha, MetaData& a_shapeMetaData)
	{
		float xBound = a_halfExtents[0];
		float yBound = a_halfExtents[1];
		float zBound = a_halfExtents[2];

		RE::NiPoint3 upperLeft1  = a_rotation * RE::NiPoint3(+xBound, +yBound, +zBound);
		RE::NiPoint3 upperRight1 = a_rotation * RE::NiPoint3(-xBound, +yBound, +zBound);
		RE::NiPoint3 lowerRight1 = a_rotation * RE::NiPoint3(-xBound, +yBound, -zBound);
		RE::NiPoint3 lowerLeft1  = a_rotation * RE::NiPoint3(+xBound, +yBound, -zBound);
		RE::NiPoint3 upperLeft2  = a_rotation * RE::NiPoint3(+xBound, -yBound, +zBound);
		RE::NiPoint3 upperRight2 = a_rotation * RE::NiPoint3(-xBound, -yBound, +zBound);
		RE::NiPoint3 lowerRight2 = a_rotation * RE::NiPoint3(-xBound, -yBound, -zBound);
		RE::NiPoint3 lowerLeft2  = a_rotation * RE::NiPoint3(+xBound, -yBound, -zBound);

		std::vector<RE::NiPoint3>  frontPlane{ a_center + upperLeft1,  a_center + upperRight1, a_center + lowerRight1, a_center + lowerLeft1 };
		std::vector<RE::NiPoint3>   backPlane{ a_center + upperLeft2,  a_center + upperRight2, a_center + lowerRight2, a_center + lowerLeft2 };
		std::vector<RE::NiPoint3>   leftPlane{ a_center + upperLeft1,  a_center + lowerLeft1,  a_center + lowerLeft2,  a_center + upperLeft2 };
		std::vector<RE::NiPoint3>  rightPlane{ a_center + upperRight1, a_center + upperRight2, a_center + lowerRight2, a_center + lowerRight1 };
		std::vector<RE::NiPoint3>    topPlane{ a_center + upperLeft1,  a_center + upperLeft2,  a_center + upperRight2, a_center + upperRight1 };
		std::vector<RE::NiPoint3> bottomPlane{ a_center + lowerLeft1,  a_center + lowerLeft2,  a_center + lowerRight2, a_center + lowerRight1 };

		GetDrawHandler()->DrawPolygon(frontPlane,	0, a_baseColor, a_baseAlpha, 0, 0, false, a_shapeMetaData);
		GetDrawHandler()->DrawPolygon(backPlane,	0, a_baseColor, a_baseAlpha, 0, 0, false, a_shapeMetaData);
		GetDrawHandler()->DrawPolygon(leftPlane,	0, a_baseColor, a_baseAlpha, 0, 0, false, a_shapeMetaData);
		GetDrawHandler()->DrawPolygon(rightPlane,	0, a_baseColor, a_baseAlpha, 0, 0, false, a_shapeMetaData);
		GetDrawHandler()->DrawPolygon(topPlane,		0, a_baseColor, a_baseAlpha, 0, 0, false, a_shapeMetaData);
		GetDrawHandler()->DrawPolygon(bottomPlane,	0, a_baseColor, a_baseAlpha, 0, 0, false, a_shapeMetaData);

		// draw borders along the box edges
		for (int i = 0; i < 4; i++)
		{
			int nextIndex = i == 3 ? 0 : i + 1;
			GetDrawHandler()->DrawLine(frontPlane[i], frontPlane[nextIndex], 5, a_edgeColor, a_edgeAlpha);
			GetDrawHandler()->DrawLine(backPlane[i], backPlane[nextIndex],	 5, a_edgeColor, a_edgeAlpha);
			GetDrawHandler()->DrawLine(frontPlane[i], backPlane[i],			 5, a_edgeColor, a_edgeAlpha);
		}
	}
}
