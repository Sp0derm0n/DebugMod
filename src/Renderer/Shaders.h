#pragma once

#include "D3DContext.h"
#include <winrt/base.h>


namespace Renderer
{
	namespace Shaders
	{
        typedef struct ShaderDecl 
        {
            uint8_t uid = 0;
            const char* source = nullptr;
        } ShaderDecl;

        constexpr ShaderDecl VertexColorScreenVS = {
            0,
            R"(
struct VS_INPUT 
{
	float4 vPos : POS;
	float4 vColor : COL;
};

struct VS_OUTPUT 
{
	float4 vPos : SV_POSITION;
	float4 vColor : COLOR0;
};

cbuffer PerFrame : register(b1) 
{
	float4x4 matProjView;
};

VS_OUTPUT main(VS_INPUT input) 
{
	float4 pos = float4(input.vPos.xyz, 1.0f);
	pos = mul(matProjView, pos);

	VS_OUTPUT output;
	output.vPos = pos;
	output.vColor = input.vColor;
	return output;
}
		)"};

        constexpr ShaderDecl VertexColorScreenPS = {
            1,
            R"(
struct PS_INPUT 
{
	float4 pos : SV_POSITION;
	float4 color : COLOR0;
};

struct PS_OUTPUT 
{
	float4 color : SV_Target;
};

PS_OUTPUT main(PS_INPUT input) 
{
	PS_OUTPUT output;
	output.color = input.color;
	return output;
}
		)"};
	

		constexpr ShaderDecl VertexColorWorldVS = {
			4,
			R"(
struct VS_INPUT {
	float3 vPos     : POS;
	float2 vUV      : UV;
	float3 vNormal  : NORMAL;
	float4 vColor   : COLOR;
};

struct VS_OUTPUT {
	float4 vPos     : SV_POSITION;
	float2 vUV      : COLOR0;
	float3 vNormal  : COLOR1;
	float4 vColor   : COLOR2;
};

cbuffer PerObject : register(b0) {
	float4x4 matModel;
};

cbuffer PerFrame : register(b1) {
	float4x4 matProjView;
};

VS_OUTPUT main(VS_INPUT input) {
	float4 pos = float4(input.vPos, 1.0f);
	pos = mul(matModel, pos);
	pos = mul(matProjView, pos);

	VS_OUTPUT output;
	output.vPos = pos;
	output.vUV = input.vUV;
	// If we ever use normals later, run thru inverse transpose first
	output.vNormal = input.vNormal;
	output.vColor = input.vColor;

	return output;
}
		)" };

		constexpr ShaderDecl VertexColorWorldPS = {
		5,
		R"(
struct PS_INPUT {
	float4 pos      : SV_POSITION;
	float2 uv       : COLOR0;
	float3 normal   : COLOR1;
	float4 color    : COLOR2;
};

struct PS_OUTPUT {
	float4 color : SV_Target;
};

cbuffer PerFrame : register(b1) {
	float4x4 matProjView;
	float4 tint;
	float curTime;
};

PS_OUTPUT main(PS_INPUT input) {
	PS_OUTPUT output;
	// scale tint.rgb by color.xyz so that black colors remain unchanged
	output.color = float4(lerp(input.color.xyz, tint.xyz * input.color.xyz, 0.5f), input.color.w);
	return output;
}
		)" };
	}

    enum class PipelineStage 
    {
        Vertex,
        Fragment,
    };

    struct ShaderCreateInfo 
    {
        Shaders::ShaderDecl source;
        std::string entryName = "main";
        std::string version;
        PipelineStage stage;

        ShaderCreateInfo(const Shaders::ShaderDecl& source, PipelineStage stage, std::string&& entryName = "main",
                         std::string&& version = "5_0")
            : source(source), entryName(entryName), version(version), stage(stage) {}

        size_t Hash() const 
        {
            size_t seed = source.uid;
            Utils::HashCombine(seed, entryName);
            Utils::HashCombine(seed, version);
            Utils::HashCombine(seed, stage);
            return seed;
        }

        bool operator==(const ShaderCreateInfo& other) const 
        {
            return stage == other.stage && version == other.version && entryName == other.entryName &&
                   source.uid == other.source.uid;
        }
    };

    class Shader {
        public:
            Shader(const ShaderCreateInfo& createInfo, D3DContext& ctx) noexcept;
            ~Shader() noexcept;
            Shader(const Shader&) = delete;
            Shader(Shader&&) noexcept = delete;
            Shader& operator=(const Shader&) = delete;
            Shader& operator=(Shader&&) noexcept = delete;

            // Use the shader for draw operations
            void Use() noexcept;
            // Returns true if the shader is valid
            bool IsValid() const noexcept;

    private:
        D3DContext context;
        winrt::com_ptr<ID3DBlob> binary;
        PipelineStage stage;
        bool validBinary = false;
        bool validProgram = false;

        // no com_ptr here - we is being bad
        union 
        {
            ID3D11VertexShader* vertex;
            ID3D11PixelShader* fragment;
        } program;

        bool Compile(const std::string& source, const std::string& entryName, const std::string& version) noexcept;

        friend class VertexBuffer;
    };

    class ShaderCache {
        public:
            ShaderCache(const ShaderCache&) = delete;
            ShaderCache(ShaderCache&&) noexcept = delete;
            ShaderCache& operator=(const ShaderCache&) = delete;
            ShaderCache& operator=(ShaderCache&&) noexcept = delete;

            static ShaderCache& Get() noexcept 
            {
                static ShaderCache cache;
                return cache;
            }

            void Release() noexcept;

            std::shared_ptr<Shader> Load(const ShaderCreateInfo& info, Renderer::D3DContext& ctx) noexcept;

            struct SCIHasher 
            {
                size_t operator()(const ShaderCreateInfo& key) const { return key.Hash(); }
            };

            struct SCICompare 
            {
                size_t operator()(const ShaderCreateInfo& k1, const ShaderCreateInfo& k2) const { return k1 == k2; }
            };

        private:
            ShaderCache() noexcept;

        private:
            std::unordered_map<ShaderCreateInfo, std::weak_ptr<Renderer::Shader>, SCIHasher, SCICompare> shaders;
    };

}