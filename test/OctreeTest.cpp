#include <catch.hpp>

#include <random>

#include <glm/glm.hpp>

#include "world/volume/Octree.hpp"

using namespace explo;

TEST_CASE("OctreeVolumeStorage-MortonCode")
{
	srand(NULL);

	glm::ivec3 pos1(23, 10, 2);
	REQUIRE(Octree::to_morton_code(pos1) == 0x1479);

	glm::ivec3 pos2(81, 99, 7);
	REQUIRE(Octree::to_morton_code(pos2) == 0xd1137);

	glm::ivec3 pos3(67, 2, 11);
	REQUIRE(Octree::to_morton_code(pos3) == 0x4083d);

	for (int i = 0; i < 16; i++)
	{
		glm::ivec3 pos{};
		pos.x = rand() % 50 + 50;
		pos.y = rand() % 50 + 50;
		pos.z = rand() % 50 + 50;

		REQUIRE(Octree::to_voxel_position(Octree::to_morton_code(pos)) == pos);
	}
}

TEST_CASE("OctreeVolumeStorage-SetGet-Traverse")
{
	Octree octree(4); // Depth: 4, Octree: 16x16x16

	glm::ivec3 my_voxel_pos = glm::ivec3(13, 8, 2);
	uint64_t my_morton_code = Octree::to_morton_code(my_voxel_pos);
	octree.set_voxel_at(my_morton_code, 23);

	REQUIRE(octree.get_voxel_at(my_morton_code) == 23);

	bool found = false;
	octree.traverse([&](uint32_t value, uint32_t level, uint32_t morton_code)
	{
		REQUIRE(value == 23);
		REQUIRE(my_morton_code == morton_code);
		REQUIRE(my_voxel_pos == Octree::to_voxel_position(morton_code));
		found = true;
	});
	REQUIRE(found);
}
