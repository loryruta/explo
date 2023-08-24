#include "WorldView.hpp"

#include "DeltaChunkIterator.hpp"
#include "util/JobChain.hpp"
#include "Game.hpp"
#include "log.hpp"

using namespace explo;

#define CALL_RENDER_API

#ifdef CALL_RENDER_API
#	include "video/RenderApi.hpp"
#endif

WorldView::WorldView(World& world, glm::ivec3 const& init_position, int render_distance) :
	m_world(world),
	m_render_distance(render_distance)
{
	// Use an old position such that DeltaChunkIterator will iterate over all the chunks (as the world has changed)
	m_position = init_position + glm::ivec3(m_render_distance * 2);
	set_position(init_position);
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

	// Destroys the chunks that went out of the WorldView
	DeltaChunkIterator old_chunks_iterator(m_position, old_position, m_render_distance, [&](glm::ivec3 const& chunk_pos)
	{
		m_world.unload_chunk(chunk_pos);

#ifdef CALL_RENDER_API
		glm::ivec3 rel_pos = chunk_pos - old_position + m_render_distance;
		explo::run_on_main_thread([ rel_pos ]() { RenderApi::world_view_destroy_chunk(rel_pos); });
#endif
	});
	old_chunks_iterator.iterate();

	// Shifts the WorldView of the offset, this has to be run _after_ the old chunks are destroyed. Since it's a circular shift,
	// old chunks at the world view's edge, will go upfront
#ifdef CALL_RENDER_API
	explo::run_on_main_thread([ offset ]() { RenderApi::world_view_shift(offset); });
#endif

	// Finally we iterate the new chunks, generate them through the World and upload them for rendering
	DeltaChunkIterator new_chunks_iterator(old_position, m_position, m_render_distance, [&](glm::ivec3 const& chunk_pos)
	{
		glm::ivec3 rel_pos = chunk_pos - m_position + m_render_distance;
		m_world.load_chunk_async(chunk_pos, [ rel_pos ](std::shared_ptr<Chunk> const& chunk)
		{
#ifdef CALL_RENDER_API
			explo::run_on_main_thread([ rel_pos, weak_chunk = std::weak_ptr(chunk) ]()
			{
				std::shared_ptr<Chunk> chunk = weak_chunk.lock();
				if (!chunk)
					return;

				RenderApi::world_view_upload_chunk(rel_pos, *chunk);
			});
#endif
		});
	});
	new_chunks_iterator.iterate();
}

void WorldView::set_position(glm::ivec3 const& chunk_pos)
{
	offset_position(chunk_pos - m_position);
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
