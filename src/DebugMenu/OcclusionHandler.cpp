#include "OcclusionHandler.h"
#include "DebugMenu.h"

namespace DebugMenu
{
	OcclusionHandler::OcclusionHandler()
	{
		logger::debug("Initialized OcclusionHandler");
	}

	void OcclusionHandler::Draw()
	{
		if (MCM::settings::showOcclusion) DrawOcclusion();
	}

	float OcclusionHandler::GetRange()
	{
		return MCM::settings::occlusionRange;
	}

	void OcclusionHandler::DrawOcclusion()
	{
		RE::NiPoint3 origin = GetCenter();
		float range = GetRange();
		Utils::ForEachCellInRange(origin, range, [&](const RE::TESObjectCELL* a_cell)
		{
			a_cell->ForEachReference([&](RE::TESObjectREFR* a_ref)
			{
				
				if (a_ref->GetBaseObject()->formID == 0x17) // planemarker
				{
					float dx = a_ref->GetPositionX() - origin.x;
					float dy = a_ref->GetPositionY() - origin.y;

					if (dx * dx + dy * dy > range*range) return RE::BSContainer::ForEachResult::kContinue;

					auto extra = &a_ref->extraList;
					for (const auto& data : a_ref->extraList)
					{
						if (data.GetType() == RE::ExtraDataType::kPrimitive)
						{
							uintptr_t dataAddr = reinterpret_cast<uintptr_t>(&data);
							uintptr_t primitiveAddr = *reinterpret_cast<uintptr_t*>(dataAddr + 0x10);
							float xBound = *reinterpret_cast<float*>(primitiveAddr + 0x0C); // the sizes are half the box length
							float yBound = *reinterpret_cast<float*>(primitiveAddr + 0x10);
							float zBound = *reinterpret_cast<float*>(primitiveAddr + 0x14);

						

							RE::NiAVObject* object = a_ref->Get3D();

							//if (a_ref->IsDisabled() || !AV) continue;

							RE::NiMatrix3 rotation;
							if (object) rotation = object->world.rotate;
							else rotation = RE::NiMatrix3{ a_ref->GetAngleX(), a_ref->GetAngleY(), a_ref->GetAngleZ() };

							uint32_t baseColor = a_ref->IsDisabled() ? MCM::settings::disabledOcclusionColor : MCM::settings::occlusionColor;

							RE::NiPoint3 center = a_ref->GetPosition();
							RE::NiPoint3 upperLeft1  = rotation * RE::NiPoint3(+xBound, +yBound, +zBound);
							RE::NiPoint3 upperRight1 = rotation * RE::NiPoint3(-xBound, +yBound, +zBound);
							RE::NiPoint3 lowerRight1 = rotation * RE::NiPoint3(-xBound, +yBound, -zBound);
							RE::NiPoint3 lowerLeft1  = rotation * RE::NiPoint3(+xBound, +yBound, -zBound);
							RE::NiPoint3 upperLeft2  = rotation * RE::NiPoint3(+xBound, -yBound, +zBound);
							RE::NiPoint3 upperRight2 = rotation * RE::NiPoint3(-xBound, -yBound, +zBound);
							RE::NiPoint3 lowerRight2 = rotation * RE::NiPoint3(-xBound, -yBound, -zBound);
							RE::NiPoint3 lowerLeft2  = rotation * RE::NiPoint3(+xBound, -yBound, -zBound);

							std::vector<RE::NiPoint3>  frontPlane{ center + upperLeft1,  center + upperRight1, center + lowerRight1, center + lowerLeft1 };
							std::vector<RE::NiPoint3>   backPlane{ center + upperLeft2,  center + upperRight2, center + lowerRight2, center + lowerLeft2 };
							std::vector<RE::NiPoint3>   leftPlane{ center + upperLeft1,  center + lowerLeft1,  center + lowerLeft2,  center + upperLeft2 };
							std::vector<RE::NiPoint3>  rightPlane{ center + upperRight1, center + upperRight2, center + lowerRight2, center + lowerRight1 };
							std::vector<RE::NiPoint3>    topPlane{ center + upperLeft1,  center + upperLeft2,  center + upperRight2, center + upperRight1 };
							std::vector<RE::NiPoint3> bottomPlane{ center + lowerLeft1,  center + lowerLeft2,  center + lowerRight2, center + lowerRight1 };

							auto metaData = CreateMetaData();
							metaData->occlusionBounds = RE::NiPoint3(2 * xBound, 2 * yBound, 2 * zBound);
							metaData->cell = a_cell;
							metaData->ref = a_ref;
							metaData->infoType = InfoType::kOcclusion;

							GetDrawHandler()->DrawPolygon(frontPlane, 0, baseColor, MCM::settings::occlusionAlpha, 0, 0, false, metaData);
							GetDrawHandler()->DrawPolygon(backPlane, 0, baseColor, MCM::settings::occlusionAlpha, 0, 0, false, metaData);
							GetDrawHandler()->DrawPolygon(leftPlane, 0, baseColor, MCM::settings::occlusionAlpha, 0, 0, false, metaData);
							GetDrawHandler()->DrawPolygon(rightPlane, 0, baseColor, MCM::settings::occlusionAlpha, 0, 0, false, metaData);
							GetDrawHandler()->DrawPolygon(topPlane, 0, baseColor, MCM::settings::occlusionAlpha, 0, 0, false, metaData);
							GetDrawHandler()->DrawPolygon(bottomPlane, 0, baseColor, MCM::settings::occlusionAlpha, 0, 0, false, metaData);

							// draw borders around the cube edges
							for (int i = 0; i < 4; i++)
							{
								int nextIndex = i == 3 ? 0 : i + 1;
								GetDrawHandler()->DrawLine(frontPlane[i], frontPlane[nextIndex], 5, MCM::settings::occlusionBorderColor, MCM::settings::occlusionBorderAlpha);
								GetDrawHandler()->DrawLine(backPlane[i], backPlane[nextIndex], 5, MCM::settings::occlusionBorderColor, MCM::settings::occlusionBorderAlpha);
								GetDrawHandler()->DrawLine(frontPlane[i], backPlane[i], 5, MCM::settings::occlusionBorderColor, MCM::settings::occlusionBorderAlpha);
							}
						}
					}
				}
				return RE::BSContainer::ForEachResult::kContinue;
			});
		});
	}
}