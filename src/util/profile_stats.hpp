#pragma once

#include <cstdint>
#include <functional>
#include <type_traits>
#include <mutex>

namespace explo
{
	// ------------------------------------------------------------------------------------------------
	// monitor
	// ------------------------------------------------------------------------------------------------

	template<typename _t>
	class base_monitor
	{
	protected:
		_t m_handle;
		mutable std::mutex m_mutex;

	public:
		_t read() const
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			return m_handle;
		}
	};

	template<typename _t>
	class monitor : public base_monitor<_t> {};

	template<typename _t>
	void access(_t const& object, std::function<void(_t const&)> const& callback)
	{
		callback(object);
	}

	template<typename _t> // template specialization for monitor<_t>
	void access(monitor<_t> const& monitored_object, std::function<void(_t const&)> const& callback)
	{
		_t t = monitored_object.read();
		callback(t);
	}

	// ------------------------------------------------------------------------------------------------
	// profile_stats
	// ------------------------------------------------------------------------------------------------

	class profile_stats
	{
		uint64_t m_min_elapsed_ns = UINT64_MAX;
		uint64_t m_max_elapsed_ns = 0;
		uint64_t m_last_elapsed_ns = UINT64_MAX;

		double m_avg_elapsed_ns;

		size_t m_sample_count = 0; ///< The total number of samples pushed

	public:
		explicit profile_stats() = default;
		~profile_stats() = default;

		uint64_t min_ns() const  { return m_min_elapsed_ns;  }
		uint64_t max_ns() const  { return m_max_elapsed_ns;  }
		uint64_t last_ns() const { return m_last_elapsed_ns; }
		double avg_ns() const    { return m_avg_elapsed_ns; }

		double min_ms() const  { return m_min_elapsed_ns / 1'000'000.0; }
		double max_ms() const  { return m_max_elapsed_ns / 1'000'000.0; }
		double avg_ms() const  { return m_avg_elapsed_ns / 1'000'000.0; }
		double last_ms() const { return m_last_elapsed_ns / 1'000'000.0; }

		void push_elapsed_time(uint64_t elapsed_time);
	};

	// ------------------------------------------------------------------------------------------------
	// monitor<profile_stats>
	// ------------------------------------------------------------------------------------------------

	template<>
	class monitor<profile_stats> : public base_monitor<profile_stats>
	{
	public:
		void push_elapsed_time(uint64_t elapsed_time);
	};


} // namespace explo
