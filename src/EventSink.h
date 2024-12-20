#pragma once

class EventSink : 
	public RE::BSTEventSink<RE::InputEvent*>,
	public RE::BSTEventSink<RE::TESCellFullyLoadedEvent>
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
		bool key1HasBeenProcessed = false; // open debug menu
		bool key2HasBeenProcessed = false; // primary click
		bool key3HasBeenProcessed = false; // scroll up
		bool key4HasBeenProcessed = false; // scroll down


		RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* eventPtr, RE::BSTEventSource<RE::InputEvent*>*);
		RE::BSEventNotifyControl ProcessEvent(const RE::TESCellFullyLoadedEvent* a_event, RE::BSTEventSource<RE::TESCellFullyLoadedEvent>*);


	private:
		void HandleDebugMenu(bool a_isInputReleased);
		void HandlePrimaryClick(bool a_isInputReleased);
		void HandleScrollUp(bool a_isInputReleased);
		void HandleScrollDown(bool a_isInputReleased);
};
