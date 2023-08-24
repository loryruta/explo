#include "SyncJobExecutor.hpp"

#include <cassert>

using namespace explo;

SyncJobExecutor::SyncJobExecutor()
{
}

size_t SyncJobExecutor::get_job_count() const
{
	std::lock_guard<std::mutex> lock(m_mutex);

	return m_jobs.size();
}

void SyncJobExecutor::enqueue_job(JobT const& job)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	m_jobs.push_back(job);
}

void SyncJobExecutor::process()
{
	// Iterates for the jobs that were enqueued just before process() was called. This not to iterate jobs that could be
	// enqueued by a job itself
	std::deque<JobT> jobs;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		jobs = std::move(m_jobs);
	}

	for (JobT const& job : jobs)
		job();
}
