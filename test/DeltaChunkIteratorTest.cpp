#include <catch.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "log.hpp"
#include "world/DeltaChunkIterator.hpp"
#include "world/WorldView.hpp"

using namespace explo;

TEST_CASE("DeltaChunkIterator-basic")
{
	constexpr int k_render_distance = 1;
	constexpr int k_side = k_render_distance * 2 + 1;

	glm::ivec3 old_chunk_pos{};
	glm::ivec3 new_chunk_pos{};
	std::vector<int> cover_map{};

	std::tie(old_chunk_pos, new_chunk_pos, cover_map) =
		GENERATE(
			Catch::Generators::table<glm::ivec3, glm::ivec3, std::vector<int>>(
				{ // 1
					std::make_tuple(
						glm::ivec3(0, 0, 0),
						glm::ivec3(-1, 0, 0),
						std::vector<int>{
							0, 1, 1,
							0, 1, 1,
							0, 1, 1, }
						), // 2
					std::make_tuple(
						glm::ivec3(0, 0, 0),
						glm::ivec3(0, -1, 0),
						std::vector<int>{
							1, 1, 1,
							1, 1, 1,
							0, 0, 0, }
						), // 3
					std::make_tuple(
						glm::ivec3(0, 0, 0),
						glm::ivec3(1, 0, 0),
						std::vector<int>{
							1, 1, 0,
							1, 1, 0,
							1, 1, 0, }
						), // 4
					std::make_tuple(
						glm::ivec3(0, 0, 0),
						glm::ivec3(0, 1, 0),
						std::vector<int>{
							0, 0, 0,
							1, 1, 1,
							1, 1, 1, }
						), // 5
					std::make_tuple(
						glm::ivec3(0, 0, 0),
						glm::ivec3(1, 1, 0),
						std::vector<int>{
							0, 0, 0,
							1, 1, 0,
							1, 1, 0, }
						), // 6
					std::make_tuple(
						glm::ivec3(0, 0, 0),
						glm::ivec3(2, 2, 0),
						std::vector<int>{
							0, 0, 0,
							0, 0, 0,
							1, 0, 0, }
						), // 7
					std::make_tuple(
						glm::ivec3(0, 0, 0),
						glm::ivec3(0, 3, 0),
						std::vector<int>{
							0, 0, 0,
							0, 0, 0,
							0, 0, 0, }
						), // 8
					std::make_tuple(
						glm::ivec3(0, 0, 0),
						glm::ivec3(0, -2, 0),
						std::vector<int>{
							1, 1, 1,
							0, 0, 0,
							0, 0, 0, }
						)
				}
			));

	DeltaChunkIterator iterator(old_chunk_pos, new_chunk_pos, k_render_distance, [&](glm::ivec3 const& chunk_pos)
	{
		glm::ivec3 rel_pos = chunk_pos - new_chunk_pos + k_render_distance;
		if (rel_pos.z != 0) return;
		int i = (k_side - rel_pos.y - 1) * k_side + rel_pos.x;
		cover_map[i]++;
	});
	iterator.iterate();

	for (int i : cover_map) REQUIRE(i == 1); // Make sure every chunk is visited once
}

TEST_CASE("DeltaChunkIterator-checkLoadUnloadOnOffset")
{
	// This test checks what's done by the world view when the player moves. The DeltaChunkIterator class is used to iterate
	// and load the new chunks that enter the WorldView, but it's also used to iterate and unload the old chunks that have
	// exited the world view.

	const int k_render_distance = 1;
	const int k_side = k_render_distance * 2 + 1;
	const int k_size = k_side * k_side * k_side;

	glm::ivec3 old_chunk_pos{};
	glm::ivec3 new_chunk_pos{};
	std::tie(old_chunk_pos, new_chunk_pos) =
		GENERATE(
			Catch::Generators::table<glm::ivec3, glm::ivec3>(
				{
					std::make_tuple(glm::ivec3(0, 0, 0), glm::ivec3(1, 0, 0)),
					std::make_tuple(glm::ivec3(0, 0, 0), glm::ivec3(0, 1, 0)),
					std::make_tuple(glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 1)),
					std::make_tuple(glm::ivec3(0, 0, 0), glm::ivec3(1, 0, 1)),
					std::make_tuple(glm::ivec3(0, 0, 0), glm::ivec3(1, 1, 0)),
					std::make_tuple(glm::ivec3(0, 0, 0), glm::ivec3(0, 1, 1)),
					std::make_tuple(glm::ivec3(0, 0, 0), glm::ivec3(1, 1, 1)),
					std::make_tuple(glm::ivec3(0, 0, 0), glm::ivec3(2, 2, 1)),
					std::make_tuple(glm::ivec3(0, 0, 0), glm::ivec3(1, 2, 2)),
					std::make_tuple(glm::ivec3(0, 0, 0), glm::ivec3(2, 2, 2)),
					std::make_tuple(glm::ivec3(0, 0, 0), glm::ivec3(2, 5, 2)),
				}
			));

	std::unordered_set<glm::ivec3, vec_hash> loaded_chunks{};

	WorldView::iterate_chunks(old_chunk_pos, k_render_distance, [&](glm::ivec3 const& chunk_pos)
	{
		loaded_chunks.emplace(chunk_pos);
	});

	CHECK(loaded_chunks.size() == k_size);

	DeltaChunkIterator old_chunks_iterator(new_chunk_pos, old_chunk_pos, k_render_distance, [&](glm::ivec3 const& chunk_pos)
	{
		loaded_chunks.erase(chunk_pos);
	});
	old_chunks_iterator.iterate();

	DeltaChunkIterator new_chunks_iterator(old_chunk_pos, new_chunk_pos, k_render_distance, [&](glm::ivec3 const& chunk_pos)
	{
		loaded_chunks.emplace(chunk_pos);
	});
	new_chunks_iterator.iterate();

	WorldView::iterate_chunks(new_chunk_pos, k_render_distance, [&](glm::ivec3 const& chunk_pos)
	{
		REQUIRE(loaded_chunks.contains(chunk_pos));
	});

	WorldView::iterate_chunks(old_chunk_pos, k_render_distance, [&](glm::ivec3 const& chunk_pos)
	{
		if (!WorldView::is_chunk_position_inside(new_chunk_pos, k_render_distance, chunk_pos))
			REQUIRE(!loaded_chunks.contains(chunk_pos));
	});

	CHECK(loaded_chunks.size() == k_size);
}
