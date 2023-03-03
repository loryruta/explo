#pragma once

#include <glm/glm.hpp>

#include <vector>

#include "BlockData.hpp"

namespace explo
{
	class BlockRegistry
	{
	private:
		std::vector<BlockData> m_block_data;

	public:
		explicit BlockRegistry();
		~BlockRegistry() = default;

		std::vector<BlockData> const& get_block_data() const { return m_block_data; };
		size_t size() const { return m_block_data.size(); }
	};
} // namespace explo
