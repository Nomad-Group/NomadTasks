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

		std::vector<IJob *> _jobs;

	  public:
		JobSystem() = default;
		virtual ~JobSystem() = default;
		static std::unique_ptr<JobSystem> Create();

		void Run()
		{
			std::vector<IJob *> jobsToRun;

			{
				std::lock_guard<std::recursive_mutex> lk{ _executionMutex };
				std::swap(jobsToRun, _jobs);
			}

			for (auto job : jobsToRun) {
				job->Execute();
			}
		}

		void Reset()
		{
			_jobs.clear();
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
			_jobs.emplace_back(n);
			return *n;
		}

		void AddInternal(IJob *fn)
		{
			std::lock_guard<std::recursive_mutex> lk{ _executionMutex };
			_jobs.push_back(fn);
		}
	};
} // namespace nmd::job
