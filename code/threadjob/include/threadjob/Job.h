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

#include <mutex>
#include <queue>

#include <threadjob/IJob.h>

namespace nmd::job
{
	class JobSystem;

	template <typename _Ret>
	class Job : public IJob
	{
	  private:
		std::function<_Ret()> _callback;
		bool _resultK = true;

		virtual bool ShouldRemove()
		{
			return _resultK;
		}

		template <typename Ret = _Ret>
		typename std::enable_if<std::is_same<Ret, bool>::value, bool>::type ExecuteInternal()
		{
			_resultK = _callback();
			return _resultK;
		}

		template <typename Ret = _Ret>
		typename std::enable_if<!std::is_same<Ret, bool>::value, bool>::type ExecuteInternal()
		{
			_callback();
			return true;
		}

		virtual bool Run(JobSystem* mgr) override
		{
			if (!Execute()) {
				return false;
			}

			if (_next) {
				mgr->Add(_next);
			}

			return ShouldRemove();
		}

	  protected:
		virtual bool Execute() override
		{
			return ExecuteInternal();
		}

	  public:
		Job(std::function<_Ret()> fn)
			: _callback(fn)
		{
		}

		bool operator()(JobSystem *mgr)
		{
			return Run(mgr);
		}
	};
} // namespace nmd::job
