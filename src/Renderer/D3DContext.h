#pragma once

#include "Utils.h"
#include "BasicDetour.h"
#include <winrt/base.h>

namespace Renderer
{
    struct D3DContext 
	{
        // @Note: we don't want to refCount the swap chain - Let skyrim manage the lifetime.
        // As long as the game is running, we have a valid swapchain.
        IDXGISwapChain* swapChain = nullptr;
        winrt::com_ptr<ID3D11Device> device = nullptr;
        winrt::com_ptr<ID3D11DeviceContext> context;
        // Size of the output window in pixels
        glm::vec2 windowSize = {};
        HWND hWnd = nullptr;
    };

    

    typedef struct DSStateKey 
	{
        bool write;
        bool test;
        D3D11_COMPARISON_FUNC mode;

        DSStateKey(bool write, bool test, D3D11_COMPARISON_FUNC mode) : write(write), test(test), mode(mode) {}

        size_t Hash() const 
		{
            size_t seed = 0;
            Utils::HashCombine(seed, write);
            Utils::HashCombine(seed, test);
            Utils::HashCombine(seed, mode);
            return seed;
        }

        bool operator==(const DSStateKey& other) const 
		{
            return write == other.write && test == other.test && mode == other.mode;
        }
    } DSStateKey;

    typedef struct DSState 
	{
        winrt::com_ptr<ID3D11DepthStencilState> state;

        DSState(Renderer::D3DContext& ctx, DSStateKey& info) 
		{
            D3D11_DEPTH_STENCIL_DESC dsDesc;
            dsDesc.DepthEnable = info.write;
            dsDesc.DepthWriteMask = info.test ? D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ALL
                                              : D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ZERO;
            dsDesc.DepthFunc = info.mode;
            dsDesc.StencilEnable = false;
            ctx.device->CreateDepthStencilState(&dsDesc, state.put());
        }

        ~DSState() {}

        DSState(const DSState&) = delete;
        DSState(DSState&& loc) noexcept
		{
            // Doing this, we don't do extra ref counting
            state.attach(loc.state.get());
            loc.state.detach();
        };
        DSState& operator=(const DSState&) = delete;
        DSState& operator=(DSState&& loc) noexcept 
		{
            // Doing this, we don't do extra ref counting
            state.attach(loc.state.get());
            loc.state.detach();
        };
    } DSState;

    struct DSHasher 
	{
        size_t operator()(const DSStateKey& key) const { return key.Hash(); }
    };

    struct DSCompare 
	{
        size_t operator()(const DSStateKey& k1, const DSStateKey& k2) const { return k1 == k2; }
    };

    typedef struct BlendStateKey 
	{
        D3D11_BLEND_DESC desc;
        float factors[4] = {1.0f, 1.0f, 1.0f, 1.0f};

        BlendStateKey() { ZeroMemory(&desc, sizeof(D3D11_BLEND_DESC)); }

        size_t HashRTBlendDesc(const D3D11_RENDER_TARGET_BLEND_DESC& rtDesc) const 
		{
            size_t seed = 0;
            Utils::HashCombine(seed, rtDesc.BlendEnable);
            Utils::HashCombine(seed, rtDesc.SrcBlend);
            Utils::HashCombine(seed, rtDesc.DestBlend);
            Utils::HashCombine(seed, rtDesc.SrcBlendAlpha);
            Utils::HashCombine(seed, rtDesc.DestBlendAlpha);
            Utils::HashCombine(seed, rtDesc.BlendOp);
            Utils::HashCombine(seed, rtDesc.BlendOpAlpha);
            Utils::HashCombine(seed, rtDesc.RenderTargetWriteMask);
            return seed;
        }

        size_t Hash() const 
		{
            size_t seed = 0;
            Utils::HashCombine(seed, desc.AlphaToCoverageEnable);
            Utils::HashCombine(seed, desc.IndependentBlendEnable);
            Utils::HashCombine(seed, HashRTBlendDesc(desc.RenderTarget[0]));  // @NOTE: We only work with the main RT right now
            return seed;
        }

        bool RTBlendDescEq(const D3D11_RENDER_TARGET_BLEND_DESC& other) const 
		{
            return desc.RenderTarget[0].BlendEnable == other.BlendEnable &&
                   desc.RenderTarget[0].BlendOp == other.BlendOp &&
                   desc.RenderTarget[0].BlendOpAlpha == other.BlendOpAlpha &&
                   desc.RenderTarget[0].DestBlend == other.DestBlend &&
                   desc.RenderTarget[0].DestBlendAlpha == other.DestBlendAlpha &&
                   desc.RenderTarget[0].RenderTargetWriteMask == other.RenderTargetWriteMask &&
                   desc.RenderTarget[0].SrcBlend == other.SrcBlend &&
                   desc.RenderTarget[0].SrcBlendAlpha == other.SrcBlendAlpha;
        }

        bool operator==(const BlendStateKey& other) const 
		{
            return desc.AlphaToCoverageEnable == other.desc.AlphaToCoverageEnable &&
                   desc.IndependentBlendEnable == other.desc.IndependentBlendEnable &&
                   RTBlendDescEq(other.desc.RenderTarget[0]);
        }
    } BlendStateKey;

    struct BlendStateHasher 
	{
        size_t operator()(const BlendStateKey& key) const { return key.Hash(); }
    };

    struct BlendStateCompare 
	{
        size_t operator()(const BlendStateKey& k1, const BlendStateKey& k2) const { return k1 == k2; }
    };

    typedef struct BlendState 
	{
        winrt::com_ptr<ID3D11BlendState> state;

        BlendState(Renderer::D3DContext& ctx, BlendStateKey& info) 
		{
            ctx.device->CreateBlendState(&info.desc, state.put());
        }

        ~BlendState() {}

        BlendState(const BlendState&) = delete;
        BlendState(BlendState&& loc) noexcept 
		{
            // Doing this, we don't do extra ref counting
            state.attach(loc.state.get());
            loc.state.detach();
        };
        BlendState& operator=(const BlendState&) = delete;
        BlendState& operator=(BlendState&& loc) noexcept 
		{
            // Doing this, we don't do extra ref counting
            state.attach(loc.state.get());
            loc.state.detach();
        }
    } BlendState;

    typedef struct RasterStateKey 
	{
        D3D11_RASTERIZER_DESC desc;

        RasterStateKey(D3D11_RASTERIZER_DESC desc) : desc(desc) {}

        size_t Hash() const 
		{
            size_t seed = 0;
            Utils::HashCombine(seed, desc.FillMode);
            Utils::HashCombine(seed, desc.CullMode);
            Utils::HashCombine(seed, desc.FrontCounterClockwise);
            Utils::HashCombine(seed, desc.DepthBias);
            Utils::HashCombine(seed, desc.DepthBiasClamp);
            Utils::HashCombine(seed, desc.SlopeScaledDepthBias);
            Utils::HashCombine(seed, desc.DepthClipEnable);
            Utils::HashCombine(seed, desc.ScissorEnable);
            Utils::HashCombine(seed, desc.MultisampleEnable);
            Utils::HashCombine(seed, desc.AntialiasedLineEnable);
            return seed;
        }

        bool operator==(const RasterStateKey& other) const 
		{
            return desc.FillMode == other.desc.FillMode && desc.CullMode == other.desc.CullMode &&
                   desc.FrontCounterClockwise == other.desc.FrontCounterClockwise &&
                   desc.DepthBias == other.desc.DepthBias && desc.DepthBiasClamp == other.desc.DepthBiasClamp &&
                   desc.DepthClipEnable == other.desc.DepthClipEnable &&
                   desc.ScissorEnable == other.desc.ScissorEnable &&
                   desc.MultisampleEnable == other.desc.MultisampleEnable &&
                   desc.AntialiasedLineEnable == other.desc.AntialiasedLineEnable;
        }
    } RasterStateKey;

    typedef struct RasterState 
	{
        winrt::com_ptr<ID3D11RasterizerState> state;

        RasterState(Renderer::D3DContext& ctx, RasterStateKey& info) 
		{
            ctx.device->CreateRasterizerState(&info.desc, state.put());
        }

        ~RasterState() {}

        RasterState(const RasterState&) = delete;
        RasterState(RasterState&& loc) noexcept {
            // Doing this, we don't do extra ref counting
            state.attach(loc.state.get());
            loc.state.detach();
        };
        RasterState& operator=(const RasterState&) = delete;
        RasterState& operator=(RasterState&& loc) noexcept {
            // Doing this, we don't do extra ref counting
            state.attach(loc.state.get());
            loc.state.detach();
        };
    } RasterState;

	

    struct RasterStateHasher 
	{
        size_t operator()(const RasterStateKey& key) const { return key.Hash(); }
    };

    struct RasterStateCompare 
	{
        size_t operator()(const RasterStateKey& k1, const RasterStateKey& k2) const { return k1 == k2; }
    };

    struct D3DObjectsStore 
	{
        winrt::com_ptr<ID3D11DepthStencilView> depthStencilView;
        winrt::com_ptr<ID3D11RenderTargetView> gameRTV;

        std::unordered_map<DSStateKey, DSState, DSHasher, DSCompare> loadedDepthStates;

        std::unordered_map<BlendStateKey, BlendState, BlendStateHasher, BlendStateCompare> loadedBlendStates;

        std::unordered_map<RasterStateKey, RasterState, RasterStateHasher, RasterStateCompare> loadedRasterStates;

        void release() 
		{
            depthStencilView = nullptr;
            gameRTV = nullptr;
            loadedRasterStates.clear();
            loadedDepthStates.clear();
            loadedBlendStates.clear();
        }
    };

    static D3DObjectsStore d3dObjects;
    static std::unique_ptr<VTableDetour<IDXGISwapChain>> dxgiHook;

    struct D3D11Resources 
	{
        int32_t unk0;                        // 0x0
        int32_t unk4;                        // 0x4
        int32_t unk8;                        // 0x8
        int32_t unkC;                        // 0xC
        int32_t unk10;                       // 0x10
        int32_t unk14;                       // 0x14
        int32_t unk18;                       // 0x18
        int32_t unk1C;                       // 0x1C
        uintptr_t unk20;                     // 0x20
        uintptr_t unk28;                     // 0x28
        uintptr_t unk30;                     // 0x30
        ID3D11Device* device;                // 0x38
        ID3D11DeviceContext* ctx;            // 0x40
        HWND window;                         // 0x48
        DWORD windowX;                       // 0x50
        DWORD windowY;                       // 0x54
        DWORD windowW;                       // 0x58
        DWORD windowH;                       // 0x5C
        IDXGISwapChain* swapChain;           // 0x60
        uintptr_t unk68;                     // 0x68
        uintptr_t unk70;                     // 0x70
        ID3D11RenderTargetView* unkRTV78;    // 0x78
        ID3D11ShaderResourceView* unkSRV80;  // 0x80
        uintptr_t unk88;                     // 0x88
        uintptr_t unk90;                     // 0x90
        uintptr_t unk98;                     // 0x98
        uintptr_t unkA0;                     // 0xA0
        uintptr_t unkA8;                     // 0xA8
        uintptr_t unkB0;                     // 0xB0
    };
    static_assert(offsetof(D3D11Resources, unkC) == 0xC);
    static_assert(offsetof(D3D11Resources, unk1C) == 0x1C);
    static_assert(offsetof(D3D11Resources, device) == 0x38);
    static_assert(offsetof(D3D11Resources, swapChain) == 0x60);
    static_assert(offsetof(D3D11Resources, unkB0) == 0xB0);

	void SetRasterState(D3DContext& ctx, D3D11_FILL_MODE fillMode, D3D11_CULL_MODE cullMode, bool frontCCW,
		int32_t depthBias = 0, float depthBiasClamp = 0.0f, float slopeScaledDepthBias = 0.0f, bool lineAA = true,
		bool depthClip = false, bool scissorEnable = false, bool msaa = false) noexcept;

    extern D3DContext gameContext;

    static D3DContext& GetContext() noexcept { return gameContext; };


}