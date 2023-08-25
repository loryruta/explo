#pragma once

#include <memory>
#include <vector>

#include <volk.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <vren/context.hpp>
#include <vren/toolbox.hpp>
#include <vren/pipeline/clustered_shading.hpp>
#include <vren/presenter.hpp>
#include <vren/pipeline/imgui_renderer.hpp>

#include "util/profile_stats.hpp"
#include "world/WorldView.hpp"
#include "world/Chunk.hpp"
#include "BakedWorldView.hpp"
#include "pipeline/CullWorldView.hpp"
#include "pipeline/DrawChunkList.hpp"
#include "world/BlockRegistry.hpp"

namespace explo
{
    class Renderer
    {
		friend class BakedWorldView;
		friend class BakedWorldViewCircularGrid;
		friend class CullWorldView;
		friend class DebugUi;
		friend class DeviceBuffer;
		friend class DeviceImage3d;
		friend class DrawChunkList;

	private:
		GLFWwindow* m_window;

		vren::context m_context;

		profile_stats m_profile_stats;

		vren::vk_surface_khr m_surface;
		vren::presenter m_presenter;

		vren::render_graph_allocator m_render_graph_allocator;

        vren::cluster_and_shade m_cluster_and_shade;
		vren::imgui_renderer m_imgui_renderer;

        std::shared_ptr<vren::vk_utils::depth_buffer_t> m_color_buffer;
        std::shared_ptr<vren::vk_utils::depth_buffer_t> m_depth_buffer;
        std::shared_ptr<vren::gbuffer> m_gbuffer;

        vren::light_array m_light_array;
        vren::material_buffer m_material_buffer;

		std::function<void()> m_ui_setup_function;

		vren::camera m_camera;
		glm::mat4 m_view_matrix;
		glm::mat4 m_projection_matrix;

		std::unique_ptr<BakedWorldView> m_baked_world_view;

		/// A list of draw calls for every visible chunk (format is VkDrawIndexedIndirectCommand).
		/// Vertex, index, instance buffers are managed by BakedWorldView.
		vren::vk_utils::buffer m_chunk_draw_list;

		/// An atomic counter which is used to allocate the draw call within m_chunk_draw_list.
		vren::vk_utils::buffer m_chunk_draw_list_idx;

		CullWorldView m_cull_world_view;
		DrawChunkList m_draw_chunk_list;

    public:
		static constexpr size_t k_chunk_draw_list_buffer_size = 8388608; // 8MB

		VkClearColorValue m_background_color = VkClearColorValue{0.77f, 0.6f, 1.0f, 0.0f};

        explicit Renderer(GLFWwindow* m_window);
        ~Renderer();

		glm::uvec2 get_framebuffer_size() const;

		auto const& get_profile_stats() const { return m_profile_stats; }

		void on_window_resize(int width, int height);

		void set_ui_setup_function(std::function<void()> const& ui_setup_function)
		{
			m_ui_setup_function = ui_setup_function;
		}

		/* Camera */

		void set_camera_position(glm::vec3 const& position);
		void set_camera_rotation(float yaw, float pitch);
		void set_camera_projection_params(float fov_y, float aspect_ratio, float near_plane, float far_plane);

		void rebuild_camera_view_matrix();
		void rebuild_camera_projection_matrix();

		/* World view */

		bool has_world_view() const { return bool(m_baked_world_view); }
		void recreate_world_view(int render_distance);
		BakedWorldView& get_world_view();

		void upload_block_registry(BlockRegistry const& block_registry);

		void render();

    private:
        void on_swapchain_change(vren::swapchain const& swapchain);

		/// Flushes the host operations pending on the device buffers (e.g. vertex buffer, index buffer, ...).
		void flush_device_buffer_operations(VkCommandBuffer cmd_buf, vren::resource_container& res_container);

        void on_frame(
            uint32_t frame_idx,
            uint32_t swapchain_image_idx,
            vren::swapchain const& swapchain,
            VkCommandBuffer command_buffer,
            vren::resource_container& resource_container
        );
    };
}
