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
		void camera_set_projection(glm::mat4 const& projection_matrix);

		/* World view */

		void world_view_recreate(int render_distance);
		void world_view_shift(glm::ivec3 const& offset);
		void world_view_upload_chunk(glm::ivec3 const& position, Chunk const& chunk);
		void world_view_destroy_chunk(glm::ivec3 const& position);

		/* Block registry */

		void block_registry_upload(BlockRegistry const& block_registry);

		/* UI */

		/// Sets a function in charge of displaying the UI using the ImGui API.
		void ui_draw(std::function<void()> const& callback);
	};
}
