#pragma once

#include "Menu.h"
#include "AnimationHandler.h"

namespace ScaleformUI
{
	class InputHandler : public RE::BSTEventSink<RE::InputEvent*>
	{
		public:

			static InputHandler* GetSingleton()
			{
				static InputHandler singleton;
				return std::addressof(singleton);
			}

			RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* a_eventPtr, RE::BSTEventSource<RE::InputEvent*>*);

			void Init();
			void AddMenu(Menu* a_menu);
			bool IsInitialized();
			void SetScrollUpKey(uint32_t a_keycode);
			void SetScrollDownKey(uint32_t a_keycode);


		private:
			friend class Hook_MainUpdate;

			enum class ButtonName
			{
				kNone = 0,
				kPrimary = 1,
				kSecondary = 2,
				kMouseWheelDown = 3,
				kMouseWheelUp = 4,
				kScrollDownKey = 5, 
				kScrollUpKey = 6
			};

			struct ButtonEvent
			{
				
				uint32_t	keyCode = 0;
				float		duration = 0.0f;
				bool		active = false;
				bool		isDown = false;

				ButtonEvent() {}
				ButtonEvent(uint32_t a_keyCode) : keyCode(a_keyCode) {}
				void Reset() { active = false; isDown = false; duration = 0.0f; }
			};

			using UITask = std::function<void()>;
			
			std::vector<Menu*>	menus; // InputHandler does not own the menus


			bool								initialized = false;
			float								durationWithNoMouseInput = 0.0f;
			float*								deltaTime = nullptr;
			float								keyScrollDistance = 0.2f; // ratio of screen height
			float								mouseWheelScrollDistance = 0.05f; // ratio of screen height
			std::map<ButtonName, ButtonEvent>	buttonEvents{
				{ ButtonName::kPrimary,			256 },
				{ ButtonName::kScrollDownKey,	0 },
				{ ButtonName::kScrollUpKey,		0 },
				{ ButtonName::kMouseWheelDown,	264 },
				{ ButtonName::kMouseWheelUp,	265 }};

			void	ResetKeys();
			void	Update();
			void	ProcessInputs();
			void	UpdateMenus();
			Size	GetScrollDistance();
	};

	class Hook_MainUpdate
	{
		public:
			static void Install()
			{
				logger::debug("Hook: MainUpdate");

				auto& trampoline = SKSE::GetTrampoline();
				REL::Relocation<uintptr_t> hook{ RELOCATION_ID(35551, 36544), REL::VariantOffset(0x11F, 0x160, 0x160) };  // main loop
				_Update = trampoline.write_call<5>(hook.address(), Update);
			}

		private:
			static void Update(RE::Main* a_this, float a_2)
			{
				_Update(a_this, a_2);

				InputHandler::GetSingleton()->Update();
			}
			static inline REL::Relocation<decltype(Update)> _Update;
	};
}