#include "debug_ui.hpp"

#include <tuple>

#include <imgui.h>

#include "Game.hpp"
#include "util/misc.hpp"
#include "util/system.hpp"

using namespace explo;

debug_ui::debug_ui()
{}


void explo::debug_ui::display_renderer_window()
{
	if (ImGui::Begin("Renderer"))
	{
		ImGui::Text("Nothing in here at the moment... :(");
	}

	ImGui::End();
}

void explo::debug_ui::display_jobs_window()
{
	explo::ThreadPool& thread_pool = game().m_thread_pool;

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

void explo::debug_ui::display()
{
	display_renderer_window();
	display_jobs_window();
}

