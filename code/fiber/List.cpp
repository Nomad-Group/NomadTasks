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

#include <fiber/List.h>
#include <fiber/Manager.h>

nmd::fiber::List::List(nmd::fiber::Manager* mgr, JobPriority defaultPriority) :
	_manager(mgr),
	_defaultPriority(defaultPriority),
	_counter(mgr)
{}

nmd::fiber::List::~List()
{}

void nmd::fiber::List::Add(JobPriority prio, JobInfo job)
{
	job.SetCounter(&_counter);

	_manager->ScheduleJob(prio, job);
}

nmd::fiber::List& nmd::fiber::List::operator+=(const JobInfo& job)
{
	Add(_defaultPriority, job);
	return *this;
}

void nmd::fiber::List::Wait(uint32_t targetValue)
{
	_manager->WaitForCounter(&_counter, targetValue);
}