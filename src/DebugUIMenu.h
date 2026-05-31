#pragma once

#include "MCM.h"

class DebugMenuUI : public RE::IMenu
{
	public:
		constexpr static const char* MENU_PATH = "DebugMenu";
		constexpr static const char* MENU_NAME = "DebugMenu";

		RE::GPtr<RE::GFxMovieView> movie;

		DebugMenuUI();

		static void Register();

		RE::UI_MESSAGE_RESULTS ProcessMessage(RE::UIMessage& a_message);
		
		static RE::stl::owner<RE::IMenu*> Creator() { return new DebugMenuUI(); }


	private:
		uint32_t currentMarkerPresetIndex = 1;

		class Logger : public RE::GFxLog
		{
			public:
			void LogMessageVarg(LogMessageType, const char* a_fmt, std::va_list a_argList) override
			{
				std::string fmt(a_fmt ? a_fmt : "");
				while (!fmt.empty() && fmt.back() == '\n') {
					fmt.pop_back();
				}

				std::va_list args;
				va_copy(args, a_argList);
				std::vector<char> buf(static_cast<std::size_t>(std::vsnprintf(0, 0, fmt.c_str(), a_argList) + 1));
				std::vsnprintf(buf.data(), buf.size(), fmt.c_str(), args);
				va_end(args);

				logger::debug("\n{}"sv, buf.data());
			}
		};


};

