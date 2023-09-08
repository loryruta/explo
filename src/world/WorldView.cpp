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

WorldView::WorldView(World& world, glm::ivec3 const& init_position, glm::ivec3 const& render_distance) :
	m_world(world),
	m_render_distance(render_distance)
{
	// Use an old position such that DeltaChunkIterator will iterate over all the chunks (as the world has changed)
	m_position = init_position + glm::ivec3(m_render_distance * 2 + 1);
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
	glm::ivec3 side = get_side();

	bool is_in = true;
	is_in &= rel_chunk_pos.x >= 0 && rel_chunk_pos.x < side.x;
	is_in &= rel_chunk_pos.y >= 0 && rel_chunk_pos.y < side.y;
	is_in &= rel_chunk_pos.z >= 0 && rel_chunk_pos.z < side.z;
	return is_in;
}

bool WorldView::is_position_inside(glm::ivec3 const& chunk_pos) const
{
	return is_relative_position_inside(get_relative_chunk_position(chunk_pos));
}

void WorldView::offset_position(glm::ivec3 const& offset)
{
	if (offset.x == 0 && offset.y == 0 && offset.z == 0) return;

	glm::ivec3 old_position = m_position;
	m_position += offset;

	// Destroy the chunks that went out of the world view
	DeltaChunkIterator old_chunks_iterator(m_position, old_position, m_render_distance, [&](glm::ivec3 const& chunk_pos)
	{
		m_world.unload_chunk(chunk_pos);

#ifdef CALL_RENDER_API
		RenderApi::world_view_destroy_chunk(chunk_pos);
#endif
	});
	old_chunks_iterator.iterate();

#ifdef CALL_RENDER_API
	// Set the world view new position for the renderer, this has to be done *after* destroying old chunks. If the position were
	// set before destroying the chunks, old chunk references would go missing and leak memory
	RenderApi::world_view_set_position(m_position);
#endif

	// Iterate the new chunks, generate and upload them for rendering
	DeltaChunkIterator new_chunks_iterator(old_position, m_position, m_render_distance, [&](glm::ivec3 const& chunk_pos)
	{
		glm::ivec3 rel_pos = chunk_pos - m_position + m_render_distance;

		m_world.load_chunk_async(chunk_pos, [](std::shared_ptr<Chunk> const& chunk)
		{
#ifdef CALL_RENDER_API
			explo::run_on_main_thread([ weak_chunk = std::weak_ptr(chunk) ]()
			{
				std::shared_ptr<Chunk> chunk = weak_chunk.lock();
				if (!chunk) return;

				// Try to upload the chunk for rendering. Since the generation is asynchronous, we could be asking the renderer
				// to upload a chunk that is now outside the world view (e.g. the player moved very fast). In this case the
				// renderer will silently ignore the uploading
				RenderApi::world_view_upload_chunk(*chunk);
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

bool WorldView::is_chunk_position_inside(
	glm::ivec3 const& world_view_pos,
	glm::ivec3 const& render_distance,
	glm::ivec3 const& chunk_pos
	)
{
	glm::ivec3 side = render_distance * 2 + 1;
	glm::ivec3 rel_pos = chunk_pos - world_view_pos + render_distance;
	return
		rel_pos.x >= 0 && rel_pos.x < side.x &&
		rel_pos.y >= 0 && rel_pos.y < side.y &&
		rel_pos.z >= 0 && rel_pos.z < side.z;
}

void WorldView::iterate_chunks(
	glm::ivec3 const& position,
	glm::ivec3 const& render_distance,
	std::function<void(glm::ivec3 const&)> const& callback
)
{
	glm::ivec3 offset{};
	for (offset.x = -render_distance.x; offset.x <= render_distance.x; offset.x++)
	{
		for (offset.y = -render_distance.y; offset.y <= render_distance.y; offset.y++)
		{
			for (offset.z = -render_distance.z; offset.z <= render_distance.z; offset.z++)
			{
				glm::ivec3 chunk_pos = position + offset;
				callback(chunk_pos);
			}
		}
	}
}
