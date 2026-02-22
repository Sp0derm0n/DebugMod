#pragma once

class EventSink : 
	public RE::BSTEventSink<RE::InputEvent*>
{
    EventSink() = default;
    EventSink(const EventSink&) = delete;
    EventSink(EventSink&&) = delete;
    EventSink& operator=(const EventSink&) = delete;
    EventSink& operator=(EventSink&&) = delete;

	

	public:
		static EventSink* GetSingleton() 
		{
			static EventSink singleton;
			return std::addressof(singleton);
		}


		uint32_t keycode2 = 256; // mouse primary click
		uint32_t keycode3 = 264; // mouse wheel up
		uint32_t keycode4 = 265; // mouse wheel down

		bool key1HasBeenProcessed = false; // open debug menu
		bool key2HasBeenProcessed = false; // primary click
		bool key3HasBeenProcessed = false; // scroll up hotkey (pgUP)
		bool key4HasBeenProcessed = false; // scroll down hotkey (pgDOWN)
		bool key5HasBeenProcessed = false; // scroll wheel up
		bool key6HasBeenProcessed = false; // scroll wheel down


		RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* eventPtr, RE::BSTEventSource<RE::InputEvent*>*);



	private:
		void HandleDebugMenu(bool a_isInputReleased);
		void HandlePrimaryClick(bool a_isInputReleased);
		void HandleScrollUp(bool a_isInputReleased);
		void HandleScrollDown(bool a_isInputReleased);
		void HandleScrollWheelUp(bool a_isInputReleased);
		void HandleScrollWheelDown(bool a_isInputReleased);
};
