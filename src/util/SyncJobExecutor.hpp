#pragma once

#include <deque>
#include <functional>
#include <mutex>

namespace explo
{
	class SyncJobExecutor
	{
	public:
		using JobT = std::function<void()>;

	private:
		std::deque<JobT> m_jobs;

		mutable std::mutex m_mutex;

	public:
		explicit SyncJobExecutor();
		~SyncJobExecutor() = default;

		size_t get_job_count() const;

		void enqueue_job(JobT const& job);

		void process();
	};
} // namespace explo
