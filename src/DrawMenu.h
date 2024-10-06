#pragma once

class DrawMenu : public RE::IMenu
{
public:
	constexpr static const char* MENU_PATH = "DrawMenu";
	constexpr static const char* MENU_NAME = "DrawMenu";


	void DrawPoint(RE::NiPoint2 a_position, float a_radius, uint32_t a_color, uint32_t a_alpha);
	void DrawLine(RE::NiPoint2 a_start, RE::NiPoint2 a_end, float a_startRadius, float a_endRadius, uint32_t a_color, uint32_t a_alpha);
	void DrawSquare(RE::NiPoint2 a_leftLowerCorner, RE::NiPoint2 a_leftUpperCorner, RE::NiPoint2 a_rightUpperCorner, RE::NiPoint2 a_rightLowerCorner, uint32_t a_color, uint32_t a_baseAlpha, uint32_t a_borderAlpha);
	
	float canvasWidth;
	float canvasHeight;


	RE::GPtr<RE::GFxMovieView> movie;
	
	DrawMenu()
	{
		auto menu = static_cast<RE::IMenu*>(this);
		menu->depthPriority = 0;
		auto scaleformManager = RE::BSScaleformManager::GetSingleton();

		const auto success = scaleformManager->LoadMovieEx(this, MENU_PATH, RE::GFxMovieView::ScaleModeType::kNoBorder, 0.0, [](RE::GFxMovieDef* a_def) -> void {
			a_def->SetState(RE::GFxState::StateType::kLog,
				RE::make_gptr<Logger>().get());
		});
		if (success) logger::info(" -LoadMovieEx successful");
		else logger::info(" -LoadMovieEx unsuccessful");

		movie = menu->uiMovie;
		canvasWidth = movie->GetVisibleFrameRect().right;
		canvasHeight = movie->GetVisibleFrameRect().bottom;

	}

	static RE::stl::owner<RE::IMenu*> Creator() { return new DrawMenu(); }

	static void Register()
	{
		auto ui = RE::UI::GetSingleton();
		if (ui) 
		{
			ui->Register(MENU_NAME, Creator);
			logger::info("Registered menu {}", MENU_NAME);
		}
	}

	static void OpenMenu()
	{
		if (auto UIMessageQueue = RE::UIMessageQueue::GetSingleton())
			UIMessageQueue->AddMessage(MENU_NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr);
	}

	static void CloseMenu()
	{
		if (auto UIMessageQueue = RE::UIMessageQueue::GetSingleton())
			UIMessageQueue->AddMessage(MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
			
	}

	RE::UI_MESSAGE_RESULTS ProcessMessage(RE::UIMessage& a_message)
	{
		if (a_message.type == RE::UI_MESSAGE_TYPE::kShow) 
		{
			auto ui = RE::UI::GetSingleton();
			auto g_drawMenu = ui ? ui->GetMenu<DrawMenu>(DrawMenu::MENU_NAME) : nullptr;
			
		}		

		return RE::UI_MESSAGE_RESULTS::kHandled;
	}

	

	void clearCanvas();

	private:
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

				logger::info("{}"sv, buf.data());
			}
		};
};

