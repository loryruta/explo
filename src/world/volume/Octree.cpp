#include "Octree.hpp"

using namespace explo;

Octree::Octree(uint32_t depth) :
	m_depth(depth)
{
}

Octree::~Octree()
{
}

uint32_t Octree::get_voxel_at(uint32_t morton_code) const
{
	uint32_t node_idx = 0; // Root
	for (int level = 0; level < m_depth; level++)
	{
		uint32_t child_idx = (morton_code >> ((m_depth - level - 1) * 3)) & 0x7;
		if ((node_idx + child_idx) >= m_data.size())
			return 0;

		uint32_t child_val = m_data[node_idx + child_idx];
		if ((child_val & 0x80000000) != 0) // Parent node
			node_idx = child_val & 0x7FFFFFFF;
		else // Leaf node
			return child_val;
	}
	return 0;
}

void Octree::set_voxel_at(uint32_t morton_code, uint32_t value)
{
	uint32_t node_idx = 0;
	for (int level = 0; level < m_depth; level++)
	{
		uint32_t child_idx = (morton_code >> ((m_depth - level - 1) * 3)) & 0x7;
		if ((node_idx + child_idx) >= m_data.size())
			m_data.resize(m_data.size() + k_grow_size); // Will keep current data and zero new data

		uint32_t child_val = m_data[node_idx + child_idx];
		if ((child_val & 0x80000000) != 0) // Parent node
		{
			node_idx = child_val & 0x7FFFFFFF;
		}
		else // Leaf node
		{
			if (child_val == value)
			{ // The reached node is a leaf node and already has the value
				return;
			}
			else if (level == m_depth - 1)
			{ // I've reached the max resolution, I can just set the leaf node value and return
				m_data[node_idx + child_idx] = value;
			}
			else
			{ // The leaf node becomes a parent node, and we allocate its children
				m_data[node_idx + child_idx] = m_next_alloc_index | 0x80000000;
				node_idx = m_next_alloc_index;
				m_next_alloc_index += 8;
			}
		}
	}
}

void Octree::traverse_r(
	uint32_t node_idx,
	uint32_t level,
	uint32_t morton_code,
	TraversalCallbackT const& callback
	) const
{
	for (int child_idx = 0; child_idx < 8; child_idx++)
	{
		if ((node_idx + child_idx) >= m_data.size())
			return;

		uint32_t child_morton_code = morton_code | (child_idx << ((m_depth - level - 1) * 3));
		uint32_t child_val = m_data[node_idx + child_idx];
		if ((child_val & 0x80000000) != 0) // Parent node
		{
			traverse_r(
				child_val & 0x7FFFFFFF,
				level + 1,
				child_morton_code,
				callback
				);
		}
		else if (child_val > 0) // Leaf node
		{
			callback(
				child_val,
				level,
				child_morton_code
				);
		}
	}
}

void Octree::traverse(TraversalCallbackT const& callback) const
{
	traverse_r(0, 0, 0, callback);
}

uint32_t Octree::to_morton_code(glm::ivec3 const& voxel_pos)
{
	uint32_t x = voxel_pos.x;
	uint32_t y = voxel_pos.y;
	uint32_t z = voxel_pos.z;

	uint32_t morton_code = 0;
	int i = 0;
	while (x || y || z)
	{
		morton_code |= ((x & 1) | ((y & 1) << 1) | ((z & 1) << 2)) << (i * 3);

		x >>= 1;
		y >>= 1;
		z >>= 1;

		i++;
	}
	return morton_code;
}

glm::ivec3 Octree::to_voxel_position(uint32_t morton_code)
{
	glm::ivec3 voxel_pos{};
	int i = 0;
	while (morton_code > 0)
	{
		voxel_pos.x |= (morton_code & 1) << i;
		voxel_pos.y |= ((morton_code & 0b10) >> 1) << i;
		voxel_pos.z |= ((morton_code & 0b100) >> 2) << i;

		morton_code >>= 3;
		i++;
	}
	return voxel_pos;
}
