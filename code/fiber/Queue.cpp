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

#include <fiber/Queue.h>
#include <fiber/Manager.h>

nmd::fiber::Queue::Queue(nmd::fiber::Manager* mgr, JobPriority defaultPriority) :
	_manager(mgr),
	_defaultPriority(defaultPriority),
	_counter(mgr)
{}

nmd::fiber::Queue::~Queue()
{}

void nmd::fiber::Queue::Add(JobPriority prio, JobInfo job)
{
	job.SetCounter(&_counter);
	_queue.emplace_back(prio, job);
}

nmd::fiber::Queue& nmd::fiber::Queue::operator+=(const JobInfo& job)
{
	Add(_defaultPriority, job);
	return *this;
}

nmd::fiber::Queue& nmd::fiber::Queue::operator+=(JobInfo &&job)
{
	job.SetCounter(&_counter);
	_queue.emplace_back(_defaultPriority, job);

	return *this;
}

bool nmd::fiber::Queue::Step()
{
	if (_queue.empty()) {
		return false;
	}

	const auto& job = _queue.front();
	_manager->ScheduleJob(job.first, job.second);
	_manager->WaitForCounter(&_counter);

	_queue.erase(_queue.begin());
	return true;
}

void nmd::fiber::Queue::Execute()
{
	while (Step())
		;
}