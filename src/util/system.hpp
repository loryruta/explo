#pragma once

namespace explo
{
	size_t get_total_virtual_memory();
	size_t get_used_virtual_memory();
	size_t get_virtual_memory_used_by_current_process();

	size_t get_total_physical_memory();
	size_t get_used_physical_memory();
	size_t get_physical_memory_used_by_current_process();

} // namespace explo
