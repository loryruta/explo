#include "JobChain.hpp"

using namespace explo;

JobChain::JobChain() {}

JobChain::~JobChain() {}

JobChain &JobChain::then(JobT const &job)
{
    m_jobs.push_back(job);
    return *this;
}

void JobChain::dispatch() const
{
    for (JobT const &job : m_jobs) job();
}

void enqueue_on_thread_pool(ThreadPool &thread_pool, std::list<JobChain::JobT> const &jobs)
{
    thread_pool.enqueue_job(
        [&thread_pool, jobs = jobs]() mutable
        {
            if (!jobs.empty())
            {
                JobChain::JobT job = jobs.front();
                jobs.pop_front();

                job();

                enqueue_on_thread_pool(thread_pool, jobs);
            }
        }
    );
}

void JobChain::dispatch(ThreadPool &thread_pool) const
{
    enqueue_on_thread_pool(thread_pool, m_jobs);
}
