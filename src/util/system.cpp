#include "system.hpp"

#include <windows.h>
#include <psapi.h>

// https://stackoverflow.com/questions/63166/how-to-determine-cpu-and-memory-consumption-from-inside-a-process

/* Windows */

MEMORYSTATUSEX get_memory_status()
{
	MEMORYSTATUSEX memory_info{};
	memory_info.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memory_info);
	return memory_info;
}

size_t explo::get_total_virtual_memory()
{
	return get_memory_status().ullTotalPageFile;
}

size_t explo::get_used_virtual_memory()
{
	MEMORYSTATUSEX memory_info = get_memory_status();
	return memory_info.ullTotalPageFile - memory_info.ullAvailPageFile;
}

size_t explo::get_virtual_memory_used_by_current_process()
{
	PROCESS_MEMORY_COUNTERS_EX pmc;
	GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*) &pmc, sizeof(pmc));
	return pmc.PrivateUsage;
}

size_t explo::get_total_physical_memory()
{
	MEMORYSTATUSEX memory_info = get_memory_status();
	return memory_info.ullTotalPhys;
}

size_t explo::get_used_physical_memory()
{
	MEMORYSTATUSEX memory_info = get_memory_status();
	return memory_info.ullTotalPhys - memory_info.ullAvailPhys;
}

size_t explo::get_physical_memory_used_by_current_process()
{
	PROCESS_MEMORY_COUNTERS_EX pmc;
	GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*) &pmc, sizeof(pmc));
	return pmc.WorkingSetSize;
}
