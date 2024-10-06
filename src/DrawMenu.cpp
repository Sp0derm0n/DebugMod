#include "DrawMenu.h"
#include "Utils.h"

void DrawMenu::clearCanvas()
{
	if (movie)
	{ 
		movie->Invoke("clear", nullptr, nullptr, 0);

		/*
		if (const auto TES = RE::TES::GetSingleton(); TES)
		{
			//logger::info("TES obtained");
			TES->ForEachReferenceInRange(RE::PlayerCharacter::GetSingleton(), 1000.0f, [&](RE::TESObjectREFR* a_ref)
			{	
				auto base = a_ref->GetBaseObject();
				if (a_ref->As<RE::TESObjectREFR>() != RE::PlayerCharacter::GetSingleton() && (a_ref->Is(RE::FormType::NPC) || base && base->Is(RE::FormType::NPC)))
				{
					if (auto actor = a_ref->As<RE::Actor>(); actor)
					{
						if (!actor->IsDisabled() && !actor->IsDead() && actor->Get3D())
						{
							//logger::info("actor: {}", actor->GetDisplayFullName());
							auto [screenPosition1, scale1] = worldPositionToScreenPosition(actor->GetPosition());
							//g_DrawMenu->DrawPoint(screenPosition1, 15*scale1, 0xFFFFFF, 30);
							//DrawPoint(PointData(actor->GetPosition(), 15*scale1, 0xFFFFFF, 30));
							DrawPoint(screenPosition1, 20*scale1, 0xFFFFFF, 90);
						}	
					}
				}
				return RE::BSContainer::ForEachResult::kContinue;
			});
		}*/
	}
}


void DrawMenu::DrawPoint(RE::NiPoint2 a_position, float a_radius, uint32_t a_color, uint32_t a_alpha)
{
	if (!movie) return;

	// The angle of each of the eight segments is 45 degrees (360 divided by 8), which
	// equals π/4 radians.
	constexpr float angleDelta = 3.14159265359 / 4;

	// Find the distance from the circle's center to the control points for the curves.
	float ctrlDist = a_radius / cosf(angleDelta / 2.f);

	// Initialize the angle
	float angle = 0.f;

	RE::GFxValue argsLineStyle[3]{ 0, 0, 0 };
	movie->Invoke("lineStyle", nullptr, argsLineStyle, 3);

	RE::GFxValue argsFill[2]{ a_color, a_alpha };
	movie->Invoke("beginFill", nullptr, argsFill, 2);


	// Move to the starting point, one radius to the right of the circle's center.
	RE::GFxValue argsStartPos[2]{a_position.x + a_radius, a_position.y };
	movie->Invoke("moveTo", nullptr, argsStartPos, 2);

	// Repeat eight times to create eight segments.
	for (int i = 0; i < 8; ++i) {
		// Increment the angle by angleDelta (π/4) to create the whole circle (2π).
		angle += angleDelta;

		// The control points are derived using sine and cosine.
		float rx = a_position.x + cosf(angle - (angleDelta / 2)) * (ctrlDist);
		float ry = a_position.y + sinf(angle - (angleDelta / 2)) * (ctrlDist);

		// The anchor points (end points of the curve) can be found similarly to the
		// control points.
		float ax = a_position.x + cosf(angle) * a_radius;
		float ay = a_position.y + sinf(angle) * a_radius;

		// Draw the segment.
		RE::GFxValue argsCurveTo[4]{ rx, ry, ax, ay };
		movie->Invoke("curveTo", nullptr, argsCurveTo, 4);
	}

	movie->Invoke("endFill", nullptr, nullptr, 0);
}

void DrawMenu::DrawLine(RE::NiPoint2 a_start, RE::NiPoint2 a_end, float a_startRadius, float a_endRadius, uint32_t a_color, uint32_t a_alpha)
{
	if (!movie) return;

	/*
	RE::GFxValue argsLineStyle[3]{a_thickness, a_color, a_alpha};
	movie->Invoke("lineStyle", nullptr, argsLineStyle, 3);

	RE::GFxValue argsStartPos[2]{ a_start.x, a_start.y };
	movie->Invoke("moveTo", nullptr, argsStartPos, 2);

	RE::GFxValue argsEndPos[2]{ a_end.x, a_end.y };
	movie->Invoke("lineTo", nullptr, argsEndPos, 2);

	movie->Invoke("endFill", nullptr, nullptr, 0);
	*/

	constexpr float theta = 3.14159265359 / 4;

	
	//float ctrlDist = a_endRadius / cosf(angleDelta / 2.f);



	RE::NiPoint2 endDirection{a_end.y-a_start.y, a_start.x-a_end.x}; // rotates a vector 90 deg clockwise is achived by newx = oldy, newy = -oldx
	RE::NiPoint2 startDirection = endDirection/endDirection.Length() * a_startRadius;
	endDirection *=  a_endRadius/endDirection.Length();


	RE::GFxValue argsLineStyle[3]{ 0, 0, 0 };
	movie->Invoke("lineStyle", nullptr, argsLineStyle, 3);

	RE::GFxValue argsFill[2]{ a_color, a_alpha };
	movie->Invoke("beginFill", nullptr, argsFill, 2);

	float ax = a_end.x + endDirection.x;
	float ay = a_end.y + endDirection.y;

	float cs = cosf(theta);
	float sn = sinf(theta);

	float cs2 = cosf(theta/2);
	float sn2 = sinf(theta/2);

	// Move to the starting point, one radius to the right of the circle's center.
	RE::GFxValue argsStartPos[2]{ ax, ay };

	movie->Invoke("moveTo", nullptr, argsStartPos, 2);
	for (int i = 0; i < 4; ++i) {
		float rx = a_end.x + (endDirection.x*cs2 - endDirection.y*sn2)/cs2;
		float ry = a_end.y + (endDirection.x*sn2 + endDirection.y*cs2)/cs2;

		ax = endDirection.x*cs - endDirection.y*sn;
		ay = endDirection.x*sn + endDirection.y*cs;

		endDirection.x = ax;
		endDirection.y = ay;

		ax += a_end.x;
		ay += a_end.y;


		RE::GFxValue argsCurveTo[4]{ rx, ry, ax, ay };
		movie->Invoke("curveTo", nullptr, argsCurveTo, 4);
	}

	endDirection.x *= a_startRadius/a_endRadius;
	endDirection.y *= a_startRadius/a_endRadius;

	ax = a_start.x + endDirection.x;
	ay = a_start.y + endDirection.y;

	RE::GFxValue argsNextPos[2]{ ax, ay };
	movie->Invoke("lineTo", nullptr, argsNextPos, 2);
	
	for (int i = 0; i < 4; ++i) {
		float rx = a_start.x + (endDirection.x*cs2 - endDirection.y*sn2)/cs2;
		float ry = a_start.y + (endDirection.x*sn2 + endDirection.y*cs2)/cs2;

		ax = endDirection.x*cs - endDirection.y*sn;
		ay = endDirection.x*sn + endDirection.y*cs;

		endDirection.x = ax;
		endDirection.y = ay;

		ax += a_start.x;
		ay += a_start.y;

		RE::GFxValue argsCurveTo[4]{ rx, ry, ax, ay };
		movie->Invoke("curveTo", nullptr, argsCurveTo, 4);
	}

	movie->Invoke("endFill", nullptr, nullptr, 0);
}


void DrawMenu::DrawSquare(RE::NiPoint2 a_leftLowerCorner, RE::NiPoint2 a_leftUpperCorner, RE::NiPoint2 a_rightUpperCorner, RE::NiPoint2 a_rightLowerCorner, uint32_t a_color, uint32_t a_alpha, uint32_t a_borderAlpha)
{
	if (!movie) return;

	RE::GFxValue argsLineStyle[3]{ 2, a_color, a_borderAlpha};
	movie->Invoke("lineStyle", nullptr, argsLineStyle, 3);

	RE::GFxValue argsFill[2]{ a_color, a_alpha };
	movie->Invoke("beginFill", nullptr, argsFill, 2);


	RE::GFxValue argsStartPos[2]{ a_leftLowerCorner.x, a_leftLowerCorner.y };
	movie->Invoke("moveTo", nullptr, argsStartPos, 2);

	RE::GFxValue argsNextPos1[2]{ a_leftUpperCorner.x, a_leftUpperCorner.y };
	movie->Invoke("lineTo", nullptr, argsNextPos1, 2);

	RE::GFxValue argsNextPos2[2]{ a_rightUpperCorner.x, a_rightUpperCorner.y };
	movie->Invoke("lineTo", nullptr, argsNextPos2, 2);

	RE::GFxValue argsNextPos3[2]{ a_rightLowerCorner.x, a_rightLowerCorner.y };
	movie->Invoke("lineTo", nullptr, argsNextPos3, 2);




	movie->Invoke("endFill", nullptr, nullptr, 0);
}
