#include "Shaders.h"
#include <d3dcompiler.h>


namespace Renderer
{
    ShaderCache::ShaderCache() noexcept {};

    
void ShaderCache::Release() noexcept { shaders.clear(); }

    Shader::Shader(const ShaderCreateInfo& createInfo, D3DContext& ctx) noexcept
        : stage(createInfo.stage), context(ctx) 
	{
        validBinary = Compile(createInfo.source.source, createInfo.entryName, createInfo.version);
        if (!validBinary) return;
        if (stage == PipelineStage::Vertex) 
		{
            const auto result = context.device->CreateVertexShader(binary->GetBufferPointer(), binary->GetBufferSize(), nullptr, &program.vertex);
			validProgram = SUCCEEDED(result);
            if (!validProgram) 
				FatalError(L"DebugMenu: A shader failed to compile");
        } 
		else 
		{
            const auto result = context.device->CreatePixelShader(binary->GetBufferPointer(), binary->GetBufferSize(), nullptr, &program.fragment);
            validProgram = SUCCEEDED(result);
            if (!validProgram) 
				FatalError(L"DebugMenu: A shader failed to compile");
		}
    }

    Shader::~Shader() noexcept {
        if (validProgram) {
            switch (stage) {
                case PipelineStage::Vertex:
                    program.vertex->Release();
                    break;
                case PipelineStage::Fragment:
                    program.fragment->Release();
                    break;
            }
        }
        validProgram = false;
    }

    void Shader::Use() noexcept {
        if (stage == PipelineStage::Vertex) {
            context.context->VSSetShader(program.vertex, nullptr, 0);
        } else {
            context.context->PSSetShader(program.fragment, nullptr, 0);
        }
    }

    bool Shader::Compile(const std::string& source, const std::string& entryName,
                                 const std::string& version) noexcept {
        constexpr UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR;
        winrt::com_ptr<ID3DBlob> errorBlob;

        auto versionStr =
            stage == PipelineStage::Vertex ? std::string("vs_").append(version) : std::string("ps_").append(version);

        const auto result = D3DCompile(source.c_str(), source.length(), nullptr, nullptr, nullptr, entryName.c_str(),
                                       versionStr.c_str(), compileFlags, 0, binary.put(), errorBlob.put());

        if (!SUCCEEDED(result)) {
            if (errorBlob) {
                logger::debug("Shader compilation failed; {}", static_cast<const char*>(errorBlob->GetBufferPointer()));
            }
            return false;
        }
        return true;
    }

    bool Shader::IsValid() const noexcept { return validProgram && validBinary; }

    std::shared_ptr<Shader> ShaderCache::Load(const ShaderCreateInfo& info, D3DContext& ctx) noexcept 
	{
        const auto it = shaders.find(info);
        if (it != shaders.end()) {
            if (auto ptr = it->second.lock(); ptr != nullptr) {
                return ptr;
            } else {
                shaders.erase(it);
            }
        }

        auto ptr = std::make_shared<Shader>(info, ctx);
        if (ptr->IsValid()) shaders.insert({info, ptr});
        return ptr;
    }

}
