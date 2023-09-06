#include "Renderer.hpp"

#include <fstream>

#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>

#include "Game.hpp"
#include "log.hpp"

using namespace explo;

Renderer::Renderer(GLFWwindow* window) :
	m_window(window),

	m_context([]() {
		vren::context_info context_info{
			.m_app_name = "explo",
			.m_app_version = VK_MAKE_VERSION(1, 0, 0),
			.m_layers = {},
			.m_extensions = {},
			.m_device_extensions = {}
		};

		uint32_t glfw_extension_count = 0;
		char const** glfw_extensions;
		glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
		context_info.m_extensions.insert(context_info.m_extensions.end(), glfw_extensions, glfw_extensions + glfw_extension_count);

		return vren::context(context_info);
	}()),

    m_surface([&]() {
        VkSurfaceKHR surface_handle{};
        VREN_CHECK(glfwCreateWindowSurface(m_context.m_instance, m_window, nullptr, &surface_handle), &m_context);
        return vren::vk_surface_khr(m_context, surface_handle);
    }()),

    m_presenter(m_context, m_surface, [this](vren::swapchain const& swapchain) {
        on_swapchain_change(swapchain);
    }),

    m_render_graph_allocator(),

    m_cluster_and_shade(m_context),
	m_imgui_renderer(m_context, vren::imgui_windowing_backend_hooks{
		.m_init_callback      = [this]() { ImGui_ImplGlfw_InitForVulkan(m_window, true); },
		.m_new_frame_callback = []() { ImGui_ImplGlfw_NewFrame(); },
		.m_shutdown_callback  = []() { ImGui_ImplGlfw_Shutdown(); }
	}),

    m_light_array(m_context),
    m_material_buffer(m_context),

	m_chunk_draw_list(vren::vk_utils::create_device_only_buffer(
		m_context,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
		nullptr,
		k_chunk_draw_list_buffer_size
		)),
	m_chunk_draw_list_idx(vren::vk_utils::create_device_only_buffer(
		m_context,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
		nullptr,
		sizeof(uint32_t)
		)),

	m_cull_world_view(*this),
	m_draw_chunk_list(*this)
{
	vren::directional_light* directional_lights =
		m_light_array.m_directional_light_buffer.get_mapped_pointer<vren::directional_light>();

	directional_lights[0].m_direction = glm::vec3(0.707f, -0.707f,  0.);
	directional_lights[0].m_color = glm::vec3(1.0f);

	m_light_array.m_directional_light_count = 1;
}

Renderer::~Renderer()
{
}

glm::uvec2 Renderer::get_framebuffer_size() const
{
	return glm::uvec2(
		m_presenter.get_swapchain()->m_image_width,
		m_presenter.get_swapchain()->m_image_height
		);
}

void Renderer::on_swapchain_change(vren::swapchain const& swapchain)
{
    uint32_t width = swapchain.m_image_width;
    uint32_t height = swapchain.m_image_height;

    m_color_buffer = std::make_shared<vren::vk_utils::color_buffer_t>(
        vren::vk_utils::create_color_buffer(
            m_context,
            width, height,
            VREN_COLOR_BUFFER_OUTPUT_FORMAT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT
        )
    );

    m_depth_buffer = std::make_shared<vren::vk_utils::depth_buffer_t>(
        vren::vk_utils::create_depth_buffer(
			m_context,
            width, height,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
        )
    );

    m_gbuffer = std::make_shared<vren::gbuffer>(m_context, width, height);

    LOG_D("render", "ColorBuffer, DepthBuffer, GBuffer resized to ({}, {})", width, height);
}

void Renderer::on_window_resize(int width, int height)
{
	LOG_D("render", "Window resized to ({}, {})", width, height);

    m_presenter.recreate_swapchain(width, height);
}

void Renderer::flush_device_buffer_operations(VkCommandBuffer cmd_buf, vren::resource_container& res_container)
{
	m_baked_world_view->m_vertex_buffer.record(cmd_buf, res_container);
	m_baked_world_view->m_index_buffer.record(cmd_buf, res_container);
	m_baked_world_view->m_instance_buffer.record(cmd_buf, res_container);

	m_baked_world_view->m_circular_grid.m_gpu_image->record(cmd_buf, res_container);
}

static bool g_render_graph_dump_taken = false;

void Renderer::on_frame(
    uint32_t frame_idx,
    uint32_t swapchain_image_idx,
    vren::swapchain const& swapchain,
    VkCommandBuffer cmd_buf,
    vren::resource_container& res_container
)
{
	if (!m_baked_world_view) return;

	// Barrier to ensure this frame's commands are executed only when ALL the previous commands have executed on GPU
	vren::vk_utils::pipeline_barrier(cmd_buf);

	// Records the host-conducted operations on device buffers; then place barriers to synchronize them with the render-graph
	// execution. This can't be a render-graph node because operations can potentially change buffers (e.g. resize), and render-graph
	// executor wouldn't be able to place barriers
	flush_device_buffer_operations(cmd_buf, res_container);

	vren::vk_utils::pipeline_barrier(cmd_buf, *m_baked_world_view->m_vertex_buffer.get_buffer());
	vren::vk_utils::pipeline_barrier(cmd_buf, *m_baked_world_view->m_index_buffer.get_buffer());
	vren::vk_utils::pipeline_barrier(cmd_buf, *m_baked_world_view->m_instance_buffer.get_buffer());

	vren::vk_utils::pipeline_barrier(cmd_buf, m_baked_world_view->m_circular_grid.m_gpu_image->get_image()->m_image);

	// Begin render graph
	uint64_t start_ns = get_nanos_since_epoch();

    vren::render_graph_builder render_graph(m_render_graph_allocator);

	auto render_target =
		vren::render_target::cover(swapchain.m_image_width, swapchain.m_image_height, *m_color_buffer, *m_depth_buffer);

	glm::uvec2 screen(swapchain.m_image_width, swapchain.m_image_height);

    VkImage output = swapchain.m_images.at(swapchain_image_idx);

	res_container.add_resources(m_color_buffer, m_depth_buffer, m_gbuffer);

    // Clears color buffer
    render_graph.concat(vren::clear_color_buffer(m_render_graph_allocator, m_color_buffer->get_image(), m_background_color));

    // Clears depth buffer
    render_graph.concat(vren::clear_depth_stencil_buffer(m_render_graph_allocator, m_depth_buffer->get_image(), { .depth = 1.0f }));

    // Clears gbuffer
    render_graph.concat(vren::clear_gbuffer(m_render_graph_allocator, *m_gbuffer));

	// Filters only the world view chunks that are visible to the camera
	render_graph.concat(m_cull_world_view.create_render_graph_node(m_render_graph_allocator));

	// Draws the list of visible chunks
	render_graph.concat(m_draw_chunk_list.create_render_graph_node(m_render_graph_allocator));

	// Applies deferred shading
    render_graph.concat(
        m_cluster_and_shade(
            m_render_graph_allocator,
            screen,
			m_camera,
            *m_gbuffer,
            *m_depth_buffer,
            m_light_array,
            m_material_buffer,
            *m_color_buffer
        )
    );

	// Renders UI
	if (m_ui_setup_function)
	{
		render_graph.concat(m_imgui_renderer.render(m_render_graph_allocator, render_target, m_ui_setup_function));
	}

    // Blits the color buffer to the swapchain image for presentation
    render_graph.concat(
        vren::blit_color_buffer_to_swapchain_image(
            m_render_graph_allocator,
            *m_color_buffer,
            swapchain.m_image_width,
            swapchain.m_image_height,
            output,
            swapchain.m_image_width,
            swapchain.m_image_height
        )
    );

    // Transits swapchain image to present layout
    render_graph.concat(vren::transit_swapchain_image_to_present_layout(m_render_graph_allocator, output));

    // Records the render-graph on the command buffer that is eventually submitted
    vren::render_graph_executor executor(frame_idx, cmd_buf, res_container);
    executor.execute(m_render_graph_allocator, render_graph.get_head());

	if (!g_render_graph_dump_taken)
	{
		std::ofstream of("./render_graph.dot");
		vren::render_graph_dumper dumper(of);
		dumper.dump(m_render_graph_allocator, render_graph.get_head());

		g_render_graph_dump_taken = true;
	}

    m_render_graph_allocator.clear();

	m_profile_stats.push_elapsed_time(get_nanos_since_epoch() - start_ns);
}

/* Camera */

void Renderer::set_camera_position(glm::vec3 const& position)
{
	m_camera.m_position = position;
	rebuild_camera_view_matrix();
}

void Renderer::set_camera_rotation(float yaw, float pitch)
{
	m_camera.m_yaw = yaw;
	m_camera.m_pitch = pitch;
	rebuild_camera_view_matrix();
}

void Renderer::set_camera_projection_params(float fov_y, float aspect_ratio, float near_plane, float far_plane)
{
	m_camera.m_fov_y = fov_y;
	m_camera.m_aspect_ratio = aspect_ratio;
	m_camera.m_near_plane = near_plane;
	m_camera.m_far_plane = far_plane;
	rebuild_camera_projection_matrix();
}

void Renderer::rebuild_camera_view_matrix()
{
	m_view_matrix = m_camera.get_view();
}

void Renderer::rebuild_camera_projection_matrix()
{
	m_projection_matrix = m_camera.get_projection();
}

/* World view */

void Renderer::recreate_world_view(int render_distance)
{
	m_baked_world_view = std::make_unique<BakedWorldView>(*this, render_distance);
}

BakedWorldView& Renderer::get_world_view()
{
	// todo check baked_world_view exists
	return *m_baked_world_view;
}

void Renderer::upload_block_registry(BlockRegistry const& block_registry)
{
	vren::texture_manager& texture_manager = m_context.m_toolbox->m_texture_manager;

	texture_manager.m_textures.clear();

	if (block_registry.size() > 0)
	{
		std::vector<uint32_t> image_data{};
		image_data.reserve(block_registry.size());

		for (BlockData const& block_data : block_registry.get_block_data())
			image_data.push_back(block_data.m_color);

		vren::vk_utils::texture texture = vren::vk_utils::create_texture(
			m_context,
			block_registry.size(),
			1,
			image_data.data(),
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_FILTER_NEAREST,
			VK_FILTER_NEAREST,
			VK_SAMPLER_MIPMAP_MODE_NEAREST,
			VK_SAMPLER_ADDRESS_MODE_REPEAT,
			VK_SAMPLER_ADDRESS_MODE_REPEAT,
			VK_SAMPLER_ADDRESS_MODE_REPEAT
		);
		texture_manager.m_textures.push_back(std::move(texture));

		LOG_D("upload_block_registry", "Texture uploaded and registered in texture manager");
	}

	vren::vk_utils::texture metallic_roughness_texture =
		vren::vk_utils::create_color_texture(m_context, 255, /* roughness */ 128, /* metallic */ 255, 255);
	texture_manager.m_textures.push_back(std::move(metallic_roughness_texture));

	texture_manager.rewrite_descriptor_set();

	// Update the material buffer
	vren::material_buffer& material_buffer = m_material_buffer;
	vren::material* material_buffer_ptr = material_buffer.m_buffer.get_mapped_pointer<vren::material>();

	material_buffer_ptr[0] = vren::material{
		.m_base_color_texture_idx = 0,         // Block atlas
		.m_metallic_roughness_texture_idx = 1, // Metallic roughness texture
		.m_metallic_factor = 1.0f,
		.m_roughness_factor = 1.0f,
		.m_base_color_factor = glm::vec4(1),
	};
	material_buffer.m_material_count = 1;

	LOG_D("upload_block_registry", "Material buffer updated");
}

void Renderer::render()
{
    m_presenter.present([&](
        uint32_t frame_idx,
        uint32_t swapchain_image_idx,
        vren::swapchain const& swapchain,
        VkCommandBuffer command_buffer,
        vren::resource_container& resource_container
    )
    {
        on_frame(
            frame_idx,
            swapchain_image_idx,
            swapchain,
            command_buffer,
            resource_container
        );
    });
}
