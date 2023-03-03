#pragma once

#include <unordered_map>
#include <memory>

#include <glm/glm.hpp>

#include "Chunk.hpp"
#include "util/misc.hpp"
#include "world/surface/SurfaceGenerator.hpp"

namespace explo
{
	class World : public std::enable_shared_from_this<World>
	{
		friend class Chunk;
		friend class Entity;

	public:
		using ChunkLoadedCallbackT = std::function<void(std::shared_ptr<Chunk> const&)>;

	private:
		VolumeGenerator& m_volume_generator;
		SurfaceGenerator& m_surface_generator;

		std::unordered_map<glm::ivec3, std::shared_ptr<Chunk>, vec_hash> m_chunks;

	public:
		explicit World(VolumeGenerator& volume_generator, SurfaceGenerator& surface_generator);
		~World();

		VolumeGenerator& get_volume_generator() const { return m_volume_generator; }
		SurfaceGenerator& get_surface_generator() const { return m_surface_generator; }

		Chunk& load_chunk_async(glm::ivec3 const& chunk_pos, ChunkLoadedCallbackT const& callback);
		bool unload_chunk(glm::ivec3 const& chunk_pos);

	private:
		void generate_chunk_surface(Chunk& chunk);
		void generate_chunk_async(std::shared_ptr<Chunk> const& chunk, ChunkLoadedCallbackT const& callback);
	};
}
