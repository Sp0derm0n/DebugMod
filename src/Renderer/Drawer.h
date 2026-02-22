#pragma once

#include "VertexBuffer.h"
#include "MeshDrawer.h"
#include "CBuffer.h"

namespace Renderer
{
   
    struct VSMatricesCBuffer 
    {
        glm::mat4 matProjView = glm::identity<glm::mat4>();
        glm::vec4 tint = {1.0f, 1.0f, 1.0f, 1.0f};
        float curTime = 0.0f;
        float pad[3] = {0.0f, 0.0f, 0.0f};
    };
    static_assert(sizeof(VSMatricesCBuffer) % 16 == 0);

	struct VSPerObjectCBuffer 
	{
		glm::mat4 model = glm::identity<glm::mat4>();
	};
	static_assert(sizeof(VSPerObjectCBuffer) % 16 == 0);

    typedef struct Point 
    {
        glm::vec4 pos;
        glm::vec4 col;

        Point(glm::vec3 position, glm::vec4 color) : col(color) { pos = {position.x, position.y, position.z, 1.0f}; }
    } Point;

    typedef struct Line 
    {
        Point start;
        Point end;
        Line(Point&& start, Point&& end) : start(start), end(end) {};
    } Line;

    using LineList = std::vector<Line>;
	using MeshList = std::vector<std::shared_ptr<MeshDrawer>>;

    // Number of points we can submit in a single draw call
    constexpr size_t LineDrawPointBatchSize = 256;
    // Number of buffers to use
    constexpr size_t NumBuffers = 2;

    class LineDrawer 
    {
        public:
            explicit LineDrawer(D3DContext& ctx);
            ~LineDrawer();
            LineDrawer(const LineDrawer&) = delete;
            LineDrawer(LineDrawer&&) noexcept = delete;
            LineDrawer& operator=(const LineDrawer&) = delete;
            LineDrawer& operator=(LineDrawer&&) noexcept = delete;

            // Submit a list of lines for drawing
            void Submit(const LineList& lines) noexcept;

        protected:
            std::shared_ptr<Shader> vs;
            std::shared_ptr<Shader> ps;

        private:
            std::array<std::unique_ptr<VertexBuffer>, NumBuffers> vbo;

            void CreateObjects(D3DContext& ctx);
            void DrawBatch(uint32_t bufferIndex, LineList::const_iterator& begin, LineList::const_iterator& end);
    };

    static constexpr float RenderScale = 1.0f;//0.0142875f;

    void InitDrawer();
    void DrawLine(const vec3u& a_point1, const vec3u& a_point2, vec4u& a_color);
	void DrawMesh(std::shared_ptr<MeshDrawer>& meshDrawer);
    
	void ClearLines();
	void ClearMeshes();

	std::shared_ptr<Shader> GetMeshVS();
	std::shared_ptr<Shader> GetMeshPS();
	std::shared_ptr<CBuffer> GetPerObjectCBuffer();

    __forceinline glm::vec3 ToRenderScale(const glm::vec3& position) noexcept { return position * RenderScale; }
    __forceinline glm::vec3 FromRenderScale(const glm::vec3& position) noexcept { return position / RenderScale; }

}