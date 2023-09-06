#include "CullWorldView.hpp"

#include "video/Renderer.hpp"

using namespace explo;

CullWorldView::CullWorldView(Renderer& renderer) :
	m_renderer(renderer),
	m_pipeline(create_pipeline())
{
}

CullWorldView::~CullWorldView()
{
}

void CullWorldView::record(VkCommandBuffer cmd_buf, vren::resource_container& res_container)
{
	// Clear the chunk draw list index (atomic counter)
	vkCmdFillBuffer(cmd_buf, m_renderer.m_chunk_draw_list_idx.m_buffer.m_handle, 0, sizeof(uint32_t), 0 /* data */);

	vren::vk_utils::pipeline_barrier(cmd_buf, m_renderer.m_chunk_draw_list_idx);

	// Run cull world view
	m_pipeline.bind(cmd_buf);

	vren::context& context = m_renderer.m_context;
	BakedWorldView& baked_world_view = *m_renderer.m_baked_world_view;
	BakedWorldViewCircularGrid& circular_grid = baked_world_view.m_circular_grid;

	// Push constants
	m_pipeline.push_constants(cmd_buf, VK_SHADER_STAGE_COMPUTE_BIT, &m_renderer.m_camera.m_position, sizeof(m_renderer.m_camera.m_position), offsetof(PushConstants, m_camera));
	m_pipeline.push_constants(cmd_buf, VK_SHADER_STAGE_COMPUTE_BIT, &circular_grid.m_start, sizeof(circular_grid.m_start), offsetof(PushConstants, m_world_view_start));
	m_pipeline.push_constants(cmd_buf, VK_SHADER_STAGE_COMPUTE_BIT, &circular_grid.m_render_distance, sizeof(circular_grid.m_render_distance), offsetof(PushConstants, m_render_distance));

	m_pipeline.acquire_and_bind_descriptor_set(context, cmd_buf, res_container, 0, [&](VkDescriptorSet descriptor_set)
	{
		// Baked world view 3d image
		vren::vk_utils::write_storage_image_descriptor(
			context,
			descriptor_set,
			0, // binding
			circular_grid.m_gpu_image->get_image()->get_image_view(),
			VK_IMAGE_LAYOUT_GENERAL
			);

		// Chunk draw list (output buffer)
		vren::vk_utils::write_buffer_descriptor(
			context,
			descriptor_set,
			1, // binding
			m_renderer.m_chunk_draw_list.m_buffer.m_handle,
			Renderer::k_chunk_draw_list_buffer_size, 0
			);

		// Chunk draw list index (atomic counter)
		vren::vk_utils::write_buffer_descriptor(
			context,
			descriptor_set,
			2, // binding
			m_renderer.m_chunk_draw_list_idx.m_buffer.m_handle,
			sizeof(uint32_t), 0
			);
	});

	m_pipeline.dispatch(cmd_buf, 8, 8, 8); // todo workgroups count dynamical based on world view size

	res_container.add_resources(
		circular_grid.m_gpu_image->get_image()
		);
}

vren::render_graph_t CullWorldView::create_render_graph_node(vren::render_graph_allocator& allocator)
{
	vren::render_graph_node* node = allocator.allocate();
	node->set_name("CullWorldView");
	node->set_src_stage(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
	node->set_dst_stage(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

	node->add_buffer({
		.m_name = "chunk_draw_list",
		.m_buffer = m_renderer.m_chunk_draw_list.m_buffer.m_handle,
	}, VK_ACCESS_SHADER_WRITE_BIT);
	node->add_buffer({
		.m_name = "chunk_draw_list_idx",
		.m_buffer = m_renderer.m_chunk_draw_list_idx.m_buffer.m_handle,
	}, VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT);

	node->add_image({
		.m_name = "baked_world_view-circular_grid",
		.m_image = m_renderer.m_baked_world_view->m_circular_grid.m_gpu_image->get_image()->get_image(),
		.m_image_aspect = VK_IMAGE_ASPECT_COLOR_BIT,
		.m_mip_level = 0,
		.m_layer = 0,
	}, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_SHADER_READ_BIT);

	node->set_callback([this](uint32_t frame_idx, VkCommandBuffer cmd_buf, vren::resource_container& res_container)
	{
		record(cmd_buf, res_container);
	});
	return vren::render_graph_gather(node);
}

vren::pipeline CullWorldView::create_pipeline()
{
	vren::shader_module shader = vren::load_shader_module_from_file(m_renderer.m_context, "./resources/shaders/cull_world_view.comp.spv");
	return vren::create_compute_pipeline(m_renderer.m_context, vren::specialized_shader(shader));
}
