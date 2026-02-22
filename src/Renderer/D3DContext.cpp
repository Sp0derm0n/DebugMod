#include "D3DContext.h"

namespace Renderer
{
	void SetRasterState(D3DContext& ctx, D3D11_FILL_MODE fillMode, D3D11_CULL_MODE cullMode, bool frontCCW,
		int32_t depthBias, float depthBiasClamp, float slopeScaledDepthBias, bool lineAA,
		bool depthClip, bool scissorEnable, bool msaa) noexcept
	{
		D3D11_RASTERIZER_DESC desc;
		desc.FillMode = fillMode;
		desc.CullMode = cullMode;
		desc.FrontCounterClockwise = frontCCW;
		desc.DepthBias = depthBias;
		desc.DepthBiasClamp = depthBiasClamp;
		desc.SlopeScaledDepthBias = slopeScaledDepthBias;
		desc.DepthClipEnable = depthClip;
		desc.ScissorEnable = scissorEnable;
		desc.MultisampleEnable = msaa;
		desc.AntialiasedLineEnable = lineAA;
		auto key = RasterStateKey{ desc };

		auto it = d3dObjects.loadedRasterStates.find(key);
		if (it != d3dObjects.loadedRasterStates.end()) {
			ctx.context->RSSetState(it->second.state.get());
			return;
		}

		auto state = RasterState{ ctx, key };
		ctx.context->RSSetState(state.state.get());
		d3dObjects.loadedRasterStates.emplace(key, std::move(state));
	}

}