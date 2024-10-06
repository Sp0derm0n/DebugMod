#include "HUDHandler.h"
#include "DrawMenu.h"
#include "Utils.h"

void HUDHandler::init()
{
	canvasWidth = 1;
	canvasHeight = 1;
}

RE::NiPoint3 lp = RE::NiPoint3(1,1,1);


RE::NiPointer<RE::NiCamera> GetNiCamera(RE::PlayerCamera* camera)
{
	// Do other things parent stuff to the camera node? Better safe than sorry I guess
	if (camera->cameraRoot->GetChildren().size() == 0) return nullptr;
	for (auto& entry : camera->cameraRoot->GetChildren()) {
		auto asCamera = skyrim_cast<RE::NiCamera*>(entry.get());
		if (asCamera) return RE::NiPointer<RE::NiCamera>(asCamera);
	}
	return nullptr;
}

void HUDHandler::updateCameraPosition()
{
	auto playerCamera = RE::PlayerCamera::GetSingleton();
	RE::NiQuaternion q;
	auto lastpos = RE::NiPoint3(0,0,0);
	cameraFOV = playerCamera->GetRuntimeData2().worldFOV;

	
	if (playerCamera->IsInFirstPerson())
	{
		auto fps = reinterpret_cast<RE::FirstPersonState*>(playerCamera->GetRuntimeData().cameraStates[RE::CameraState::kFirstPerson].get());
		Utils::FPSGetCurrentRotation(fps, q);
		cameraPosition = playerCamera->cameraRoot->world.translate;
		cameraFOV += fps->firstPersonFOVControl->local.translate.z;
	}
	else if (playerCamera->IsInThirdPerson())
	{
		auto tps = reinterpret_cast<RE::ThirdPersonState*>(playerCamera->GetRuntimeData().cameraStates[RE::CameraState::kThirdPerson].get());
		Utils::TPSGetCurrentRotation(tps, q);
		Utils::TPSGetCurrentPosition(tps, cameraPosition); 
		cameraFOV += tps->thirdPersonFOVControl->local.translate.z;
	}
	else if (playerCamera->IsInFreeCameraMode())
	{
		auto fcs = reinterpret_cast<RE::FreeCameraState*>(playerCamera->GetRuntimeData().cameraStates[RE::CameraState::kFree].get());
		Utils::FreeCamStateGetCurrentRotation(fcs, q);
		cameraPosition = playerCamera->GetRuntimeData2().pos;
	}
	cameraPosition = playerCamera->cameraRoot->world.translate;

	// The camera rotation quaternion describes the transformation of the vector pointing north (0, 1, 0)
	auto directionStartVec = RE::NiPoint3(0, 1,  0);
	auto heightStartVec	   = RE::NiPoint3(0, 0, -1); //canvas y (z value of the initial cameraposition) value increases downwards
	auto widthStartVec	   = RE::NiPoint3(1, 0,  0);

	cameraDirectionUnitVector = Utils::vectorRotation(directionStartVec, Utils::quaternionUnit(q));
	cameraHeightUnitVector = Utils::vectorRotation(heightStartVec, Utils::quaternionUnit(q));
	cameraWidthUnitVector = Utils::vectorRotation(widthStartVec, Utils::quaternionUnit(q));
}

RE::GPtr<DrawMenu> HUDHandler::GetDrawMenuHud()
{
	auto ui = RE::UI::GetSingleton();
	return ui ? ui->GetMenu<DrawMenu>(DrawMenu::MENU_NAME) : nullptr;
}

void HUDHandler::updatePoints()
{

}

void HUDHandler::updateHUD()
{
	if(g_DrawMenu) //add framerate by using deltaTime
	{

		updateCameraPosition();
		g_DrawMenu->clearCanvas();

		auto begin = std::chrono::high_resolution_clock::now();

		// dont get the screenposition of the same point twice
		for (auto& point : pointsToDraw)
		{
			auto screenPositionData = worldPositionToScreenPosition(point->position);
			if (!screenPositionData.isBehindCamera && isOnScreen(screenPositionData.position, point->radius * screenPositionData.scale) && screenPositionData.scale != 0)
			{
				g_DrawMenu->DrawPoint(screenPositionData.position, point->radius*screenPositionData.scale, point->color, point->alpha);
			}
		}
		for (auto& line : linesToDraw)
		{
			auto screenPositionData1 = worldPositionToScreenPosition(line->start);
			auto screenPositionData2 = worldPositionToScreenPosition(line->end);

			if (screenPositionData1.isBehindCamera && !screenPositionData2.isBehindCamera && isOnScreen(screenPositionData1.position, 0))
			{
				screenPositionData1 = DrawLineWithOnePointBehindCamera(screenPositionData1, screenPositionData2, line->start, line->end);
			}
			else if (!screenPositionData1.isBehindCamera && screenPositionData2.isBehindCamera)
			{
				screenPositionData2 = DrawLineWithOnePointBehindCamera(screenPositionData2, screenPositionData1, line->end, line->start);
			}

			if (isOnScreen(screenPositionData1.position, screenPositionData2.position, line->thickness))
			{
				g_DrawMenu->DrawLine(screenPositionData1.position, screenPositionData2.position, line->thickness*screenPositionData1.scale, line->thickness*screenPositionData2.scale , line->color, line->alpha);
			}
		}

		auto end = std::chrono::high_resolution_clock::now();
		auto dt = std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count();
		//logger::info("dt {}", dt);
	}
	
}

Utils::ScreenPositionData HUDHandler::DrawLineWithOnePointBehindCamera(Utils::ScreenPositionData a_behindData, Utils::ScreenPositionData a_inFrontData, RE::NiPoint3 a_behindPoint, RE::NiPoint3 a_inFrontPoint)
{
	auto worldVectorBetweenPoints = a_behindPoint - a_inFrontPoint;
	worldVectorBetweenPoints /= worldVectorBetweenPoints.Length();

	auto newScreenPositionData = worldPositionToScreenPosition(a_inFrontPoint + worldVectorBetweenPoints);
	auto canvasVectorBetweenPoints = newScreenPositionData.position - a_inFrontData.position;
	canvasVectorBetweenPoints *= 1.42*canvasWidth/canvasVectorBetweenPoints.Length(); // make the new line equal sqrt(2) the canvas width so that under no circumstance will the line ever be to short to move out of the screen
	
	newScreenPositionData.position = a_inFrontData.position + canvasVectorBetweenPoints;
	return newScreenPositionData;
}

bool HUDHandler::isOnScreen(RE::NiPoint2 a_point, float a_radius)
{
	if (a_point.x + a_radius < 0 || a_point.y + a_radius < 0 || a_point.x - a_radius > g_DrawMenu->canvasWidth || a_point.y - a_radius > g_DrawMenu->canvasHeight)
		return false;
	return true;
}
bool HUDHandler::isOnScreen(RE::NiPoint2 a_start, RE::NiPoint2 a_end, float a_thickness)
{
	if (isOnScreen(a_start, a_thickness) || isOnScreen(a_end, a_thickness))
		return true;

	float a = (a_end.y - a_start.y) / (a_end.x - a_start.x);
	float b = a_start.y - a*a_start.x;

	bool lineCrossesLeftYAxis = b > 0 && b < canvasHeight;
	float yValueAtLeftEdge = b;
	float yValueAtRightEdge = a*canvasWidth + b;
	float xValueAtTop = - b/a;
	float xValueAtBottom = canvasHeight - xValueAtTop;

	if ((yValueAtLeftEdge > 0 && yValueAtLeftEdge < canvasHeight) || (yValueAtRightEdge > 0 && yValueAtRightEdge < canvasHeight) || (xValueAtBottom > 0 && xValueAtBottom < canvasWidth) || (xValueAtTop > 0 && xValueAtTop < canvasWidth))
		return true;
	return false;
}


Utils::ScreenPositionData HUDHandler::worldPositionToScreenPosition(RE::NiPoint3 a_position)
{
	return Utils::worldPositionToScreenPosition(a_position, cameraPosition, cameraFOV, cameraDirectionUnitVector, cameraWidthUnitVector, cameraHeightUnitVector, g_DrawMenu->canvasWidth, g_DrawMenu->canvasHeight);
}

void HUDHandler::DrawPoint(RE::NiPoint3 a_position, float a_scale, uint32_t a_color, uint32_t a_alpha)
{
	pointsToDraw.push_back(std::make_unique<PointData>(a_position, a_scale, a_color, a_alpha));
}
void HUDHandler::DrawLine(RE::NiPoint3 a_start, RE::NiPoint3 a_end, float a_thickness, uint32_t a_color, uint32_t a_alpha)
{
	linesToDraw.push_back(std::make_unique<LineData>(a_start, a_end, a_thickness, a_color, a_alpha));
}

/*

static uintptr_t g_worldToCamMatrix = RELOCATION_ID(519579, 406126).address();
static RE::NiRect<float>* g_viewPort = (RE::NiRect<float>*)RELOCATION_ID(519618, 406160).address();

std::pair<RE::NiPoint2, float> HUDHandler::worldPositionToScreenPosition(RE::NiPoint3 a_position)
{
	RE::NiPoint2 screenPos;
	float depth;

	RE::NiCamera::WorldPtToScreenPt3((float(*)[4])g_worldToCamMatrix, *g_viewPort, a_position, screenPos.x, screenPos.y, depth, 1e-5f);
	RE::GRectF rect = g_DrawMenu->movie->GetVisibleFrameRect();

	screenPos.x = rect.left + (rect.right - rect.left) * screenPos.x;
	screenPos.y = 1.f - screenPos.y;
	screenPos.y = rect.top + (rect.bottom - rect.top) * screenPos.y;

	return std::make_pair(screenPos,0.1);
}*/
