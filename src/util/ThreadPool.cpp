#include "ThreadPool.hpp"

#include "log.hpp"

using namespace explo;

ThreadPool::ThreadPool(size_t num_threads)
{
    m_threads.reserve(num_threads);

    for (size_t thread_id = 0; thread_id < num_threads; thread_id++)
    {
        m_threads.emplace_back(
            [this, thread_id]
            {
                thread_loop(thread_id);
            }
        );
    }
}

ThreadPool::ThreadPool() :
    ThreadPool(std::thread::hardware_concurrency() > 0 ? std::thread::hardware_concurrency() : 8)
{
}

ThreadPool::~ThreadPool()
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_should_terminate = true;
    }

    m_condition_variable.notify_all();

    for (std::thread &thread : m_threads) thread.join();
}

size_t ThreadPool::get_thread_count() const
{
    return m_threads.size();
}

bool ThreadPool::is_thread_working(size_t thread_id)
{
    // Here we use the same mutex used to synchronize the queue. We could improve it by locking on a different mutex
    std::lock_guard<std::mutex> lock(m_mutex);

    return m_thread_working.test(thread_id);
}

size_t ThreadPool::get_job_count()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    return m_jobs.size();
}

size_t ThreadPool::enqueue_job(std::function<void()> const &job)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    size_t job_id = m_next_job_id++;
    m_jobs.push_back(job);

    m_condition_variable.notify_all();

    return job_id;
}

void ThreadPool::drop_job(size_t job_id)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_jobs.erase(m_jobs.begin() + job_id);
}

void ThreadPool::drain()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_drained_condition.wait(
        lock,
        [this]
        {
            return m_jobs.empty();
        }
    );
}

void ThreadPool::thread_loop(size_t thread_id)
{
    while (true)
    {
        std::function<void()> job;
        {
            std::unique_lock<std::mutex> lock(m_mutex);

            m_thread_working[thread_id] = false;

            // If after executing the job we find out that there are no jobs left, we signal that the jobs queue is drained
            if (m_jobs.empty()) m_drained_condition.notify_all();

            m_condition_variable.wait(
                lock,
                [this]
                {
                    return m_jobs.size() > 0;
                }
            );

            if (m_should_terminate) return;

            job = m_jobs.front();
            m_jobs.pop_front();

            m_thread_working[thread_id] = true;
        }

        job();
    }
}
