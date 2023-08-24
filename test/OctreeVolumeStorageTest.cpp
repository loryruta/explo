#include <catch.hpp>

#include <random>

#include <glm/glm.hpp>

#include "world/volume/storage/OctreeVolumeStorage.hpp"

using namespace explo;

TEST_CASE("OctreeVolumeStorage-MortonCode")
{
	srand(NULL);

	glm::ivec3 pos1(23, 10, 2);
	REQUIRE(OctreeVolumeStorage::make_morton_code(pos1) == 0x1479);

	glm::ivec3 pos2(81, 99, 7);
	REQUIRE(OctreeVolumeStorage::make_morton_code(pos2) == 0xd1137);

	glm::ivec3 pos3(67, 2, 11);
	REQUIRE(OctreeVolumeStorage::make_morton_code(pos3) == 0x4083d);

	for (int i = 0; i < 16; i++)
	{
		glm::ivec3 pos{};
		pos.x = rand() % 50 + 50;
		pos.y = rand() % 50 + 50;
		pos.z = rand() % 50 + 50;

		uint64_t morton_code = OctreeVolumeStorage::make_morton_code(pos);
		REQUIRE(OctreeVolumeStorage::make_position_from_morton_code(morton_code) == pos);
	}
}

TEST_CASE("OctreeVolumeStorage-SetGet-Traverse")
{
	OctreeVolumeStorage octree(glm::uvec3(16, 16, 16));

	glm::ivec3 my_block_pos = glm::ivec3(13, 8, 2);
	uint64_t my_morton_code = OctreeVolumeStorage::make_morton_code(my_block_pos);
	octree.set_block_type_at(my_block_pos, 23);

	REQUIRE(octree.get_block_type_at(my_block_pos) == 23);

	bool found = false;
	octree.traverse_octree([&](OctreeVolumeStorage::OctreeNode const& node, uint32_t level, uint64_t morton_code)
	{
		if (node.is_leaf())
		{
			if (node.m_block == 0) return;

			REQUIRE(node.m_block == 23);
			REQUIRE(my_morton_code == morton_code);
			REQUIRE(my_block_pos == OctreeVolumeStorage::make_position_from_morton_code(morton_code));

			found = true;
		}
	});
	REQUIRE(found);
}
