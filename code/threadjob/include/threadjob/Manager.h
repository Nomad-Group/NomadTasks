/*
 * All or portions of this file Copyright (c) NOMAD Group<nomad-group.net> or its affiliates or
 * its licensors.
 *
 * For complete copyright and license terms please see the LICENSE at the root of this
 * distribution (the "License"). All use of this software is governed by the License,
 * or, if provided, by the license below or the license accompanying this file. Do not
 * remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *
 */

#pragma once

#include <threadjob/DelayedJob.h> // includes Job.h and IJob.h
#include <threadjob/JobFunctionTraits.h>
#include <functional>
#include <memory>

namespace nmd::job
{
	class JobSystem
	{
	  private:
		std::recursive_mutex _executionMutex;

		std::vector<IJob *> _jobs = {};
		std::vector<IJob *> _nextJobs = {};

	  public:
		JobSystem() = default;
		virtual ~JobSystem() = default;
		static std::unique_ptr<JobSystem> Create();

		// If you want to run more jobs, change the default val in the func header
		void Run(uint32_t jobsToExecute = 250)
		{
			uint32_t jobsExecuted = 0;

			std::vector<IJob *> runnedJobs;

			{
				std::lock_guard<std::recursive_mutex> lk{ _executionMutex };
				_jobs = _nextJobs;
			}

			auto it = _jobs.begin();

			// Run jobs and remove them from list
			for (uint32_t n = 0; n < jobsToExecute; ++n) {
				if (_jobs.empty() || it == _jobs.end()) {
					break;
				}
				
				auto fn = *it;

				if ((*fn)(this)) {
					delete fn;
					runnedJobs.push_back(fn);
					it = _jobs.erase(it);
				} else {
					++it;
				}

				++jobsToExecute;
			}

			// Move jobs to other vectors for next run and clear jobs queue
			{
				std::lock_guard<std::recursive_mutex> lk{ _executionMutex };
				for (auto &it : runnedJobs) {
					_nextJobs.erase(std::remove(_nextJobs.begin(), _nextJobs.end(), it), _nextJobs.end());
				}
			}

			_jobs.clear();
		}

		void Reset()
		{
			_jobs.clear();
			_nextJobs.clear();
		}

		void Add(IJob *job)
		{
			AddInternal(job);
		}

		template <typename Function, typename... Args>
		IJob &Add(Function &&func, Args &&... args)
		{
			using rt = typename job_function_traits<std::decay_t<Function>>::return_type;
			std::function<rt()> fn = std::bind(func, std::forward<Args>(args)...);
			return AddInternal<rt>(fn);
		}
		
		template <typename Function, typename... Args>
		IJob &AddJob(Function &&func, Args &&... args)
		{
			using rt = typename job_function_traits<std::decay_t<Function>>::return_type;
			std::function<rt()> fn = std::bind(func, std::forward<Args>(args)...);
			return AddInternal<rt>(fn);
		}

		template <typename Function, typename... Args>
		IJob &AddJobFor(std::chrono::milliseconds f, Function &&func, Args &&... args)
		{
			using rt = typename job_function_traits<std::decay_t<Function>>::return_type;
			std::function<rt()> fn = std::bind(func, std::forward<Args>(args)...);

			auto n = new DelayedJob<rt>(fn, std::chrono::milliseconds(0), f);
			AddInternal(n);
			return *n;
		}

		template <typename Function, typename... Args>
		IJob &AddDelayedJob(std::chrono::milliseconds in, Function &&func, Args &&... args)
		{
			using rt = typename job_function_traits<std::decay_t<Function>>::return_type;
			std::function<rt()> fn = std::bind(func, std::forward<Args>(args)...);

			auto n = new DelayedJob<rt>(fn, in);
			AddInternal(n);
			return *n;
		}

		template <typename Function, typename... Args>
		IJob &AddDelayedJobFor(std::chrono::milliseconds in, std::chrono::milliseconds f, Function &&func, Args &&... args)
		{
			using rt = typename job_function_traits<std::decay_t<Function>>::return_type;
			std::function<rt()> fn = std::bind(func, std::forward<Args>(args)...);

			auto n = new DelayedJob<rt>(fn, in, f);
			AddInternal(n);
			return *n;
		}

	  private:
		template <typename T>
		IJob &AddInternal(std::function<T()> fn)
		{
			std::lock_guard<std::recursive_mutex> lk{ _executionMutex };
			auto n = new Job<T>(fn);
			_nextJobs.emplace_back(n);
			return *n;
		}

		void AddInternal(IJob *fn)
		{
			std::lock_guard<std::recursive_mutex> lk{ _executionMutex };
			_nextJobs.push_back(fn);
		}
	};
} // namespace nmd::job
