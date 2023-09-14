#pragma once

#include <bitset>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <functional>
#include <thread>
#include <vector>

namespace explo
{
    class ThreadPool
    {
       public:
        inline static constexpr size_t k_max_threads = 32;

       private:
        std::vector<std::thread> m_threads;
        std::deque<std::function<void()>> m_jobs;
        std::condition_variable m_condition_variable;
        std::condition_variable m_drained_condition;

        std::mutex m_mutex;

        bool m_should_terminate = false;

        size_t m_next_job_id = 0;

        std::bitset<k_max_threads> m_thread_working;

       public:
        explicit ThreadPool(size_t num_threads);
        explicit ThreadPool();
        ~ThreadPool();

        size_t get_thread_count() const;
        bool is_thread_working(size_t thread_id);

        size_t get_job_count();
        size_t enqueue_job(std::function<void()> const &job);
        void drop_job(size_t job_id);

        void drain();

       private:
        void thread_loop(size_t thread_id);
    };
}  // namespace explo
