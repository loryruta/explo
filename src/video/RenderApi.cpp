#include "RenderApi.hpp"

#include "video/Renderer.hpp"

using namespace explo;

std::unique_ptr<Renderer> s_renderer;

void RenderApi::init()
{
	// TODO init Renderer
}

void RenderApi::destroy()
{
	s_renderer.reset();
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
}

void RenderApi::camera_set_rotation(float yaw, float pitch)
{
}

void RenderApi::camera_set_projection(glm::mat4 const& projection_matrix)
{
}

/* Block registry */

void RenderApi::block_registry_upload(BlockRegistry const& block_registry)
{
	s_renderer->upload_block_registry(block_registry);
}

/* World view */

void RenderApi::world_view_shift(glm::ivec3 const& offset)
{
	s_renderer->world_view().shift(offset);
}

void RenderApi::world_view_upload_chunk(glm::ivec3 const& position, Chunk const& chunk)
{
	s_renderer->world_view().upload_chunk(position, chunk);
}

void RenderApi::world_view_destroy_chunk(glm::ivec3 const& position)
{
	s_renderer->world_view().destroy_chunk(position);
}
