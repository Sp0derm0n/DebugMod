#pragma once

class EventSink : 
	public RE::BSTEventSink<RE::InputEvent*>//,
	//public RE::BSTEventSink<RE::BSAnimationGraphEvent>
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

		uint32_t keycode1 = 2; // mouse middle click
		bool key1HasBeenProcessed = false;
		bool isDrawMenuOpen = false;

		RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* eventPtr, RE::BSTEventSource<RE::InputEvent*>*);

};
