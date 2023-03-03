#include "SyncJobExecutor.hpp"

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
	std::lock_guard<std::mutex> lock(m_mutex);

	for (JobT const& job: m_jobs)
		job();

	m_jobs.clear();
}
