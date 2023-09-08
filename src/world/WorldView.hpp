#pragma once

#include <memory>

#include <glm/glm.hpp>

#include "World.hpp"
#include "util/Image3d.hpp"
#include "util/CircularImage3d.hpp"
#include "util/VirtualAllocator.hpp"

namespace explo
{
	/// This class handles the loading/unloading of chunks surrounding a particular chunk position (the center), with a radius (the render distance).
	/// While it's mainly used to render the world for the player, it could also be used to load the chunks around an entity for physics computation.
	class WorldView : public std::enable_shared_from_this<WorldView>
	{
		static constexpr size_t k_max_render_distance = 32;
		static constexpr size_t k_max_side = k_max_render_distance * 2 + 1;
		static constexpr size_t k_max_size = k_max_side * k_max_side * k_max_side;

	private:
		World& m_world;
		glm::ivec3 m_position;
		glm::ivec3 m_render_distance;

	public:
		explicit WorldView(World& world, glm::ivec3 const& init_position, glm::ivec3 const& render_distance);
		~WorldView();

		glm::ivec3 get_render_distance() const { return m_render_distance; }
		glm::ivec3 get_side() const { return m_render_distance * 2 + 1; }
		size_t get_size() const { return m_render_distance.x * m_render_distance.y * m_render_distance.z; }

		/// Returns the chunk position relative to the world view. Such that (0,0,0) is left-bottom-back.
		glm::ivec3 get_relative_chunk_position(glm::ivec3 const& chunk_pos) const;

		/// Checks whether the given chunk position, relative to the world view, is inside the world view.
		bool is_relative_position_inside(glm::ivec3 const& rel_chunk_pos) const;

		/// Checks whether the given chunk position is inside the world view.
		bool is_position_inside(glm::ivec3 const& chunk_pos) const;

		glm::ivec3 get_position() const { return m_position; }
		void offset_position(glm::ivec3 const& offset);
		void set_position(glm::ivec3 const& chunk_pos);

		// ------------------------------------------------------------------------------------------------ Static methods

		static bool is_chunk_position_inside(
			glm::ivec3 const& world_view_pos,
			glm::ivec3 const& render_distance,
			glm::ivec3 const& chunk_pos
			);

		/// Iterates the chunks of the world view defined by the given position and render_distance. Calls callback for every occurrence.
		static void iterate_chunks(
			glm::ivec3 const& world_view_pos,
			glm::ivec3 const& render_distance,
			std::function<void(glm::ivec3 const&)> const& callback
			);
	};

} // namespace explo
