#pragma once

#include <functional>

#include "ThreadPool.hpp"

namespace explo
{
	/// An utility class used to avoid callbacks nesting for sequential jobs dispatch.
	class JobChain
	{
	public:
		using JobT = std::function<void()>;

	private:
		std::list<JobT> m_jobs;

	public:
		explicit JobChain();
		~JobChain();

		JobChain& then(JobT const& job);

		void dispatch() const;
		void dispatch(ThreadPool& thread_pool) const;
	};


} // namespace explo
