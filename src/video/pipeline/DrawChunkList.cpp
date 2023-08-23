#include "DrawChunkList.hpp"

#include "video/Renderer.hpp"

using namespace explo;

DrawChunkList::DrawChunkList(Renderer& renderer) :
	m_renderer(renderer),
	m_basic_renderer(m_renderer.m_context)
{
}

DrawChunkList::~DrawChunkList()
{
}

void DrawChunkList::record(VkCommandBuffer cmd_buf, vren::resource_container& resource_container)
{
	auto vertex_buffer = m_renderer.m_baked_world_view->m_vertex_buffer.get_buffer();
	auto index_buffer = m_renderer.m_baked_world_view->m_index_buffer.get_buffer();
	auto instance_buffer = m_renderer.m_baked_world_view->m_instance_buffer.get_buffer();

	glm::uvec2 fb_size = m_renderer.get_framebuffer_size();
	m_basic_renderer.make_rendering_scope(cmd_buf, fb_size, *m_renderer.m_gbuffer, *m_renderer.m_depth_buffer, [&]()
	{
		m_basic_renderer.set_viewport(cmd_buf, fb_size.x, fb_size.y);
		m_basic_renderer.set_scissor(cmd_buf, fb_size.x, fb_size.y);

		m_basic_renderer.set_vertex_buffer(cmd_buf, *vertex_buffer);
		m_basic_renderer.set_index_buffer(cmd_buf, *index_buffer);
		m_basic_renderer.set_instance_buffer(cmd_buf, *instance_buffer);

		m_basic_renderer.set_push_constants(cmd_buf, vren::basic_renderer::push_constants{
			.m_camera_view = {}, // TODO
			.m_camera_projection = {}, // TODO
		});

		vkCmdDrawIndexedIndirectCount(
			cmd_buf,
			m_renderer.m_chunk_draw_list.m_buffer.m_handle, 0, // Buffer
			m_renderer.m_chunk_draw_list_idx.m_buffer.m_handle, 0, // Count buffer
			274625 /* 65^3, derived from max render distance */,
			sizeof(VkDrawIndexedIndirectCommand) // stride
			);
	});

	resource_container.add_resources(
		m_renderer.m_gbuffer,
		m_renderer.m_depth_buffer,
		vertex_buffer,
		index_buffer,
		instance_buffer
		);
}

vren::render_graph_t DrawChunkList::create_render_graph_node(vren::render_graph_allocator& allocator)
{
	BakedWorldView& baked_world_view = *m_renderer.m_baked_world_view;

	vren::render_graph_node* node = allocator.allocate();
	node->set_name("DrawChunkList");
	node->set_src_stage(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
	node->set_dst_stage(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

	node->add_buffer({ .m_buffer = m_renderer.m_chunk_draw_list.m_buffer.m_handle, }, VK_ACCESS_INDIRECT_COMMAND_READ_BIT);
	node->add_buffer({ .m_buffer = m_renderer.m_chunk_draw_list_idx.m_buffer.m_handle, }, VK_ACCESS_INDIRECT_COMMAND_READ_BIT);
	node->add_buffer({ .m_buffer = baked_world_view.m_vertex_buffer.get_buffer()->m_buffer.m_handle, }, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
	node->add_buffer({ .m_buffer = baked_world_view.m_instance_buffer.get_buffer()->m_buffer.m_handle, }, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
	node->add_buffer({ .m_buffer = baked_world_view.m_index_buffer.get_buffer()->m_buffer.m_handle, }, VK_ACCESS_INDEX_READ_BIT);
	m_renderer.m_gbuffer->add_render_graph_node_resources(*node, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
	node->add_image({
		.m_image = m_renderer.m_depth_buffer->get_image(),
		.m_image_aspect = VK_IMAGE_ASPECT_DEPTH_BIT,
	}, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);

	node->set_callback([this](uint32_t frame_idx, VkCommandBuffer cmd_buf, vren::resource_container& resource_container)
	{
		record(cmd_buf, resource_container);
	});
	return vren::render_graph_gather(node);
}
