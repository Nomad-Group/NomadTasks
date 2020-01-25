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
#include "Counter.h"
#include "Job.h" // Priority

namespace nmd::fiber
{
	class Manager;
	class Counter;

	class List
	{
		Manager* _manager;
		JobPriority _defaultPriority;

		Counter _counter;

	public:
		List(Manager*, JobPriority defaultPriority = JobPriority::Normal);
		~List();

		// Add
		void Add(JobPriority, JobInfo);

		inline void Add(const JobInfo& job)
		{
			Add(_defaultPriority, job);
		}

		template <typename... Args>
		inline void Add(JobPriority prio, Args... args)
		{
			_manager->ScheduleJob(prio, &_counter, args...);
		}

		template <typename... Args>
		inline void Add(Args... args)
		{
			_manager->ScheduleJob(_defaultPriority, &_counter, args...);
		}

		List& operator+=(const JobInfo&);

		// Wait
		void Wait(uint32_t targetValue = 0);
	};
}