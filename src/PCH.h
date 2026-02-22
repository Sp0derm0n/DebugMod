#pragma once

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"
#include "RE.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/easing.hpp>
#include <glm/gtx/spline.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/type_trait.hpp>

#include <directxtk/BufferHelpers.h>
#include <directxtk/CommonStates.h>
#include <directxtk/DDSTextureLoader.h>
#include <directxtk/DirectXHelpers.h>
#include <directxtk/Effects.h>
#include <directxtk/GamePad.h>
#include <directxtk/GeometricPrimitive.h>
#include <directxtk/GraphicsMemory.h>
#include <directxtk/Keyboard.h>
#include <directxtk/Model.h>
#include <directxtk/Mouse.h>
#include <directxtk/PostProcess.h>
#include <directxtk/PrimitiveBatch.h>
#include <directxtk/ScreenGrab.h>
#include <directxtk/SimpleMath.h>
#include <directxtk/SpriteBatch.h>
#include <directxtk/SpriteFont.h>
#include <directxtk/VertexTypes.h>
#include <directxtk/WICTextureLoader.h>

#include <polyhook2/Virtuals/VFuncSwapHook.hpp>
#include <polyhook2/ZydisDisassembler.hpp>
#include <polyhook2/Detour/x64Detour.hpp>

#include "Version.h"

using namespace std::literals;

namespace logger = SKSE::log;

static inline void FatalError(const wchar_t* message) noexcept {
	ShowCursor(true);
	FatalAppExit(0, message);
}

static inline void WarningPopup(const wchar_t* message) noexcept {
	ShowCursor(true);
	MessageBox(nullptr, message, L"DebugMenu", MB_ICONERROR);
}

using vec2u = glm::vec<2, float, glm::highp>;
using vec3u = glm::vec<3, float, glm::highp>;
using vec4u = glm::vec<4, float, glm::highp>;