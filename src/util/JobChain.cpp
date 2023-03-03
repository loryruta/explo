#include "JobChain.hpp"


using namespace explo;

JobChain& JobChain::then(Job const& job)
{
	m_jobs.push_back(job);
	return *this;
}

void JobChain::execute() const
{
	for (Job const& job : m_jobs)
		job();
}

void enqueue_on_thread_pool(ThreadPool& thread_pool, std::list<JobChain::Job> const& jobs)
{
	thread_pool.enqueue_job([&thread_pool, jobs = jobs]() mutable {
		JobChain::Job job = jobs.front();
		jobs.pop_front();

		job();

		enqueue_on_thread_pool(thread_pool, jobs);
	});
}

void JobChain::dispatch_on_thread_pool(ThreadPool& thread_pool) const
{
	enqueue_on_thread_pool(thread_pool, m_jobs);
}
