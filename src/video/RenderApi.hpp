#pragma once

#include <vector>
#include <functional>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "world/BlockRegistry.hpp"
#include "world/Chunk.hpp"

namespace explo
{
	// Forward decl
	class Renderer;

	/// An API used from core to perform rendering actions.
	namespace RenderApi
	{
		void init(GLFWwindow* window);
		void destroy();

		Renderer& renderer();

		void window_resize(uint32_t width, uint32_t height);
		void render();

		/* Camera */

		void camera_set_position(glm::vec3 const& position);
		void camera_set_rotation(float yaw, float pitch);
		void camera_set_projection_params(float fov_y, float aspect_ratio, float near_plane, float far_plane);

		/* World view */

		void world_view_recreate(glm::ivec3 const& init_chunk_pos, int render_distance);
		void world_view_set_position(glm::ivec3 const& chunk_pos);
		void world_view_upload_chunk(Chunk const& chunk);
		void world_view_destroy_chunk(glm::ivec3 const& chunk_pos);

		/* Block registry */

		void block_registry_upload(BlockRegistry const& block_registry);

		/* UI */

		/// Sets a function in charge of displaying the UI using the ImGui API.
		void ui_draw(std::function<void()> const& callback);
	};
}
