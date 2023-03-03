#include "WorldView.hpp"

#include "DeltaChunkIterator.hpp"
#include "util/JobChain.hpp"
#include "video/RenderApi.hpp"
#include "Game.hpp"

using namespace explo;

WorldView::WorldView(World& world, glm::ivec3 const& init_position, int render_distance) :
	m_render_distance(render_distance)
{
	set_world(world, init_position);
}

WorldView::~WorldView()
{
}

glm::ivec3 WorldView::get_relative_chunk_position(glm::ivec3 const& chunk_pos) const
{
	return chunk_pos - m_position + glm::ivec3(m_render_distance);
}

bool WorldView::is_relative_position_inside(glm::ivec3 const& rel_chunk_pos) const
{
	uint32_t side = get_side();

	bool is_in = true;
	is_in &= rel_chunk_pos.x >= 0 && rel_chunk_pos.x < side;
	is_in &= rel_chunk_pos.y >= 0 && rel_chunk_pos.y < side;
	is_in &= rel_chunk_pos.z >= 0 && rel_chunk_pos.z < side;
	return is_in;
}

bool WorldView::is_position_inside(glm::ivec3 const& chunk_pos) const
{
	return is_relative_position_inside(get_relative_chunk_position(chunk_pos));
}

void WorldView::offset_position(glm::ivec3 const& offset)
{
	glm::ivec3 old_position = m_position;

	m_position += offset;

	//m_geometry_circular_grid->shift(offset);

	DeltaChunkIterator old_chunk_iterator(m_position, old_position, m_render_distance, [&](glm::ivec3 const& chunk_pos)
	{
		m_world->unload_chunk(chunk_pos);

		// Now invalidate the references to chunks that were unloaded that, because of shifting, will be still present within the image 3d.
		// This prevents from rendering old chunk data in the wrong world position

		glm::ivec3 rel_position = get_relative_chunk_position(chunk_pos);

		explo::run_on_main_thread([ rel_position ]()
		{
			RenderApi::world_view_destroy_chunk(rel_position);
		});
	});

	DeltaChunkIterator new_chunks_iterator(old_position, m_position, m_render_distance, [&](glm::ivec3 const& chunk_pos)
	{
		m_world->load_chunk_async(chunk_pos, [ weak_world_view = weak_from_this() ](std::shared_ptr<Chunk> const& chunk)
		{
			std::shared_ptr<WorldView> world_view = weak_world_view.lock();

			if (!world_view)
				return;
			
			glm::ivec3 rel_position =
				world_view->get_relative_chunk_position(chunk->get_position());

			explo::run_on_main_thread([ rel_position, weak_chunk = std::weak_ptr(chunk) ]()
			{
				std::shared_ptr<Chunk> chunk = weak_chunk.lock();
				if (!chunk)
					return;
				RenderApi::world_view_upload_chunk(rel_position, *chunk);
			});
		});
	});
}

void WorldView::set_position(glm::ivec3 const& chunk_pos)
{
	offset_position(chunk_pos - m_position);
}

void WorldView::set_world(World& world, glm::ivec3 const& chunk_pos)
{
	m_world = &world;

	// Use an old position such that DeltaChunkIterator will iterate over all the chunks (as the world has changed)
	m_position = chunk_pos + glm::ivec3(m_render_distance * 2);
	set_position(chunk_pos);
}

int WorldView::calc_side(int render_distance)
{
	return render_distance * 2 + 1;
}

size_t WorldView::calc_size(int render_distance)
{
	size_t side = calc_side(render_distance);
	return side * side * side;
}

uint32_t WorldView::relative_position_to_index(int render_distance, glm::ivec3 const& rel_chunk_pos)
{
	size_t side = calc_side(render_distance);
	return rel_chunk_pos.y * (side * side) + rel_chunk_pos.x * side + rel_chunk_pos.z;
}
