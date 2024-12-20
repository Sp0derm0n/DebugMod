#pragma once

class DrawMenu : public RE::IMenu
{
public:
	constexpr static const char* MENU_PATH = "DrawMenu";
	constexpr static const char* MENU_NAME = "DrawMenu";

	void DrawPoint(RE::NiPoint2 a_position, float a_radius, uint32_t a_color, uint32_t a_alpha);
	void DrawSimpleLine(RE::NiPoint2 a_start, RE::NiPoint2 a_end, float a_thickness, uint32_t a_color, uint32_t a_alpha);
	void DrawLine(RE::NiPoint2 a_start, RE::NiPoint2 a_end, float a_startRadius, float a_endRadius, uint32_t a_color, uint32_t a_alpha);
	void DrawTriangle(RE::NiPoint2 a_positions[3], uint32_t a_color, uint32_t a_baseAlpha, uint32_t a_borderAlpha);
	void DrawSquare(RE::NiPoint2 a_leftLowerCorner, RE::NiPoint2 a_leftUpperCorner, RE::NiPoint2 a_rightUpperCorner, RE::NiPoint2 a_rightLowerCorner, uint32_t a_color, uint32_t a_baseAlpha, uint32_t a_borderAlpha);
	void DrawPolygon(const std::vector<RE::NiPoint2>& a_positions, float a_borderThickness, uint32_t a_color, uint32_t a_baseAlpha, uint32_t a_borderAlpha);

	void GetInfoBox(RE::GFxValue& a_infoBox);
	void ShowInfoBox();
	void HideInfoBox();
	void SetInfoText(const std::string& a_text);
	void SetScroll(int32_t& a_scroll, bool a_scrollUp);

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
		if (!success) logger::info(" Draw menu: LoadMovieEx unsuccessful");

		menuFlags.set(RE::UI_MENU_FLAGS::kAllowSaving);

		movie = menu->uiMovie;
		canvasWidth = movie->GetVisibleFrameRect().right;
		canvasHeight = movie->GetVisibleFrameRect().bottom;
		menu->inputContext = Context::kNone;

	}


	static void Register();
	static void OpenMenu();
	static void CloseMenu();

	RE::UI_MESSAGE_RESULTS ProcessMessage(RE::UIMessage& a_message);

	static RE::stl::owner<RE::IMenu*> Creator() { return new DrawMenu(); }


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

