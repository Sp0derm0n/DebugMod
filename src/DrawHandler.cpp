#include "DrawHandler.h"
#include "Linalg.h"
#include "MCM.h"
#include "UIHandler.h"


void DrawHandler::Init() // called on ui thread
{
	GetDrawMenu();
	if (!g_DrawMenu)
	{
		logger::info("Failed to initialize camera handler");
		return;
	}
	g_DrawMenu->HideInfoBox();

	canvasWidth = g_DrawMenu->canvasWidth;
	canvasHeight = g_DrawMenu->canvasHeight;

	left = canvasWidth/2*(1-MCM::settings::canvasScale);
	right = left + canvasWidth*MCM::settings::canvasScale;
	top = canvasHeight/2*(1-MCM::settings::canvasScale);
	bottom = top + canvasHeight*MCM::settings::canvasScale;

	niCamera = GetNiCamera(RE::PlayerCamera::GetSingleton());
	playerCamera = RE::PlayerCamera::GetSingleton();

	if (!niCamera || !playerCamera) logger::info("Failed to initialize camera handler");
	
}

void DrawHandler::InfoBoxScrollUp()
{
	if (!g_DrawMenu || !isMenuOpen || !isInfoBoxVisible) return;

	infoBoxScroll -= linesScrolledPerClick;
	if (infoBoxScroll < 1) infoBoxScroll = 1;

	g_DrawMenu->SetScroll(infoBoxScroll, true /* scrollUp */);
	
}

void DrawHandler::InfoBoxScrollDown()
{
	if (!g_DrawMenu || !isMenuOpen || !isInfoBoxVisible) return;

	infoBoxScroll += linesScrolledPerClick;

	g_DrawMenu->SetScroll(infoBoxScroll, false /* scrollUp */);
}

void DrawHandler::UpdateCanvasScale() 
{
	left = canvasWidth/2*(1-MCM::settings::canvasScale);
	right = left + canvasWidth*MCM::settings::canvasScale;
	top = canvasHeight/2*(1-MCM::settings::canvasScale);
	bottom = top + canvasHeight*MCM::settings::canvasScale;
}

void DrawHandler::GetDrawMenu()
{
	auto ui = RE::UI::GetSingleton();
	g_DrawMenu = ui ? ui->GetMenu<DrawMenu>(DrawMenu::MENU_NAME) : nullptr;
}

void DrawHandler::ClearAll()
{
	pointsToDraw.clear();
	linesToDraw.clear();
	polygonsToDraw.clear();
}

void DrawHandler::Update(float a_delta)
{
	projectionMatrix = Linalg::Matrix4(niCamera->GetRuntimeData().worldToCam);

	g_DrawMenu->clearCanvas();


	//auto begin = std::chrono::high_resolution_clock::now();

	DrawPolygons(a_delta);
	DrawLines();
	DrawPoints();
	if (MCM::settings::showCrosshair) DrawCrosshair();
	if (MCM::settings::showCanvasBorder) DrawCanvasBorders();
	

	//auto end = std::chrono::high_resolution_clock::now();
	//auto dt = std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count();
	//logger::info("time elapesed = {} µs", dt);
}

void DrawHandler::DrawPoints()
{
	for (const auto& pointData : pointsToDraw)
	{
		Linalg::Vector4 clipPoint = worldToClipPoint(pointData->position);

		if (isPointOnScreen(clipPoint))
		{
			auto screenspaceData = PointToScreenspace(clipPoint);
			g_DrawMenu->DrawPoint(screenspaceData.point, pointData->radius*screenspaceData.scale, pointData->color, pointData->alpha);
		}
	}
}

void DrawHandler::DrawLines()
{
	for (const auto& lineData : linesToDraw)
	{
		Linalg::Vector4 clipPoint1 = worldToClipPoint(lineData->start);
		Linalg::Vector4 clipPoint2 = worldToClipPoint(lineData->end);

		if (ClipLine(clipPoint1, clipPoint2))
		{
			auto screenspaceData1 = PointToScreenspace(clipPoint1);
			auto screenspaceData2 = PointToScreenspace(clipPoint2);

			if(lineData->isSimpleLine)
				g_DrawMenu->DrawSimpleLine(screenspaceData1.point, screenspaceData2.point, lineData->thickness*(screenspaceData1.scale + screenspaceData2.scale)/2, lineData->color, lineData->alpha);
			else
				g_DrawMenu->DrawLine(screenspaceData1.point, screenspaceData2.point, lineData->thickness*screenspaceData1.scale, lineData->thickness*screenspaceData2.scale, lineData->color, lineData->alpha);
		}
	}
}

RE::TESForm* GetFormFromFile(RE::FormID a_formID, std::string_view a_filename)
{
	/*
	using func_t = RE::TESForm*(RE::BSScript::Internal::VirtualMachine*, RE::VMStackID, RE::FormID, std::string_view);
	RE::VMStackID frame = 0;  

	REL::Relocation<func_t> func{ RELOCATION_ID(55672, 55465) };
	auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
	return func(vm, frame, a_formID, a_filename);
	*/

	using func_t = decltype(&GetFormFromFile);
	REL::Relocation<func_t> func{ RELOCATION_ID(102238, 55465) };
	return func(a_formID, a_filename);
};

void DrawHandler::DrawPolygons(float a_delta)
{
	for (const auto& polygonData : polygonsToDraw)
	{
		std::vector<Linalg::Vector4> clipPoints;
		for (const auto& position : polygonData->positions)
		{
			clipPoints.push_back(worldToClipPoint(position));
		}

		if (!ClipPolygon(clipPoints)) continue; // no clipping happens if the entire polygon is off screen

		ScreenspacePolygon polygon = PolygonToScreenspace(clipPoints);
		if (polygon.points.size() < 2) continue; // the polygon can only be drawn if it contains at least 3 points
			
		g_DrawMenu->DrawPolygon(polygon.points, polygonData->borderThickness*polygon.avgScale, polygonData->color, polygonData->baseAlpha, polygonData->borderAlpha);





		///////////// vvv - Show info - vvv /////////////////////////////////////////////////////
		if (!MCM::settings::showInfoOnHover || isPolygonHighlighted || polygonData->info.empty() || UIHandler::GetSingleton()->isMenuOpen) continue; // only show info if it isnt already on for another polygon and the polygon has a formID attached to it and the debug menu is not open
		
		if (RE::PlayerCharacter::GetSingleton()->AsActorState()->IsSprinting()) continue;

		for (int i = 0; i < polygon.points.size(); i++)
		{
			RE::NiPoint2& point = polygon.points[i];
			float centerX = canvasWidth/2;
			float centerY = canvasHeight/2;
			if (point.x > centerX - infoRadius && point.x < centerX + infoRadius && point.y > centerY - infoRadius && point.y < centerY + infoRadius)
			{
				// highlightPolygon = the polygon we're looking at. size = 0 when not looking at any 
				if (highlightPolygon.size() == 0) 
				{
					highlightPolygon = polygonData->positions;
				}
				else if (highlightPolygon != polygonData->positions) continue; // skip all other polygons when looking at a polygon


				if(clipPoints[i].w > MCM::settings::infoRange) continue; // w defined as the distance to the point

				isPolygonHighlighted = true; // sat to false each frame at the bottom, so set to true each frame here
				
				timeHovering += a_delta;
				if (timeHovering < infoDelay) continue; // have this after 'isPolygonHighlighted = true' to only run once per frame (and not once per point in range)

				g_DrawMenu->DrawPoint(point, 15*polygon.scales[i], 0xFFFFFF, 100); // draw the point each frame

				if (isInfoBoxVisible) continue; // only fill out the infobox when it opens

				isInfoBoxVisible = true;
				g_DrawMenu->SetInfoText(polygonData->info);
			}
		}
	}

	if (!isPolygonHighlighted)
	{
		highlightPolygon.clear();
		timeHovering = 0.0f;

		if (isInfoBoxVisible)
		{
			g_DrawMenu->HideInfoBox();
			isInfoBoxVisible = false;
		}
	}
	isPolygonHighlighted = false;
}

void DrawHandler::DrawCrosshair()
{
	g_DrawMenu->DrawPoint(RE::NiPoint2(canvasWidth/2, canvasHeight/2), 2, 0x000000, 100);
	g_DrawMenu->DrawPoint(RE::NiPoint2(canvasWidth/2, canvasHeight/2), 1.5, 0xFFFFFF, 100);
}

void DrawHandler::DrawCanvasBorders()
{
	RE::NiPoint2 corner1{ left, top };
	RE::NiPoint2 corner2{ right, top };
	RE::NiPoint2 corner3{ right, bottom };
	RE::NiPoint2 corner4{ left, bottom };

	g_DrawMenu->DrawSimpleLine(corner1, corner2, 4, 0xFFFFFF, 70);
	g_DrawMenu->DrawSimpleLine(corner2, corner3, 4, 0xFFFFFF, 70);
	g_DrawMenu->DrawSimpleLine(corner3, corner4, 4, 0xFFFFFF, 70);
	g_DrawMenu->DrawSimpleLine(corner4, corner1, 4, 0xFFFFFF, 70);

}

DrawHandler::ScreenspacePoint DrawHandler::PointToScreenspace(const Linalg::Vector4& a_point)
{
	
	float scale = 200/a_point.w; // 200 chosen arbitrarily
	float x = a_point.x/a_point.w;
	float y = a_point.y/a_point.w;
	x = (x + 1)/2 * canvasWidth;
	y = (1 - y)/2 * canvasHeight;
	return ScreenspacePoint(RE::NiPoint2(x, y), scale);
}
DrawHandler::ScreenspacePolygon DrawHandler::PolygonToScreenspace(const std::vector<Linalg::Vector4>& a_points)
{
	std::vector<RE::NiPoint2> screenspacePoints;
	std::vector<float> scales;
	float avgScale = 0;
	for (const auto& point : a_points)
	{
		ScreenspacePoint spPoint = PointToScreenspace(point);
		screenspacePoints.push_back(spPoint.point);
		scales.push_back(spPoint.scale);
		avgScale += spPoint.scale;
	}
	int n = a_points.size();
	return ScreenspacePolygon(screenspacePoints, scales, avgScale/(n == 0 ? 1 : n));
}

bool DrawHandler::isPointOnScreen(const Linalg::Vector4& a_clipPoint)
{
	float w = a_clipPoint.w*(1+bufferScale);
	float scaled_w = w*MCM::settings::canvasScale;
	if (a_clipPoint.z >= -w        && a_clipPoint.z <= w && 
		a_clipPoint.x >= -scaled_w && a_clipPoint.x <= scaled_w &&
		a_clipPoint.y >= -scaled_w && a_clipPoint.y <= scaled_w) 
		return true;
	return false;
}

// Sutherland–Hodgman https://en.wikipedia.org/wiki/Sutherland%E2%80%93Hodgman_algorithm
bool DrawHandler::ClipPolygon(std::vector<Linalg::Vector4>& a_points)
{

	// first if all points are at one side of the frustum, so not visible
	bool areAllPointsBehind = true;
	bool areAllPointsInFront = true;
	bool areAllPointsToTheLeft = true;
	bool areAllPointsToTheRight = true;
	bool areAllPointsAbove = true;
	bool areAllPointsBelow = true;
	for (const auto& point : a_points)
	{
		areAllPointsBehind		*= point.z < -point.w;
		areAllPointsInFront		*= point.z >  point.w;

		areAllPointsToTheLeft	*= point.x < -point.w*MCM::settings::canvasScale;
		areAllPointsToTheRight	*= point.x >  point.w*MCM::settings::canvasScale;
		areAllPointsBelow		*= point.y < -point.w*MCM::settings::canvasScale;
		areAllPointsAbove		*= point.y >  point.w*MCM::settings::canvasScale;
	}
	if (areAllPointsBehind || areAllPointsInFront || areAllPointsToTheLeft || areAllPointsToTheRight || areAllPointsAbove || areAllPointsBelow)
	{
		return false;
	}

	Linalg::Vector4   leftPlane{ -1,  0,  0,  1 };
	Linalg::Vector4  rightPlane{  1,  0,  0,  1 };
	Linalg::Vector4 bottomPlane{  0, -1,  0,  1 };
	Linalg::Vector4    topPlane{  0,  1,  0,  1 };
	Linalg::Vector4   nearPlane{  0,  0, -1,  1 };
	Linalg::Vector4    farPlane{  0,  0,  1,  1 };

	Linalg::Vector4* planes[6]{ &leftPlane, &rightPlane, &bottomPlane, &topPlane, &nearPlane, &farPlane };

	int sgn = 1;
	for (int plane = 0; plane < 6; plane++)
	{
		sgn *= -1;
		std::vector<Linalg::Vector4> input = a_points;
		a_points.clear();

		int n = input.size();
		for (int i = 0; i < n; i++) // loop over all points
		{
			Linalg::Vector4 currentPoint = input[i];
			Linalg::Vector4 prevPoint = input[i == 0 ? n-1 : i-1];

			float scale = plane < 4 ? MCM::settings::canvasScale : 1;

			float prevCoord = prevPoint.x;
			float currentCoord = currentPoint.x;
			if (plane > 1)
			{
				prevCoord = prevPoint.y;
				currentCoord = currentPoint.y;
			}
			if (plane > 3)
			{
				prevCoord = prevPoint.z;
				currentCoord = currentPoint.z;
			}
			float delta_w = currentPoint.w - prevPoint.w;

			float t = (sgn * prevPoint.w * scale - prevCoord) / (currentCoord - prevCoord - sgn * delta_w * scale);
			/*// ---Definitions of t for the 6 planes borders (prev point (1) --> current point (2) ) --O
			|  (-w1 * canvasScale - x1) / ( x2 - x1 + (w2 - w1)*cavasScale );	// left					|
			|  (+w1 * canvasScale - x1) / ( x2 - x1 - (w2 - w1)*cavasScale );	// right				|
			|  (-w1 * canvasScale - y1) / ( y2 - y1 + (w2 - w1)*cavasScale );	// bottom				|
			|  (+w1 * canvasScale - y1) / ( y2 - y1 - (w2 - w1)*cavasScale );	// top					|
			|  (-w1				  - z1)	/ ( z2 - z1 + (w2 - w1)			   );	// near					|
			|  (+w1				  - z1) / ( z2 - z1 - (w2 - w1)			   );	// top					|
			\*/// --------------------------------------------------------------------------------------O

			// we know 0 < t < 1 because current point and prev point is to either side of the plane
			Linalg::Vector4  intersectionPoint = prevPoint + (currentPoint - prevPoint)*t;

			bool isCurrentPointInside;
			bool isPrevPointOutside;
			if (plane == 0) // left
			{
				isCurrentPointInside = currentPoint.x > -currentPoint.w*MCM::settings::canvasScale;
				isPrevPointOutside = prevPoint.x < -prevPoint.w*MCM::settings::canvasScale;
			}
			else if (plane == 1) // right
			{
				isCurrentPointInside = currentPoint.x < currentPoint.w*MCM::settings::canvasScale;
				isPrevPointOutside = prevPoint.x > prevPoint.w * MCM::settings::canvasScale;
			}
			else if (plane == 2) // bottom
			{
				isCurrentPointInside = currentPoint.y > -currentPoint.w*MCM::settings::canvasScale;
				isPrevPointOutside = prevPoint.y < -prevPoint.w * MCM::settings::canvasScale;
			}
			else if (plane == 3) // top
			{
				isCurrentPointInside = currentPoint.y < currentPoint.w*MCM::settings::canvasScale;
				isPrevPointOutside = prevPoint.y > prevPoint.w*MCM::settings::canvasScale;
			}
			else if (plane == 4) // near
			{
				isCurrentPointInside = currentPoint.z > -currentPoint.w;
				isPrevPointOutside = prevPoint.z < -prevPoint.w;
			}
			else // far
			{
				isCurrentPointInside = currentPoint.z < currentPoint.w;
				isPrevPointOutside = prevPoint.z > prevPoint.w;
			}

			if (isCurrentPointInside)
			{
				if (isPrevPointOutside)
				{
					a_points.push_back(intersectionPoint);
				}
				a_points.push_back(currentPoint);
			}
			else if (!isPrevPointOutside)
			{
				a_points.push_back(intersectionPoint);
			}
		}
	}
	return true;
}

bool DrawHandler::ClipLine(Linalg::Vector4& a_point1, Linalg::Vector4& a_point2)
{
	if (a_point1.z < -a_point1.w && a_point2.z < -a_point2.w || a_point1.z > a_point1.w && a_point2.z > a_point2.w) // both points behind camera or too far from the camera = nothing visible
		return false;

	float scaled_w1 = a_point1.w*MCM::settings::canvasScale*(1+bufferScale);
	float scaled_w2 = a_point2.w*MCM::settings::canvasScale*(1+bufferScale);

	if (a_point1.x < -scaled_w1 && a_point2.x < -scaled_w2 || a_point1.x > scaled_w1 && a_point2.x > scaled_w2) // both points to the right or left = nothing visible
		return false;

	if (a_point1.y < -scaled_w1 && a_point2.y < -scaled_w2 || a_point1.y > scaled_w1 && a_point2.y > scaled_w2) // both points above or below =  nothing visible
		return false;

	// if the line may be visible, but one point is behind the near plane, clip that point to the nearplane - then do 2D clipping
	if (a_point1.z < -a_point1.w) // clip point 1 to z = 0
	{
		float t = (-a_point1.w - a_point1.z) / (a_point2.z - a_point1.z + (a_point2.w - a_point1.w));
		a_point1.x += (a_point2.x - a_point1.x)*t;
		a_point1.y += (a_point2.y - a_point1.y)*t;
		a_point1.z += (a_point2.z - a_point1.z)*t;
		a_point1.w += (a_point2.w - a_point1.w)*t;
	}
	else if (a_point1.z > a_point1.w) // clip point 1 to z = w
	{
		float t = (a_point1.w - a_point1.z) / (a_point2.z - a_point1.z - (a_point2.w - a_point1.w));
		a_point1.x += (a_point2.x - a_point1.x)*t;
		a_point1.y += (a_point2.y - a_point1.y)*t;
		a_point1.z += (a_point2.z - a_point1.z)*t;
		a_point1.w += (a_point2.w - a_point1.w)*t;
	}

	if (a_point2.z < -a_point2.w) // clip point 2 to z = 0
	{
		float t = (-a_point2.w - a_point2.z) / (a_point1.z - a_point2.z + (a_point1.w - a_point2.w));
		a_point2.x += (a_point1.x - a_point2.x)*t;
		a_point2.y += (a_point1.y - a_point2.y)*t;
		a_point2.z += (a_point1.z - a_point2.z)*t;
		a_point2.w += (a_point1.w - a_point2.w)*t;
	}
	else if (a_point2.z > a_point2.w) // clip point 2 to z = w
	{
		float t = (a_point2.w - a_point2.z) / (a_point1.z - a_point2.z - (a_point1.w - a_point2.w));
		a_point1.x += (a_point1.x - a_point2.x)*t;
		a_point1.y += (a_point1.y - a_point2.y)*t;
		a_point1.z += (a_point1.z - a_point2.z)*t;
		a_point1.w += (a_point1.w - a_point2.w)*t;
	}

	// if both points are on the screen, return them
	if (isPointOnScreen(a_point1) && isPointOnScreen(a_point2)) 
		return true;

	bool isLineVisible = false;
	Linalg::Vector4* points[2]{ &a_point1, &a_point2};

	for (int a = 0; a < 2; a++)
	{
		int b = 1 - a;
		if (isPointOnScreen(*points[a])) continue;

		float x;
		float y;
		float z;
		float w;
		float tMin = 2;


		Linalg::Vector4 delta = *points[b] - *points[a];

		int sgn = 1;
		for (int edge = 0; edge < 4; edge++) // left, right, bottom, top
		{
			sgn *= -1;
			float a_w = points[a]->w;
			float a_xy	   = edge < 2 ? points[a]->x : points[a]->y;
			float delta_xy = edge < 2 ? delta.x : delta.y;

			float t = (sgn*a_w*MCM::settings::canvasScale - a_xy) / (delta_xy - sgn*delta.w*MCM::settings::canvasScale);
			/*// ---Definitions of t for the 4 borders (point1 --> point 2) --------------O
			|  (-w1 * canvasScale - x1) / ( x2 - x1 + (w2 - w1)*cavasScale );	// left   |
			|  (+w1 * canvasScale - x1) / ( x2 - x1 - (w2 - w1)*cavasScale );	// right  |
			|  (-w1 * canvasScale - y1) / ( y2 - y1 + (w2 - w1)*cavasScale );	// bottom |
			|  (+w1 * canvasScale - y1) / ( y2 - y1 - (w2 - w1)*cavasScale );	// top    |
			\*/// ------------------------------------------------------------------------O


			if (t < 0 || t > 1) continue; // line doesnt cross the axis along the current edge


			float newX = points[a]->x + delta.x*t;
			float newY = points[a]->y + delta.y*t;
			float newZ = points[a]->z + delta.z*t;
			float newW = points[a]->w + delta.w*t;

			float scaledW = newW*MCM::settings::canvasScale*(1 + bufferScale); // bufferscale adds an extra region around the canvas where the point wont be cut off


			if (edge < 2) { if(newY < -scaledW || newY > scaledW) continue; } // when checking left or right edge, the line doesnt cross
			else if(newX < -scaledW || newX > scaledW) continue; // when checking top or bottom edge, the line doesnt cross

			// can be optimized: if the first point is on the right block of the canvas, only the right edge can be cut first,
			// If it is in the upper right corner, only the top or the right edge can be cut.

			if (t < tMin) // if the line crosses this edge before a previously saved edge, overwrite that edge
			{
				tMin = t;
				x = newX;
				y = newY;
				z = newZ;
				w = newW;
			}
		}
		if (tMin != 2)
		{
			points[a]->x = x;
			points[a]->y = y;
			points[a]->z = z;
			points[a]->w = w;
			isLineVisible = true; // if a points has been clipped, it means it crossed one of the edges and is therefore visible
		}
	}

	return isLineVisible;
}

RE::NiPointer<RE::NiCamera> DrawHandler::GetNiCamera(RE::PlayerCamera* camera) {
	// Do other things parent stuff to the camera node? Better safe than sorry I guess

	if (camera->cameraRoot->GetChildren().size() == 0) return nullptr;
	for (auto& entry : camera->cameraRoot->GetChildren()) {
		auto asCamera = skyrim_cast<RE::NiCamera*>(entry.get());
		if (asCamera) return RE::NiPointer<RE::NiCamera>(asCamera);
	}
	return nullptr;
}

Linalg::Vector4 DrawHandler::worldToClipPoint(const RE::NiPoint3& a_position)
{
	return projectionMatrix*Linalg::Vector4(a_position);
}

void DrawHandler::DrawPoint(RE::NiPoint3 a_position, float a_scale, uint32_t a_color, uint32_t a_alpha)
{
	pointsToDraw.push_back(std::make_unique<PointData>(a_position, a_scale, a_color, a_alpha*alphaMultiplier));
}

void DrawHandler::DrawLine(RE::NiPoint3 a_start, RE::NiPoint3 a_end, float a_thickness, uint32_t a_color, uint32_t a_alpha, bool a_isSimpleLine)
{
	linesToDraw.push_back(std::make_unique<LineData>(a_start, a_end, a_thickness, a_color, a_alpha*alphaMultiplier, a_isSimpleLine));
}

void DrawHandler::DrawPolygon(std::vector<RE::NiPoint3> a_positions, float a_borderThickness, uint32_t a_color, uint32_t a_baseAlpha, uint32_t a_borderAlpha, std::string a_info)
{
	polygonsToDraw.push_back(std::make_unique<PolygonData>(a_positions, a_borderThickness, a_color, a_baseAlpha*alphaMultiplier, a_borderAlpha*alphaMultiplier, a_info));
}

// not used
void DrawHandler::BuildProjectionMatrix()
{
	if(!niCamera || !playerCamera) return;

	auto frustum = niCamera->GetRuntimeData2().viewFrustum;
	float n = frustum.fNear;
	float f = frustum.fFar;
	float r = frustum.fRight;
	float t = frustum.fTop;

	RE::NiMatrix3 projectionMatrix3x3Component;
	projectionMatrix3x3Component.entry[0][0] = 1 / r;
	projectionMatrix3x3Component.entry[1][2] = 1 / t;
	projectionMatrix3x3Component.entry[2][1] = (f + n) / (f - n);
	projectionMatrix3x3Component.entry[1][1] = 0; // NiMatrix3 initializes as identity
	projectionMatrix3x3Component.entry[2][2] = 0;


	RE::NiMatrix3 cameraRotationMatrix = playerCamera->cameraRoot->world.rotate;
	RE::NiMatrix3 cameraRotationMatrixInverse = cameraRotationMatrix.Transpose(); // rot matrices are orthogonal
	RE::NiMatrix3 rotated3x3Component = projectionMatrix3x3Component*cameraRotationMatrixInverse;

	RE::NiPoint3 offset = RE::NiPoint3(0, 0, 2*f*n/(n - f));
	RE::NiPoint3 cameraPosition = playerCamera->cameraRoot->world.translate;
	RE::NiPoint3 transformedOffset = offset - rotated3x3Component*cameraPosition;

	cameraFacingDirection = cameraRotationMatrix*RE::NiPoint3(0, 1, 0);
	float extraOffset = -cameraFacingDirection*cameraPosition;

	projectionMatrix(0, 0) = rotated3x3Component.entry[0][0];
	projectionMatrix(0, 1) = rotated3x3Component.entry[0][1];
	projectionMatrix(0, 2) = rotated3x3Component.entry[0][2];
	projectionMatrix(1, 0) = rotated3x3Component.entry[1][0];
	projectionMatrix(1, 1) = rotated3x3Component.entry[1][1];
	projectionMatrix(1, 2) = rotated3x3Component.entry[1][2];
	projectionMatrix(2, 0) = rotated3x3Component.entry[2][0];
	projectionMatrix(2, 1) = rotated3x3Component.entry[2][1];
	projectionMatrix(2, 2) = rotated3x3Component.entry[2][2];

	projectionMatrix(0, 3) = transformedOffset[0];
	projectionMatrix(1, 3) = transformedOffset[1];
	projectionMatrix(2, 3) = transformedOffset[2];

	projectionMatrix(3, 0) = cameraFacingDirection[0];
	projectionMatrix(3, 1) = cameraFacingDirection[1];
	projectionMatrix(3, 2) = cameraFacingDirection[2];

	projectionMatrix(3, 3) = extraOffset;
}