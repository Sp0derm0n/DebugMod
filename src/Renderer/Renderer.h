#pragma once

#include "Drawer.h"
#include "MeshDrawer.h"

namespace Renderer
{
    using DrawFunc = std::function<void(D3DContext&)>;

    typedef HRESULT (*D3D11Present)(IDXGISwapChain*, UINT, UINT);
    typedef void (*CalledDuringRenderStartup)();
    typedef uintptr_t (*CalledDuringRenderShutdown)();

    static bool hookAttempted = false;
    static bool hookSuccess = false;
    static std::vector<Renderer::DrawFunc> presentCallbacks;
    static std::unique_ptr<TypedDetour<CalledDuringRenderStartup>> detCalledDuringRenderStartup;
    static std::unique_ptr<TypedDetour<CalledDuringRenderShutdown>> detCalledDuringRenderShutdown;

   


    void        SetDepthState(D3DContext& ctx, bool writeEnable, bool testEnable, D3D11_COMPARISON_FUNC testFunc) noexcept;
    void        OnPresent(DrawFunc&& callback) noexcept;
    HRESULT     Present(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags);
    void        InstallHooks();
    bool        AttachD3D();

    void mCalledDuringRenderStartup();
    uintptr_t mCalledDuringRenderShutdown();



}