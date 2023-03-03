#pragma once

#include <functional>

#include <glm/glm.hpp>

namespace explo
{
	class DeltaChunkIterator
	{
	public:
		using CallbackT = std::function<void(glm::ivec3 const&)>;

	private:
		glm::ivec3 m_old_center;
		glm::ivec3 m_new_center;
		int m_render_distance;
		CallbackT m_callback;

		std::vector<bool> m_covered;

	public:
		explicit DeltaChunkIterator(
			glm::ivec3 const& old_center,
			glm::ivec3 const& new_center,
			int render_distance,
			CallbackT const& callback
			);
		~DeltaChunkIterator() = default;

		/// Iterates over the new chunks according to the shift given by world view's old center and new center.
		void iterate();

	private:
		void iterate_recursive(glm::ivec3 const& current_chunk);
	};

} // namespace explo