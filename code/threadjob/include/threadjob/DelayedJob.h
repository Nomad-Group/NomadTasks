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

#include <threadjob/Job.h>

namespace nmd::job
{
	template <typename _Ret>
	class DelayedJob : public Job<_Ret>
	{
	  private:
		std::chrono::system_clock::time_point _start;
		std::chrono::system_clock::time_point _end;

		virtual bool ShouldRemove() override
		{
			return (std::chrono::system_clock::now() >= _end);
		}

	  protected:
		virtual bool Execute() override
		{
			if (_start < std::chrono::system_clock::now()) {
				return Job::Execute();
			}

			return false;
		}

	  public:
		DelayedJob(std::function<_Ret()> fn, std::chrono::milliseconds in, std::chrono::milliseconds f = std::chrono::milliseconds(0))
			: Job(fn)
		{
			_start = std::chrono::system_clock::now() + in;
			_end = std::chrono::system_clock::now() + in + f;
		}
	};
} // namespace nmd::job