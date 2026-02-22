#include "DrawMenu.h"
#include "Linalg.h"
#include "DebugMenu/DebugMenu.h"
#include "DrawHandler.h"

void DrawMenu::Register()
{
	auto ui = RE::UI::GetSingleton();
	if (ui) 
	{
		ui->Register(MENU_NAME, Creator);
		logger::debug("Registered menu '{}'", MENU_NAME);
	}
}

RE::UI_MESSAGE_RESULTS DrawMenu::ProcessMessage(RE::UIMessage& a_message)
{
	if (a_message.type == RE::UI_MESSAGE_TYPE::kShow)
	{
		DebugMenu::GetDrawHandler()->isMenuOpen = true;
	}

	else if (a_message.type == RE::UI_MESSAGE_TYPE::kHide) 
	{
        DebugMenu::GetDebugMenuHandler()->isCoordinatesBoxVisible = false;
        DebugMenu::GetDrawHandler()->isMenuOpen = false;
    }

	return RE::UI_MESSAGE_RESULTS::kHandled;
}

void DrawMenu::OpenMenu()
{
	if (auto UIMessageQueue = RE::UIMessageQueue::GetSingleton())
		UIMessageQueue->AddMessage(MENU_NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr);
}

void DrawMenu::CloseMenu()
{
	if (auto UIMessageQueue = RE::UIMessageQueue::GetSingleton())
		UIMessageQueue->AddMessage(MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
}

void DrawMenu::clearCanvas()
{
	if (movie)
	{ 
		movie->Invoke("clear", nullptr, nullptr, 0);
	}
}

void DrawMenu::GetBox(RE::GFxValue& a_box, const char* a_boxName)
{
	RE::GFxValue root;
	if (movie)
	{
		movie->GetVariable(&root, "_root");
	}
	else logger::debug("Movie not obtained");

	if (!root.IsObject())
	{
		logger::debug("Root not obtained");
		return;
	}

	root.GetMember(a_boxName, &a_box);
}

void DrawMenu::ShowBox(const char* a_boxName)
{
	RE::GFxValue box;
	GetBox(box, a_boxName);

	auto res = box.Invoke("Show", nullptr, nullptr, 0);
}

void DrawMenu::HideBox(const char* a_boxName)
{
	RE::GFxValue box;
	GetBox(box, a_boxName);

	auto res = box.Invoke("Hide", nullptr, nullptr, 0);
}

void DrawMenu::SetScroll(int32_t& a_scroll, bool a_scrollUp)
{
	RE::GFxValue infoBox;
	GetBox(infoBox, "infoBox");

	RE::GFxValue getMaxScroll;
	bool res = infoBox.Invoke("GetMaxScroll", &getMaxScroll, nullptr, 0);

	int maxScroll = getMaxScroll.GetNumber();
	if (a_scroll > maxScroll) a_scroll = maxScroll;

	RE::GFxValue argsScroll{ a_scroll };
	if (a_scrollUp) res = infoBox.Invoke("ScrollUp", nullptr, &argsScroll, 1);
	else res = infoBox.Invoke("ScrollDown", nullptr, &argsScroll, 1);

	//logger::info("Trying to set scroll to: {}, res: {}", a_scroll, res);
}

void DrawMenu::SetInfoText(const std::string& a_text)
{
	RE::GFxValue infoBox;
	GetBox(infoBox, "infoBox");

	RE::GFxValue argsText{ a_text };
	auto res = infoBox.Invoke("SetText", nullptr, &argsText, 1);

	RE::GFxValue _lastTextLine;
	infoBox.Invoke("GetLastLine", &_lastTextLine, nullptr, 0);

	SetScroll(DebugMenu::GetDrawHandler()->infoBoxScroll, false);
	ShowBox("infoBox");
}

void DrawMenu::SetCoordinates(float a_x, float a_y, float a_z)
{
	RE::GFxValue coordinatesBox;
	GetBox(coordinatesBox, "coordinatesBox");

	const std::string xString = fmt::format("x: {:>7.0f}", a_x);
	const std::string yString = fmt::format("y: {:>7.0f}", a_y);
	const std::string zString = fmt::format("z: {:>7.0f}", a_z);

	RE::GFxValue argsText[3]; argsText[0] = xString; argsText[1] = yString; argsText[2] = zString;
	auto res = coordinatesBox.Invoke("SetCoordinates", nullptr, argsText, 3);
}


void DrawMenu::DrawPoint(RE::NiPoint2 a_position, float a_radius, uint32_t a_color, uint32_t a_alpha)
{
	if (!movie) 
	{
		logger::debug("Movie not obtained");
		return;
	}

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
void DrawMenu::DrawSimpleLine(RE::NiPoint2 a_start, RE::NiPoint2 a_end, float a_thickness, uint32_t a_color, uint32_t a_alpha) 
{
	if (!movie) return;

	RE::GFxValue argsLineStyle[3]{a_thickness, a_color, a_alpha};
	movie->Invoke("lineStyle", nullptr, argsLineStyle, 3);

	RE::GFxValue argsStartPos[2]{ a_start.x, a_start.y };
	movie->Invoke("moveTo", nullptr, argsStartPos, 2);

	RE::GFxValue argsEndPos[2]{ a_end.x, a_end.y };
	movie->Invoke("lineTo", nullptr, argsEndPos, 2);

	movie->Invoke("endFill", nullptr, nullptr, 0);
}


void DrawMenu::DrawLine(RE::NiPoint2 a_start, RE::NiPoint2 a_end, float a_startRadius, float a_endRadius, uint32_t a_color, uint32_t a_alpha)
{
	if (!movie) return;

	constexpr float theta = 3.14159265359 / 4;

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


void DrawMenu::DrawPolygon(const std::vector<RE::NiPoint2>& a_positions, float a_borderThickness, uint32_t a_color, uint32_t a_baseAlpha, uint32_t a_borderColor, uint32_t a_borderAlpha)
{
	if (!movie) return;

	/**/

	RE::GFxValue argsLineStyle[3]{ a_borderThickness, a_borderColor, a_borderAlpha};
	movie->Invoke("lineStyle", nullptr, argsLineStyle, 3);

	RE::GFxValue argsFill[2]{ a_color, a_baseAlpha };
	movie->Invoke("beginFill", nullptr, argsFill, 2);

	RE::GFxValue argsStartPos[2]{ a_positions[0].x, a_positions[0].y};
	movie->Invoke("moveTo", nullptr, argsStartPos, 2);
	/*
	for (int i = 1; i < 3; i++)
	{
		RE::GFxValue argsNextPos[2]{ a_positions[i].x, a_positions[i].y};
		movie->Invoke("lineTo", nullptr, argsNextPos, 2);
	}
	movie->Invoke("lineTo", nullptr, argsStartPos, 2);

	int first = a_positions.size()-1;
	int last = 2;
	int sgn = 1;
	int count = 0;
	while (first != last && count < 100)
	{
		count++;
		logger::info("first, last, {}, {}", first, last );
		RE::GFxValue argsFirstPos[2]{ a_positions[first].x, a_positions[first].y};
		RE::GFxValue argsLastPos[2]{ a_positions[last].x, a_positions[last].y};

		movie->Invoke("lineTo", nullptr, argsFirstPos, 2);
		movie->Invoke("lineTo", nullptr, argsLastPos, 2);

		if (first - last > -1.5 && first - last < 1.5) break;

		int placeholder = first;
		first = last + sgn;
		if (first == last) break;
		last = placeholder - sgn;
		logger::info("after change: first, last, {}, {}", first, last);
		sgn *= -1;
	} 

	movie->Invoke("endFill", nullptr, nullptr, 0);
	*/

	//*
	for (int i = 1; i < a_positions.size(); i++)
	{
		RE::GFxValue argsNextPos[2]{ a_positions[i].x, a_positions[i].y};
		movie->Invoke("lineTo", nullptr, argsNextPos, 2);
	}
	movie->Invoke("endFill", nullptr, nullptr, 0);
	//*/
}

void DrawMenu::DrawTriangle(RE::NiPoint2 a_positions[3], uint32_t a_color, uint32_t a_baseAlpha, uint32_t a_borderAlpha)
{
	if (!movie) return;

	RE::GFxValue argsLineStyle[3]{ 2, a_color, a_borderAlpha};
	movie->Invoke("lineStyle", nullptr, argsLineStyle, 3);

	RE::GFxValue argsFill[2]{ a_color, a_baseAlpha };
	movie->Invoke("beginFill", nullptr, argsFill, 2);

	RE::GFxValue argsStartPos[2]{ a_positions[0].x, a_positions[0].y};
	movie->Invoke("moveTo", nullptr, argsStartPos, 2);

	for (uint16_t i = 1; i < 3; i++)
	{
		RE::GFxValue argsNextPos[2]{ a_positions[i].x, a_positions[i].y};
		movie->Invoke("lineTo", nullptr, argsNextPos, 2);
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
