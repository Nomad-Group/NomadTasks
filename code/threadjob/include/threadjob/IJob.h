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

#include <queue>
#include <mutex>
#include <functional>

namespace nmd::job
{
	class JobSystem;

	template <typename _Ret>
	class Job;

	class IJob
	{
	  protected:
		IJob *_next = nullptr;

		virtual bool Run(JobSystem*) = 0;

	  public:
		virtual bool Execute() = 0;

		bool operator()(JobSystem* mgr)
		{
			return Run(mgr);
		}

		template <typename Function, typename... Args>
		IJob &then(Function &&func, Args &&... args)
		{
			using rt = typename function_traits<Function>::return_type;
			std::function<rt()> fn = std::bind(func, std::forward<Args>(args)...);
			_next = new Job<rt>(fn);
			return *_next;
		}

		template <typename T>
		IJob &then(std::function<T()> fn)
		{
			_next = new Job<T>(fn);
			return *_next;
		}
	};
} // namespace nmd::job