#include "DebugUi.hpp"

#include <imgui.h>

#include "Renderer.hpp"
#include "Game.hpp"
#include "util/system.hpp"

using namespace explo;

DebugUi::DebugUi(Renderer& renderer) :
	m_renderer(renderer)
{
}

DebugUi::~DebugUi()
{
}

void DebugUi::display_player_window()
{
	Entity& player = *explo::game().m_player;

	if (ImGui::Begin("Player"))
	{
		glm::vec3 const& pos = player.get_position();
		glm::ivec3 const& chunk_pos = player.get_chunk_position();
		glm::vec3 const& chunk_rel_pos = player.get_chunk_relative_position();

		ImGui::Text("Position: (%.3f, %.3f, %.3f)", pos.x, pos.y, pos.z);
		ImGui::Text("Chunk position: (%d, %d, %d)", chunk_pos.x, chunk_pos.y, chunk_pos.z);
		ImGui::Text("Chunk-relative position: (%.3f, %.3f, %.3f)", chunk_rel_pos.x, chunk_rel_pos.y, chunk_rel_pos.z);
		ImGui::Text("Yaw: %.3f", player.get_yaw());
		ImGui::Text("Pitch: %.3f", player.get_pitch());

		glm::vec3 right = player.get_right();
		glm::vec3 up = player.get_up();
		glm::vec3 forward = player.get_forward();
		ImGui::Text("Right: (%.3f, %.3f, %.1f)", right.x, right.y, right.z);
		ImGui::Text("Up: (%.3f, %.3f, %.3f)", up.x, up.y, up.z);
		ImGui::Text("Forward: (%.3f, %.3f, %.3f)", forward.x, forward.y, forward.z);
	}

	ImGui::End();
}

void DebugUi::display_world_view_window()
{
	Entity& player = *explo::game().m_player;

	if (!player.has_world_view()) return;

	World& world = player.get_world();
	WorldView& world_view = player.get_world_view();

	glm::ivec3 render_distance = world_view.get_render_distance();
	glm::ivec3 world_view_side = world_view.get_side();

	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_Once);

	if (ImGui::Begin("World view"))
	{
		// https://github.com/ocornut/imgui/issues/2342#issuecomment-462515297

		// TODO reverse Y

		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		ImVec2 win_pos = ImGui::GetWindowPos();
		ImVec2 win_reg = ImGui::GetWindowContentRegionMin();
		win_pos.x += win_reg.x;
		win_pos.y += win_reg.y;

		ImVec2 rect_size = ImVec2(20, 20);
		ImVec2 player_rect_size = ImVec2(2, 2);
		ImVec2 rect_pad = ImVec2(4, 4);

		glm::ivec3 player_chunk_pos = player.get_chunk_position();
		glm::vec3 player_chunk_rel_pos = player.get_chunk_relative_position();

		for (int x = 0; x < world_view_side.x; x++)
		{
			for (int z = 0; z < world_view_side.z; z++)
			{
				glm::ivec3 chunk_pos(
					x - render_distance.x + player_chunk_pos.x,
					player_chunk_pos.y,
					z - render_distance.z + player_chunk_pos.z
					);

				ImU32 rect_color = 0xFF9E9E9E; // Grey, not loaded
				if (world.is_chunk_loaded(chunk_pos))
				{
					std::shared_ptr<Chunk> chunk = world.get_chunk(chunk_pos);
					if (chunk->has_surface())     rect_color = 0xFF485579; // Brownish, with surface
					else if (chunk->has_volume()) rect_color = 0xFF00FFFF; // Yellow, with volume
					else                          rect_color = 0xFF90FFCC; // Green, just loaded
				}

				draw_list->AddRectFilled(
					ImVec2(
						win_pos.x + x * rect_size.x + rect_pad.x * x,
						win_pos.y + z * rect_size.y + rect_pad.y * z
						),
					ImVec2(
						win_pos.x + x * rect_size.x + rect_size.x + rect_pad.x * x,
						win_pos.y + z * rect_size.y + rect_size.y + rect_pad.y * z
						),
					rect_color,
					0.0f,
					ImDrawFlags_None
					);
			}
		}

		ImVec2 player_rect_pos = ImVec2(
			win_pos.x + render_distance.x * rect_size.x + rect_pad.x * render_distance.x,
			win_pos.y + render_distance.z * rect_size.y + rect_pad.y * render_distance.z
			);

		glm::vec3 n = player_chunk_rel_pos / Chunk::k_world_size; // Normalized chunk-relative position
		player_rect_pos.x += n.x * rect_size.x;
		player_rect_pos.y += n.z * rect_size.y;

		draw_list->AddRectFilled(
			ImVec2(
				player_rect_pos.x - player_rect_size.x / 2,
				player_rect_pos.y - player_rect_size.y / 2
				),
			ImVec2(
				player_rect_pos.x + player_rect_size.x / 2,
				player_rect_pos.y + player_rect_size.y / 2
				),
			0xFF0000FF,
			5.0f,
			ImDrawFlags_None
			);
	}

	ImGui::End();
}

void DebugUi::display_baked_world_view_window()
{
	if (!m_renderer.has_world_view()) return;

	BakedWorldView& baked_world_view = m_renderer.get_world_view();

	if (ImGui::Begin("Baked world view"))
	{
		ImGui::Text("Vertex buffer - Size: %zu, Allocated pages: %d",
					baked_world_view.m_vertex_buffer.get_size(),
					baked_world_view.m_vertex_buffer_allocator.get_num_allocated_pages()
					);

		ImGui::Text("Index buffer - Size: %zu, Allocated pages: %d",
					baked_world_view.m_index_buffer.get_size(),
					baked_world_view.m_index_buffer_allocator.get_num_allocated_pages()
					);

		ImGui::Text("Instance buffer - Size: %zu, Allocated pages: %d",
					baked_world_view.m_instance_buffer.get_size(),
					baked_world_view.m_instance_buffer_allocator.get_num_allocated_pages()
					);

		ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();

		auto& circular_grid = baked_world_view.m_circular_grid;
		ImGui::Text("Render distance: %d", circular_grid.m_render_distance);
		ImGui::Text("Circular grid start: (%d, %d, %d)", circular_grid.m_start.x, circular_grid.m_start.y, circular_grid.m_start.z);
	}

	ImGui::End();
}

void DebugUi::display_renderer_window()
{
	if (ImGui::Begin("Renderer"))
	{
		ImGui::Text("Dt: %.3f", game().m_dt);
		ImGui::Text("FPS: %d", game().m_fps);
	}

	ImGui::End();
}

void DebugUi::display_jobs_window()
{
	ThreadPool& thread_pool = game().m_thread_pool;

	// Thread pool
	if (ImGui::Begin("Thread pool"))
	{
		ImGui::Text("Main thread jobs: -1"); // todo

		ImGui::Separator();

		ImGui::Text("Threads count: %zu", thread_pool.get_thread_count());
		ImGui::Text("Jobs count: %zu", thread_pool.get_job_count());

		ImGui::Separator();

		for (size_t thread_id = 0; thread_id < thread_pool.get_thread_count(); thread_id++)
			ImGui::Text("Thread %03zu: %s", thread_id, thread_pool.is_thread_working(thread_id) ? "YES" : "NOO");

		ImGui::Separator();

		// Virtual memory
		ImGui::Text("Virtual memory: %s/%s",
					stringify_byte_size(get_virtual_memory_used_by_current_process()).c_str(),
					stringify_byte_size(get_total_virtual_memory()).c_str()
					);

		// Physical memory
		ImGui::Text("Physical memory: %s/%s",
					stringify_byte_size(get_physical_memory_used_by_current_process()).c_str(),
					stringify_byte_size(get_total_physical_memory()).c_str()
					);
	}

	ImGui::End();
}

void DebugUi::display_vma_memory_statistics()
{
	if (ImGui::Begin("Vulkan memory statistics"))
	{
		VmaTotalStatistics stats{};
		vmaCalculateStatistics(m_renderer.m_context.m_vma_allocator, &stats);

		VmaDetailedStatistics& detailed_stats = stats.total;

		ImGui::Text("blockCount: %d", detailed_stats.statistics.blockCount);
		ImGui::Text("allocationCount: %d", detailed_stats.statistics.allocationCount);
		ImGui::Text("blockBytes: %zu", detailed_stats.statistics.blockBytes);
		ImGui::Text("allocationBytes: %zu", detailed_stats.statistics.allocationBytes);

		ImGui::Separator();

		ImGui::Text("unusedRangeCount: %d", detailed_stats.unusedRangeCount);
		ImGui::Text("allocationSizeMin: %zu", detailed_stats.allocationSizeMin);
		ImGui::Text("allocationSizeMax: %zu", detailed_stats.allocationSizeMax);
		ImGui::Text("unusedRangeSizeMin: %zu", detailed_stats.unusedRangeSizeMin);
		ImGui::Text("unusedRangeSizeMax: %zu", detailed_stats.unusedRangeSizeMax);
	}

	ImGui::End();
}

void DebugUi::display()
{
	display_jobs_window();
	display_player_window();
	display_renderer_window();
	display_baked_world_view_window();
	display_vma_memory_statistics();
}
