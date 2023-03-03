#pragma once

#include <vren/vk_helpers/buffer.hpp>
#include <vren/vk_helpers/shader.hpp>
#include <vren/pipeline/render_graph.hpp>

#include "util/camera.hpp"

namespace explo
{
	// Forward decl
	class Renderer;

	/// A shader program that takes the circular-grid of the world view and the camera and filters out the chunks that are
	/// visible. The output of this program is a list of draw-call of the visible chunks.
	class CullWorldView
	{
	private:
		Renderer& m_renderer;

		vren::pipeline m_pipeline;

	public:
		explicit CullWorldView(Renderer& renderer);
		~CullWorldView();

		void record(VkCommandBuffer cmd_buf, vren::resource_container& res_container);

		vren::render_graph_t create_render_graph_node(vren::render_graph_allocator& allocator);

	private:
		vren::pipeline create_pipeline();
	};

} // namespace explo
