#pragma once

#include <cstdint>

#include <glm/glm.hpp>

namespace explo
{
	class VolumeStorage
	{
	protected:
		glm::uvec3 m_size;

	public:
		explicit VolumeStorage(glm::uvec3 const& size) :
			m_size(size)
		{}
		virtual ~VolumeStorage() = default;

		glm::uvec3 const& get_size() const { return m_size; };

		virtual size_t get_byte_size() const = 0;

		virtual uint8_t get_block_type_at(glm::ivec3 const& block) const = 0;
		virtual void set_block_type_at(glm::ivec3 const& block, uint8_t block_type) = 0;
	};
} // namespace explo
