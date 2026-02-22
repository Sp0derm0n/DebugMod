#include "Drawer.h"
#include "Renderer.h"
#include "DrawHandler.h"
#include "D3DContext.h"
#include "DebugMenu/DebugMenu.h"



namespace Renderer 
{
    LineDrawer::LineDrawer(D3DContext& ctx) 
	{ 
		CreateObjects(ctx); 
	}

    LineDrawer::~LineDrawer() 
	{
        for (auto i = 0; i < NumBuffers; i++) vbo[i].reset();

        vs.reset();
        ps.reset();
    }

    void LineDrawer::CreateObjects(D3DContext& ctx) 
	{
        ShaderCreateInfo vsCreateInfo(Shaders::VertexColorScreenVS, PipelineStage::Vertex);
        vs = ShaderCache::Get().Load(vsCreateInfo, ctx);

        ShaderCreateInfo psCreateInfo(Shaders::VertexColorScreenPS, PipelineStage::Fragment);
        ps = ShaderCache::Get().Load(psCreateInfo, ctx);

        VertexBufferCreateInfo vbInfo;
        vbInfo.elementSize = sizeof(Point);
        vbInfo.numElements = LineDrawPointBatchSize * 2;
        vbInfo.topology = D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
        vbInfo.bufferUsage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
        vbInfo.cpuAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
        vbInfo.vertexProgram = vs;
        vbInfo.iaLayout.emplace_back(
            D3D11_INPUT_ELEMENT_DESC{"POS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0});
        vbInfo.iaLayout.emplace_back(D3D11_INPUT_ELEMENT_DESC{
            "COL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0});

        for (auto i = 0; i < NumBuffers; i++) vbo[i] = std::move(std::make_unique<VertexBuffer>(vbInfo, ctx));
    }

    void LineDrawer::Submit(const LineList& lines) noexcept {
        if (lines.size() > 0)
        {
            vs->Use();
            ps->Use();
        }


        auto begin = lines.cbegin();
        auto end = lines.cend();
        uint32_t batchCount = 0;

        while (begin != end) {
            DrawBatch(
                // Flip flop buffers to avoid pipeline stalls, if possible
                batchCount % static_cast<uint32_t>(NumBuffers), begin, end);
            batchCount++;
        }
    }

    void LineDrawer::DrawBatch(uint32_t bufferIndex, LineList::const_iterator& begin, LineList::const_iterator& end) {
        uint32_t batchSize = 0;
        uint32_t index = 0;
        auto buf = reinterpret_cast<glm::vec4*>(vbo[bufferIndex]->Map(D3D11_MAP::D3D11_MAP_WRITE_DISCARD).pData);

        while (begin != end) {
            buf[index] = begin->start.pos;
            buf[index + 1] = begin->start.col;
            buf[index + 2] = begin->end.pos;
            buf[index + 3] = begin->end.col;

            begin++;
            batchSize++;
            index += 4;

            if (batchSize >= LineDrawPointBatchSize) break;
        }
        vbo[bufferIndex]->Unmap();
        vbo[bufferIndex]->Bind();
        vbo[bufferIndex]->DrawCount(batchSize * 2);
    }

	static std::mutex renderLock;

	static VSMatricesCBuffer cbufPerFrameStaging = {};
	static std::shared_ptr<CBuffer> cbufPerFrame;

	static std::unique_ptr<LineDrawer> lineDrawer;
	static LineList lineList;
	static MeshList meshList;

	static VSPerObjectCBuffer cbufPerObjectStaging = {};
	static std::shared_ptr<CBuffer> cbufPerObject;

	static std::shared_ptr<Shader> meshVertexShader;
	static std::shared_ptr<Shader> meshPixelShader;

	std::shared_ptr<Shader> GetMeshVS()
	{
		return meshVertexShader;
	}
	std::shared_ptr<Shader> GetMeshPS()
	{
		return meshPixelShader;
	}

	std::shared_ptr<CBuffer> GetPerObjectCBuffer()
	{
		return cbufPerObject;
	}

    void InitDrawer() 
	{
        auto& ctx = GetContext();

        lineDrawer = std::make_unique<LineDrawer>(ctx);

		// Vertex and fragment programs
		Renderer::ShaderCreateInfo vsCreateInfo(Renderer::Shaders::VertexColorWorldVS, Renderer::PipelineStage::Vertex);
		Renderer::ShaderCreateInfo psCreateInfo(Renderer::Shaders::VertexColorWorldPS, Renderer::PipelineStage::Fragment);

		meshVertexShader = Renderer::ShaderCache::Get().Load(vsCreateInfo, ctx);
		meshPixelShader = Renderer::ShaderCache::Get().Load(psCreateInfo, ctx);

		Renderer::CBufferCreateInfo perObj;
		perObj.bufferUsage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
		perObj.cpuAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
		perObj.size = sizeof(decltype(cbufPerObjectStaging));
		perObj.initialData = &cbufPerObjectStaging;
		cbufPerObject = std::make_shared<Renderer::CBuffer>(perObj, ctx);

        CBufferCreateInfo perFrame;
        perFrame.bufferUsage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
        perFrame.cpuAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
        perFrame.size = sizeof(decltype(cbufPerFrameStaging));
        perFrame.initialData = &cbufPerFrameStaging;

        cbufPerFrame = std::make_shared<CBuffer>(perFrame, ctx);

        OnPresent([](D3DContext& a_ctx) 
		{
			std::lock_guard<std::mutex> lock(renderLock);

            auto& drawHandler = DebugMenu::GetDrawHandler();
			if (!drawHandler->isMenuOpen) return;
           
            for (int i = 0; i < 4; i++)
                for (int j = 0; j < 4; j++)
				{
					auto& projectionMatrix = drawHandler->GetProjectionMatrix();
					cbufPerFrameStaging.matProjView[i][j] = projectionMatrix(j, i); // Transpose since glm matrices are column major
				}
					

            cbufPerFrame->Update(&cbufPerFrameStaging, 0, sizeof(decltype(cbufPerFrameStaging)), a_ctx);

            cbufPerFrame->Bind(PipelineStage::Vertex, 1, a_ctx);
            cbufPerFrame->Bind(PipelineStage::Fragment, 1, a_ctx);

			Renderer::SetRasterState(a_ctx, D3D11_FILL_MODE::D3D11_FILL_WIREFRAME, 
									D3D11_CULL_MODE::D3D11_CULL_NONE, true, -2000, 0.0f, 0.0f, true,
									false, false, false);
				
			if (MCM::settings::collisionOcclude)
				Renderer::SetDepthState(a_ctx, true, true, D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL);
			
			// linelist and meshlist is cleared in drawhandler ClearAll() called in DebubMenu.cpp
            lineDrawer->Submit(lineList);
			for (auto& mesh : meshList)
			{
				mesh->Submit(glm::identity<glm::mat4>());
			}
        });
    }

    void DrawLine(const vec3u& a_point1, const vec3u& a_point2, vec4u& a_color)
    {
        std::lock_guard<std::mutex> lock(renderLock);
        lineList.emplace_back(Renderer::Point(a_point1, a_color),
                              Renderer::Point(a_point2, a_color));
    }

	void DrawMesh(std::shared_ptr<Renderer::MeshDrawer>& meshDrawer)
	{
		std::lock_guard<std::mutex> lock(renderLock);
		meshList.emplace_back(meshDrawer);
	}

	void ClearLines()
	{
		lineList.clear();
	}

	void ClearMeshes()
	{
		meshList.clear();
	}

}
