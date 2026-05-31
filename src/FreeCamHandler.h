#pragma once

class FreeCamHandler : public RE::BSTEventSink<RE::InputEvent*>
{
	using time_point = std::chrono::system_clock::time_point;
	public:
		
		static FreeCamHandler* GetSingleton()
		{
			static FreeCamHandler singleton;
			return std::addressof(singleton);
		}
		
		void						Init();
		RE::BSEventNotifyControl	ProcessEvent(RE::InputEvent* const* eventPtr, RE::BSTEventSource<RE::InputEvent*>*);
		bool						IsCustomFreeCamEnabled() { return isInCustomFreeCam; }
		void						PutPlayerBackOnGround();
		RE::FreeCameraState*		GetFreeCamera();
		void						ToggleTFC();
		void						OnEnterFreeCam();
		void						OnExitFreeCam();
		void						Update();
		void						PostUpdate();
		


	private:
		struct HotKey
		{
			const static uint32_t scrollUpKeyCode = 264;
			const static uint32_t scrollDownKeyCode = 265;
		};

		const float		unitsPerMeter = 70.0f;

		
		bool			isInCustomFreeCam = false;
		// Inputs
		bool			isAscendKeyDown = false;
		bool			isDescendKeyDown = false;
		float			doubleSpaceHighestDuration = 300; // milliseconds
		time_point		lastSpaceTimeStamp{}; // 1970-01-01

		float			defaultFreeCamTranslationSpeed = 20.0f;
		bool			defaultLockToZPlane = false;
		bool			defaultUseRunSpeed = false;

		float			speed = 11.0f; // m/s
		float			minSpeed = 1.0f;
		float			maxSpeed = 161.0f;
		float			speedIncrement = 2.5f;

		float			maxZSpeed = 60.0f;
		float			minZSpeed = 1.0f;

		float*			deltaTime = nullptr;
		float*			freeCamTranslationSpeed = nullptr;

		void			HandleInput(uint32_t a_keycode, bool a_isDown, float a_heldDuration);
		void			OnEnterCustomFreeCam();
		void			OnExitCustomFreeCam();
		void			ChangeSpeed(bool a_increase, bool a_isScrolling);
		void			IncreaseSpeed(bool a_isScrolling) { ChangeSpeed(true, a_isScrolling); }
		void			DecreaseSpeed(bool a_isScrolling) { ChangeSpeed(false, a_isScrolling); }
		void			SetPlayerAppCulled(bool a_cull);
		float			GetZSpeed();
		void			MoveCameraUpDown(bool a_moveUp);
		void			Ascend() { MoveCameraUpDown(true); }
		void			Descend() { MoveCameraUpDown(false); }
		void			UpdateCameraSpeed();
		void			UpdatePlayerPosition();
		RE::NiPoint3	GetCameraPosition();
};
