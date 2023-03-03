#pragma once

#include <functional>

#include "ThreadPool.hpp"

namespace explo
{
	/// An utility class used to avoid callbacks nesting for sequential jobs dispatch.
	class JobChain
	{
	public:
		using Job = std::function<void()>;

	private:
		std::list<Job> m_jobs;

	public:
		explicit JobChain() = default;
		~JobChain() = default;

		JobChain& then(Job const& job);

		/// Executes the chained jobs synchronously.
		void execute() const;

		/// Dispatches the chained jobs asynchronously on the given thread pool.
		void dispatch_on_thread_pool(ThreadPool& thread_pool) const;

	private:
		void enqueue_async_job(ThreadPool& thread_pool, Job const& job, Job const& next_job);
	};


} // namespace explo
