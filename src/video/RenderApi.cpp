#include "RenderApi.hpp"

#include "video/Renderer.hpp"

using namespace explo;

std::unique_ptr<Renderer> s_renderer;

void RenderApi::init(GLFWwindow* window)
{
	s_renderer = std::make_unique<Renderer>(window);
}

void RenderApi::destroy()
{
	s_renderer.reset();
}

Renderer& RenderApi::renderer()
{
	return *s_renderer;
}

void RenderApi::window_resize(uint32_t width, uint32_t height)
{
	s_renderer->on_window_resize(width, height);
}

void RenderApi::render()
{
	s_renderer->render();
}

/* Camera */

void RenderApi::camera_set_position(glm::vec3 const& position)
{
	s_renderer->set_camera_position(position);
}

void RenderApi::camera_set_rotation(float yaw, float pitch)
{
	s_renderer->set_camera_rotation(yaw, pitch);
}

void RenderApi::camera_set_projection_params(float fov_y, float aspect_ratio, float near_plane, float far_plane)
{
	s_renderer->set_camera_projection_params(fov_y, aspect_ratio, near_plane, far_plane);
}

/* Block registry */

void RenderApi::block_registry_upload(BlockRegistry const& block_registry)
{
	s_renderer->upload_block_registry(block_registry);
}

/* World view */

void RenderApi::world_view_recreate(glm::ivec3 const& init_position, glm::ivec3 const& render_distance)
{
	s_renderer->recreate_world_view(init_position, render_distance);
}

void RenderApi::world_view_set_position(glm::ivec3 const& offset)
{
	s_renderer->get_world_view().set_position(offset);
}

void RenderApi::world_view_upload_chunk(Chunk const& chunk)
{
	s_renderer->get_world_view().upload_chunk(chunk);
}

void RenderApi::world_view_destroy_chunk(glm::ivec3 const& chunk_pos)
{
	s_renderer->get_world_view().destroy_chunk(chunk_pos);
}

/* UI */

void RenderApi::ui_draw(std::function<void()> const& callback)
{
	s_renderer->set_ui_setup_function(callback);
}
