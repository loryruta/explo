#pragma once

#include <functional>
#include <unordered_set>

#include <glm/glm.hpp>

#include "util/misc.hpp"

namespace explo
{
	class DeltaChunkIterator
	{
	public:
		using CallbackT = std::function<void(glm::ivec3 const&)>;

	private:
		glm::ivec3 m_old_center;
		glm::ivec3 m_new_center;
		glm::ivec3 m_render_distance;
		CallbackT m_callback;

		/// A set holding the chunks for which the callback was called (very simple and stupid approach).
		std::unordered_set<glm::ivec3, vec_hash> m_visited_chunks;

	public:
		explicit DeltaChunkIterator(
			glm::ivec3 const& old_center,
			glm::ivec3 const& new_center,
			glm::ivec3 const& render_distance,
			CallbackT const& callback
			);
		~DeltaChunkIterator();

		/// Iterates over the new chunks according to the shift given by world view's old center and new center.
		void iterate();

	private:
		void iterate_along_axis(int axis);
	};

} // namespace explo