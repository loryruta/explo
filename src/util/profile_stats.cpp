#include "profile_stats.hpp"

#include <algorithm>

void explo::profile_stats::push_elapsed_time(uint64_t elapsed_time)
{
	m_min_elapsed_ns = std::min(elapsed_time, m_min_elapsed_ns);
	m_max_elapsed_ns = std::max(elapsed_time, m_max_elapsed_ns);
	m_last_elapsed_ns = elapsed_time;

	if (m_sample_count == 0)
	{
		m_avg_elapsed_ns = double(elapsed_time);
	}
	else
	{
		// Update cumulative average (https://en.wikipedia.org/wiki/Moving_average)
		m_avg_elapsed_ns += double(elapsed_time - m_avg_elapsed_ns) / double(m_sample_count + 1);
	}

	m_sample_count++;
}

void explo::monitor<explo::profile_stats>::push_elapsed_time(uint64_t elapsed_time)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	m_handle.push_elapsed_time(elapsed_time);
}
