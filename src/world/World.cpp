#include "World.hpp"

#include <memory>

#include "Game.hpp"
#include "log.hpp"
#include "util/JobChain.hpp"

using namespace explo;

World::World(VolumeGenerator& volume_generator, SurfaceGenerator& surface_generator) :
	m_volume_generator(volume_generator),
	m_surface_generator(surface_generator)
{
}

World::~World()
{
}

std::pair<Chunk&, bool> World::load_chunk_async(glm::ivec3 const& chunk_pos, ChunkLoadedCallbackT const& callback)
{
	std::shared_ptr<Chunk> chunk = std::make_shared<Chunk>(*this, chunk_pos);
	auto [iterator, inserted] = m_chunks.emplace(chunk_pos, chunk);

	if (!inserted)
		return {*iterator->second.get(), false}; // Chunk already loaded

	generate_chunk_async(chunk, callback);

	return {*chunk, true};
}

bool World::unload_chunk(glm::ivec3 const& chunk_pos)
{
	auto chunk_it = m_chunks.find(chunk_pos);
	if (chunk_it == m_chunks.end())
		return false; // Chunk wasn't loaded

	std::shared_ptr<Chunk> chunk = chunk_it->second;
	return m_chunks.erase(chunk_pos) == 1;
}

void World::generate_chunk_surface(Chunk& chunk)
{
	// TODO Volume generation shall not take place while the surface is being generated

	uint64_t started_at = current_ms();

	Surface surface;
	SurfaceWriter surface_writer(surface);

	m_surface_generator.generate(chunk, surface_writer);

	chunk.m_surface = std::make_unique<Surface>(std::move(surface));

	glm::ivec3 chunk_pos = chunk.get_position();
}

void World::generate_chunk_async(std::shared_ptr<Chunk> const& chunk, ChunkLoadedCallbackT const& callback)
{
	JobChain job_chain{};
	job_chain
		// Generate the volume
		.then([ weak_world = weak_from_this(), weak_chunk = std::weak_ptr(chunk) ]()
		{
			std::shared_ptr<World> world = weak_world.lock();
			std::shared_ptr<Chunk> chunk = weak_chunk.lock();

			if (!world || !chunk) return;

			uint64_t started_at = current_ms();

			world->m_volume_generator.generate_volume(*chunk);

			glm::ivec3 chunk_pos = chunk->get_position();
			LOG_D("World", "Volume generated; Chunk: ({}, {}, {}), dt: {}",
				chunk_pos.x, chunk_pos.y, chunk_pos.z,
				  current_ms() - started_at
				);
		})
		// Generate the surface
		.then([ weak_world = weak_from_this(), weak_chunk = std::weak_ptr(chunk) ]()
		{
			std::shared_ptr<World> world = weak_world.lock();
			std::shared_ptr<Chunk> chunk = weak_chunk.lock();

			if (!world || !chunk) return;

			uint64_t started_at = current_ms();

			world->generate_chunk_surface(*chunk);

			glm::ivec3 chunk_pos = chunk->get_position();
			LOG_D("World", "Surface generated; Chunk: ({}, {}, {}), dt: {}",
				chunk_pos.x, chunk_pos.y, chunk_pos.z,
				current_ms() - started_at
				);
		})
		// Call the user provided callback
		.then([ weak_chunk = std::weak_ptr(chunk), callback ]()
		{
			std::shared_ptr<Chunk> chunk = weak_chunk.lock();

			if (!chunk) return;

			callback(chunk);
		});
	job_chain.dispatch(game().m_thread_pool);
}
