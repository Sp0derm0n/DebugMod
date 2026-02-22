#include "Renderer.h"

namespace Renderer
{
    void SetDepthState(D3DContext& ctx, bool writeEnable, bool testEnable, D3D11_COMPARISON_FUNC testFunc) noexcept 
    {
        auto key = DSStateKey{writeEnable, testEnable, testFunc};
        auto it = d3dObjects.loadedDepthStates.find(key);
        if (it != d3dObjects.loadedDepthStates.end()) 
        {
            ctx.context->OMSetDepthStencilState(it->second.state.get(), 255);
            return;
        }

        auto state = DSState{ctx, key};
        ctx.context->OMSetDepthStencilState(state.state.get(), 255);
        d3dObjects.loadedDepthStates.emplace(key, std::move(state));
    }

    void OnPresent(DrawFunc&& callback) noexcept 
    { 
        presentCallbacks.emplace_back(callback);
    }

	D3DContext gameContext;

    static bool ReadSwapChain() 
	{
        // Hopefully SEH will shield us from unexpected crashes with other mods/programs
        // @Note: no MiniDumpScope here, we allow for this to fail and simply disable D3D features
        __try 
		{
            auto data = *REL::Relocation<D3D11Resources**>(RELOCATION_ID(524728, 411347).address());
            // Naked pointer to the swap chain
            gameContext.swapChain = data->swapChain;
            // Device
            gameContext.device.copy_from(data->device);
            // Context
            gameContext.context.copy_from(data->ctx);

            // Try and read the desc as a simple test
            DXGI_SWAP_CHAIN_DESC desc;
            if (!SUCCEEDED(data->swapChain->GetDesc(&desc))) {
                return false;
            }

            gameContext.windowSize.x = static_cast<float>(desc.BufferDesc.Width /*data->windowW*/);
            gameContext.windowSize.y = static_cast<float>(desc.BufferDesc.Height /*data->windowH*/);
            gameContext.hWnd = data->window;
        } __except (1) {
            return false;
        }

        return true;
    }

    void InstallHooks() 
	{
        if (hookAttempted) return;
        hookAttempted = true;

        if (!ReadSwapChain()) 
		{
            logger::debug("ERROR: Failed to hook IDXGISwapChain::Present and aquire device context, D3D drawing is disabled.");
            return;
        }

        auto ctx = GetContext();
        dxgiHook = std::make_unique<VTableDetour<IDXGISwapChain>>(ctx.swapChain);

        dxgiHook->Add(8, Present);
        if (!dxgiHook->Attach()) 
        {
            logger::debug("ERROR: Failed to place detour on virtual IDXGISwapChain->Present.");
            return;
        }

        hookSuccess = true;
    }

   
    void mCalledDuringRenderStartup();
    uintptr_t mCalledDuringRenderShutdown();


    void mCalledDuringRenderStartup() 
	{
        if (hookAttempted) return;

		logger::debug("Hooking D3D11");
        InstallHooks();
        if (!hookSuccess)
            WarningPopup(
                L"DebugMenu: Failed to hook DirectX. Try running with overlay "
                "software disabled if this warning keeps occurring or disable D3D11 in MCM.");

        if (hookSuccess) 
            InitDrawer();

        detCalledDuringRenderStartup->GetBase()();
    }


    static uintptr_t mCalledDuringRenderShutdown() 
	{
        logger::debug("Shutting down the rendering subsystem");
        return detCalledDuringRenderShutdown->GetBase()();
    }

    bool AttachD3D() 
	{
        detCalledDuringRenderStartup =
            std::make_unique<TypedDetour<CalledDuringRenderStartup>>(RELOCATION_ID(75827, 77649).id(), mCalledDuringRenderStartup);
        if (!detCalledDuringRenderStartup->Attach()) 
		{
            logger::debug("ERROR: Failed to place detour on target function(Render Startup), this error is fatal.");
            return false;
        }

        detCalledDuringRenderShutdown =
            std::make_unique<TypedDetour<CalledDuringRenderShutdown>>(RELOCATION_ID(75446, 77227).id(), mCalledDuringRenderShutdown);
        if (!detCalledDuringRenderShutdown->Attach()) 
		{
            logger::debug("ERROR: Failed to place detour on target function(Render Shutdown), this error is fatal.");
            return false;
        }


        return true;
    }

    HRESULT Present(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags) 
	{
        {
            //  Save some context state to restore later
            winrt::com_ptr<ID3D11DepthStencilState> gameDSState;
            uint32_t gameStencilRef;
            gameContext.context->OMGetDepthStencilState(gameDSState.put(), &gameStencilRef);

            winrt::com_ptr<ID3D11BlendState> gameBlendState;
            float gameBlendFactors[4];
            uint32_t gameSampleMask;
            gameContext.context->OMGetBlendState(gameBlendState.put(), gameBlendFactors, &gameSampleMask);

            D3D11_VIEWPORT gamePort;
            uint32_t numPorts = 1;
            gameContext.context->RSGetViewports(&numPorts, &gamePort);

            D3D11_VIEWPORT port;
            port.TopLeftX = gamePort.TopLeftX;
            port.TopLeftY = gamePort.TopLeftY;
            port.Width = gamePort.Width;
            port.Height = gamePort.Height;
            port.MinDepth = 0;
            port.MaxDepth = 1;
            gameContext.context->RSSetViewports(1, &port);

            winrt::com_ptr<ID3D11RasterizerState> rasterState;
            gameContext.context->RSGetState(rasterState.put());

			

            gameContext.context->OMGetRenderTargets(1, d3dObjects.gameRTV.put(), d3dObjects.depthStencilView.put());

            {
                for (auto& callback : presentCallbacks) 
				{
                    callback(gameContext);
                }
                auto color = d3dObjects.gameRTV.get();
                gameContext.context->OMSetRenderTargets(1, &color, d3dObjects.depthStencilView.get());

            }
            // Put things back the way we found it
            gameContext.context->RSSetState(rasterState.get());
            gameContext.context->RSSetViewports(1, &gamePort);
            gameContext.context->OMSetBlendState(gameBlendState.get(), gameBlendFactors, gameSampleMask);
            gameContext.context->OMSetDepthStencilState(gameDSState.get(), gameStencilRef);

            d3dObjects.depthStencilView = nullptr;
            d3dObjects.gameRTV = nullptr;
        }
        return dxgiHook->GetBase<D3D11Present>(8)(swapChain, syncInterval, flags);
    }
}

