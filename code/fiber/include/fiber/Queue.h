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
#include <vector>

namespace nmd::fiber
{
	class Manager;
	class Counter;

	class Queue
	{
		Manager* _manager;
		JobPriority _defaultPriority;

		Counter _counter;
		std::vector<std::pair<JobPriority, JobInfo>> _queue;

	public:
		Queue(Manager*, JobPriority defaultPriority = JobPriority::Normal);
		~Queue();

		// Add
		void Add(JobPriority, JobInfo);

		inline void Add(const JobInfo& job)
		{
			Add(_defaultPriority, job);
		}

		template <typename... Args>
		inline void Add(JobPriority prio, Args... args)
		{
			_queue.emplace_back(prio, JobInfo(&_counter, args...));
		}

		template <typename... Args>
		inline void Add(Args... args)
		{
			_queue.emplace_back(_defaultPriority, JobInfo(&_counter, args...));
		}

		Queue& operator+=(const JobInfo&);
		Queue& operator+=(JobInfo &&);

		// Execute all Jobs in Queue
		void Execute();

		// Execute first Job in Queue
		// returns false if queue is empty
		bool Step();
	};
}