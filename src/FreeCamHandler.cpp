#include "FreeCamHandler.h"
#include "DrawMenu.h"
#include "DebugMenu/DebugMenu.h"
#include "DrawHandler.h"
#include "DebugUIMenu.h"
#include "MCM.h"
#include "Interface/UIHandler.h"

void FreeCamHandler::Init()
{
	deltaTime = (float*)RELOCATION_ID(523660, 410199).address();
	freeCamTranslationSpeed = (float*)RELOCATION_ID(509808, 382522).address();
	defaultFreeCamTranslationSpeed = *freeCamTranslationSpeed;

}

RE::BSEventNotifyControl FreeCamHandler::ProcessEvent(RE::InputEvent* const* eventPtr, RE::BSTEventSource<RE::InputEvent*>*)
{

	if (!MCM::settings::modActive) return RE::BSEventNotifyControl::kContinue; 

	if (!eventPtr) return RE::BSEventNotifyControl::kContinue;

	for (auto* event = *eventPtr; event; event = event->next)
	{
		if (event->GetEventType() != RE::INPUT_EVENT_TYPE::kButton) continue;

		uint32_t offset = 0;
		if (event->GetDevice() == RE::INPUT_DEVICE::kMouse) offset = 256;

		auto* buttonEvent = event->AsButtonEvent();
		auto dxScanCode = buttonEvent->GetIDCode();

		uint32_t keyCode = dxScanCode + offset;

		HandleInput(keyCode, buttonEvent->IsDown(), buttonEvent->HeldDuration());
	}
	return RE::BSEventNotifyControl::kContinue;
}

void FreeCamHandler::HandleInput(uint32_t a_keyCode, bool a_isDown, float a_heldDuration)
{
	if (a_keyCode == MCM::settings::descendHotkey && IsCustomFreeCamEnabled())
	{
		Descend();
		
	}
	else if (a_keyCode == MCM::settings::ascendHotkey)
	{
		auto freecam = skyrim_cast<RE::FreeCameraState*>(RE::PlayerCamera::GetSingleton()->GetRuntimeData().cameraStates[RE::CameraState::kFree].get());
		logger::info("freecam x address: {:X}", reinterpret_cast<uintptr_t>(freecam));
		if (a_isDown)
		{
			auto clockNow = std::chrono::system_clock::now();
			uint64_t timeSinceLastSpace = std::chrono::duration_cast<std::chrono::milliseconds>(clockNow - lastSpaceTimeStamp).count();
			lastSpaceTimeStamp = clockNow;

			if (MCM::settings::useDoubleAscendToFly && timeSinceLastSpace < doubleSpaceHighestDuration && !RE::UI::GetSingleton()->GameIsPaused())
			{
				ToggleTFC();
				lastSpaceTimeStamp = time_point();
			}
		}
		else if (IsCustomFreeCamEnabled())
		{
			Ascend();
		}
	}
	
	else if (IsCustomFreeCamEnabled())
	{
		if (RE::UI::GetSingleton()->GameIsPaused() ||
			ScaleformUI::GetDebugMenuUI()->IsOpen() ||
			ScaleformUI::UIHandler::GetSingleton()->IsInfoBoxOpen() ||
			!a_isDown) return; // dont change speeds if menu is open

		if (a_keyCode == HotKey::scrollDownKeyCode || a_keyCode == MCM::settings::scrollDownHotkey)
		{
			bool isScrolling = a_keyCode == HotKey::scrollDownKeyCode;
			DecreaseSpeed(isScrolling);
		}
		else if (a_keyCode == HotKey::scrollUpKeyCode || a_keyCode == MCM::settings::scrollUpHotkey)
		{
			bool isScrolling = a_keyCode == HotKey::scrollUpKeyCode;
			IncreaseSpeed(isScrolling);
		}
	}
}

void FreeCamHandler::ChangeSpeed(bool a_increase, bool a_isScrolling)
{
	float speedChange = speedIncrement;
	if (!a_isScrolling) speedChange *= 2;
	if (!a_increase) speedChange *= -1.0f;

	speed += speedChange;

	if (speed > maxSpeed) speed = maxSpeed;
	else if (speed < minSpeed) speed = minSpeed;
}

void FreeCamHandler::ToggleTFC()
{
	auto playerCamera = RE::PlayerCamera::GetSingleton();
	if (playerCamera && DebugMenu::GetDebugMenuHandler()->isDebugMenuActive)
	{
		playerCamera->ToggleFreeCameraMode(false);
		auto state = playerCamera->currentState.get();
		
		// Called twice when useTFC is true, but has to be so, since this must be called
		if (state->id == RE::CameraState::kFree) 
		{
			OnEnterCustomFreeCam();
		}
		// OnExitCustomFreeCam called through the FreeCameraState->End() hook;
	}
}

// Not working properly
void FreeCamHandler::SetPlayerAppCulled(bool a_cull)
{
	auto player = RE::PlayerCharacter::GetSingleton();
	if (auto player3D = player->Get3D())
	{
		player3D->SetAppCulled(a_cull);
	}
}

void FreeCamHandler::Update()
{
	if (!IsCustomFreeCamEnabled()) return;

	UpdateCameraSpeed();
	UpdatePlayerPosition();
}

static float lookInputVecY = 0.0f;

void FreeCamHandler::UpdateCameraSpeed()
{
	// The free cam updates its angle base on the lookInputVec
	// The y value of this vector is however framerate dependent
	// Therefore, we multiply it by the ratio of our framerate and 60
	// And reset it, after the camera has been rotated

	lookInputVecY = RE::PlayerControls::GetSingleton()->data.lookInputVec.y;

	float frameRate = deltaTime ? 1/ *deltaTime : 60.0f;
	float frameRatio = frameRate / 60.0f;

	RE::PlayerControls::GetSingleton()->data.lookInputVec.y *= frameRatio;


	// The free cam speed is so nicely programmed, that it moves a fixed distance per frame
	// So we update the translation speed every frame while in custom free cam, to make it
	// independant of framerate

	if (!freeCamTranslationSpeed || !deltaTime) return;
	
	float secondsPerFrame = *deltaTime;

	float unitsPerMeter = 70.0f;

	float unitsPerFrame = speed * unitsPerMeter * secondsPerFrame;

	// Moving diagonally increases speed by sqrt 2. Using a gamepad, the x and y of move vec are proably continuous
	auto moveVector = RE::PlayerControls::GetSingleton()->data.moveInputVec;
	auto moveVectorLength = moveVector.Length();
	if (moveVectorLength > 0.1f) unitsPerFrame /= moveVectorLength;

	*freeCamTranslationSpeed = unitsPerFrame;
}

void FreeCamHandler::PostUpdate()
{
	RE::PlayerControls::GetSingleton()->data.lookInputVec.y = lookInputVecY;
}

void FreeCamHandler::UpdatePlayerPosition()
{
	if (MCM::settings::playerFollowsCamera)
	{
		if (auto freeCam = GetFreeCamera())
		{
			auto player = RE::PlayerCharacter::GetSingleton();
			player->SetPosition(freeCam->translation + RE::NiPoint3{ 0.0f, 0.0f, 70.0f }, true);

			float cameraHeading = freeCam->rotation.y;
			player->SetHeading(cameraHeading);
		}
	}
}

void FreeCamHandler::OnEnterCustomFreeCam()
{
	// ToggleFreeCamera sets the free cam position to the previous cam position
	// However, sometimes, the previous camera position is just 0 0 124, so the free cam moves to there
	// So we manually set the free cam position to the correct position
	if (auto freeCam = GetFreeCamera())
	{
		auto cameraPos = GetCameraPosition();

		if ((cameraPos.x < 1) && (cameraPos.x > -1))
		{
			ToggleTFC();
			ScaleformUI::UIHandler::GetSingleton()->SetTFCButtonCorrectOnOffIcon();
			return;
		}

		freeCam->translation = cameraPos;

		defaultLockToZPlane = freeCam->lockToZPlane;
		defaultUseRunSpeed = freeCam->useRunSpeed;

		freeCam->useRunSpeed = false;


		if (MCM::settings::lockFreeCamToZPlane)
		{
			freeCam->lockToZPlane = true;
		}
		else
		{
			freeCam->lockToZPlane = false;
		}

		isInCustomFreeCam = true;
		ScaleformUI::UIHandler::GetSingleton()->SetTFCButtonCorrectOnOffIcon();

	}
}

void FreeCamHandler::OnExitCustomFreeCam()
{

	isInCustomFreeCam = false;
	ScaleformUI::UIHandler::GetSingleton()->SetTFCButtonCorrectOnOffIcon();


	if (auto freeCam = GetFreeCamera())
	{
		freeCam->lockToZPlane = defaultLockToZPlane;
		freeCam->useRunSpeed = defaultUseRunSpeed;

		*freeCamTranslationSpeed = defaultFreeCamTranslationSpeed;
	}

	if (MCM::settings::playerFollowsCamera) PutPlayerBackOnGround();
	
}

void FreeCamHandler::OnEnterFreeCam()
{
	if (!IsCustomFreeCamEnabled() && MCM::settings::useTFC && DebugMenu::GetDebugMenuHandler()->isDebugMenuActive) 
		OnEnterCustomFreeCam();
}

void FreeCamHandler::OnExitFreeCam()
{
	if (IsCustomFreeCamEnabled()) 
		OnExitCustomFreeCam();
}

RE::NiPoint3 FreeCamHandler::GetCameraPosition()
{
	const auto player = RE::PlayerCharacter::GetSingleton();

	auto cameraPosition = player->GetPosition();

	if (player->GetPlayerRuntimeData().playerFlags.isInThirdPersonMode)
	{
		if (auto thirdPersonTESCameraState = RE::PlayerCamera::GetSingleton()->GetRuntimeData().cameraStates[RE::CameraState::kThirdPerson])
		{
			if (auto thirdPersonState = skyrim_cast<RE::ThirdPersonState*>(thirdPersonTESCameraState.get()))
			{
				cameraPosition = thirdPersonState->translation;
			}
		}
	}
	else if (auto cameraRoot = player->GetNodeByName("Camera1st [Cam1]"))
	{
		cameraPosition += cameraRoot->local.translate * player->GetScale();
	}

	return cameraPosition;
}

void FreeCamHandler::PutPlayerBackOnGround()
{
	bool teleportPlayerToGround = false;
	float distanceToGround = 0.0f;

	if (auto charController = RE::PlayerCharacter::GetSingleton()->GetCharController())
	{
		if (auto bhkWorld = charController->GetHavokWorld())
		{
			RE::BSReadLockGuard locker(bhkWorld->worldLock);

			if (auto hkpWorld = bhkWorld->GetWorld1())
			{
				float havokScale = bhkWorld->GetWorldScale();

				auto player = RE::PlayerCharacter::GetSingleton();
				auto playerPosition = player->GetPosition() * havokScale;

				float zFloor = -30000.0f * havokScale;

				RE::hkVector4 start{ playerPosition.x, playerPosition.y, playerPosition.z, 0.0f };
				RE::hkVector4 end{ playerPosition.x, playerPosition.y, zFloor, 0.0f };

				RE::hkpWorldRayCastInput rayCastInput;
				rayCastInput.from = start;
				rayCastInput.to = end;

				RE::hkpWorldRayCastOutput rayCastResult;
				hkpWorld->CastRay(rayCastInput, rayCastResult);

				if (rayCastResult.HasHit())
				{
					teleportPlayerToGround = true;

					float rayLength = playerPosition.z + zFloor;
					distanceToGround = rayLength * rayCastResult.hitFraction / havokScale;
				}
			}
		}
	}

	if (teleportPlayerToGround)
	{
		auto player = RE::PlayerCharacter::GetSingleton();
		auto playerPosition = player->GetPosition();
		auto offset = 10.0f;
		player->SetPosition({ playerPosition.x, playerPosition.y, playerPosition.z + distanceToGround + offset }, true);

	}
}

RE::FreeCameraState* FreeCamHandler::GetFreeCamera()
{
	if (auto playerCamera = RE::PlayerCamera::GetSingleton())
	{
		auto cameraState = playerCamera->currentState;
		if (!cameraState || cameraState->id != RE::CameraState::kFree) return nullptr;
		return skyrim_cast<RE::FreeCameraState*>(cameraState.get());
	}

	return nullptr;
}

float FreeCamHandler::GetZSpeed()
{
	float zSpeed = speed*0.75;
	if (zSpeed < minZSpeed) zSpeed = minZSpeed;
	else if (zSpeed > maxZSpeed) zSpeed = maxZSpeed;

	return zSpeed;
}

void FreeCamHandler::MoveCameraUpDown(bool a_moveUp)
{
	if (auto freeCam = GetFreeCamera())
	{
		auto deltaZ = GetZSpeed() * unitsPerMeter * (*deltaTime);
		if (!a_moveUp)
			deltaZ *= -1;

		freeCam->translation.z += deltaZ;
	}
}



