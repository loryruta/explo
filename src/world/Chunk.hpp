#pragma once

#include <vector>
#include <array>
#include <functional>
#include <optional>
#include <mutex>

#include <glm/glm.hpp>

#include <vren/model/model.hpp>

#include "world/surface/SurfaceGenerator.hpp"
#include "world/volume/storage/VolumeStorage.hpp"
#include "world/volume/VolumeGenerator.hpp"

namespace explo
{
	// Forward decl
	class World;
	class WorldView;

	class BakedWorldView;

	// ------------------------------------------------------------------------------------------------

	/* Surface */
	using chunk_surface_t = vren::model;

	/* Block */
	using block_t = uint8_t;

	class Chunk : public std::enable_shared_from_this<Chunk>
	{
		friend class World;
		friend class WorldView;
		friend class BakedWorldView;
		friend class VolumeGenerator;
		friend class SurfaceGenerator;

	public:
		static constexpr glm::ivec3 k_grid_size = glm::ivec3(16, 16, 16);
		static constexpr glm::vec3 k_world_size = glm::vec3(k_grid_size);

	private:
		World& m_world;
		glm::ivec3 m_position;

		mutable std::mutex m_volume_mutex;
		std::unique_ptr<VolumeStorage> m_volume;

		std::unique_ptr<Surface> m_surface;

	public:
		explicit Chunk(World& world, glm::ivec3 const& position);
		~Chunk();

		World& get_world() const { return m_world; }
		glm::ivec3 const& get_position() const { return m_position; }

		bool has_volume() const;
		VolumeStorage& get_volume() const;
		void set_volume(std::unique_ptr<VolumeStorage>&& volume);

		uint8_t get_block_type_at(glm::ivec3 const& block) const;

		bool has_surface() const { return bool(m_surface); };
		std::unique_ptr<Surface> const& get_surface() const { return m_surface; }

		glm::uvec3 const& get_grid_size() const { return k_grid_size; }

		bool test_block_position(glm::ivec3 const& block_pos);
		glm::ivec3 to_world_block_position(glm::ivec3 const& chunk_block_pos) const;
		glm::vec3 to_world_position(glm::vec3 const& chunk_pos) const;

		static glm::ivec3 to_chunk_block_position(glm::ivec3 const& block_pos);
		static glm::vec3 to_chunk_position(glm::vec3 const& world_pos);

		/// Checks whether the relative block position is inside the chunk.
		static bool test_chunk_block_position(glm::ivec3 const& chunk_block_pos);

		static glm::ivec3 get_position(glm::ivec3 const& block_pos);
	};
}
