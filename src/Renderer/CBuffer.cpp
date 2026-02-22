#include "CBuffer.h"

namespace Renderer
{
    CBuffer::CBuffer(CBufferCreateInfo & info, D3DContext & ctx) 
    {
        D3D11_BUFFER_DESC desc;
        desc.ByteWidth = static_cast<UINT>(info.size);
        desc.Usage = info.bufferUsage;
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = info.cpuAccessFlags;
        desc.MiscFlags = 0;
        desc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA init;
        init.pSysMem = info.initialData;
        init.SysMemPitch = 0;
        init.SysMemSlicePitch = 0;

        size = info.size;
        usage = info.bufferUsage;

        if (!SUCCEEDED(ctx.device->CreateBuffer(&desc, &init, buffer.put())))
            FatalError(L"DebugMenu: Failed to create D3D cbuffer.");
    }

    void CBuffer::Update(const void* newData, size_t offset, size_t bufSize, D3DContext& ctx) 
    {
        D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};

        if (!SUCCEEDED(ctx.context->Map(buffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer)))
            FatalError(L"DebugMenu: Failed to map cbuffer resource");

        auto start = reinterpret_cast<intptr_t>(mappedBuffer.pData) + offset;
        memcpy(reinterpret_cast<void*>(start), newData, bufSize);
        ctx.context->Unmap(buffer.get(), 0);
    }

    void CBuffer::Bind(PipelineStage stage, uint8_t loc, D3DContext& ctx) 
    {
        const auto buf = buffer.get();
        switch (stage) {
            case PipelineStage::Vertex:
                ctx.context->VSSetConstantBuffers(loc, 1, &buf);
                break;
            case PipelineStage::Fragment:
                ctx.context->PSSetConstantBuffers(loc, 1, &buf);
                break;
        }
    }

    size_t CBuffer::Size() const noexcept { return size; }

    D3D11_USAGE CBuffer::Usage() const noexcept { return usage; }

    winrt::com_ptr<ID3D11Buffer>& CBuffer::GetBuffer() noexcept { return buffer; }
}
