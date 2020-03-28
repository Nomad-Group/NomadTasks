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

#include <fiber/Counter.h>
#include <fiber/Exception.h>
#include <fiber/Manager.h>
#include <fiber/TLS.h>

nmd::fiber::detail::JobQueue *nmd::fiber::Manager::GetQueueByPriority(JobPriority prio)
{
	switch (prio) {
	case JobPriority::High:
		return &_highPriorityQueue;

	case JobPriority::Normal:
		return &_normalPriorityQueue;

	case JobPriority::Low:
		return &_lowPriorityQueue;

	case JobPriority::IO:
		return &_ioQueue;

	default:
		return nullptr;
	}
}

bool nmd::fiber::Manager::GetNextJob(JobInfo &job, TLS *tls)
{
	if (tls == nullptr) {
		tls = GetCurrentTLS();
	}

	// IO only does IO jobs
	if (tls->_isIO) {
		return _ioQueue.dequeue(job);
	}

	// High Priority Jobs always come first
	if (_highPriorityQueue.dequeue(job)) {
		return true;
	}

	// Ready Fibers
	for (auto it = tls->_readyFibers.begin(); it != tls->_readyFibers.end(); ++it) {
		uint16_t fiberIndex = it->first;

		// Make sure Fiber is stored
		if (!it->second->load(std::memory_order_relaxed))
			continue;

		// Erase
		delete it->second;
		tls->_readyFibers.erase(it);

		// Update TLS
		tls->_previousFiberIndex = tls->_currentFiberIndex;
		tls->_previousFiberDestination = FiberDestination::Pool;
		tls->_currentFiberIndex = fiberIndex;

		// Switch to Fiber
		tls->_threadFiber.SwitchTo(&_fibers[fiberIndex], this);
		CleanupPreviousFiber(tls);

		break;
	}

	// Normal & Low Priority Jobs
	return _normalPriorityQueue.dequeue(job) || _lowPriorityQueue.dequeue(job);
}

void nmd::fiber::Manager::ScheduleJob(JobPriority prio, const JobInfo &job)
{
	auto queue = GetQueueByPriority(prio);
	if (!queue) {
		return;
	}

	if (job.GetCounter()) {
		job.GetCounter()->Increment();
	}

	if (!queue->enqueue(job)) {
		throw nmd::fiber::Exception("EX_JOB_QUEUE_FULL");
	}
}

#include <condition_variable>

void WaitForCounter_Proxy(nmd::fiber::Manager* mgr, nmd::fiber::detail::BaseCounter* counter, uint32_t targetValue, std::condition_variable* cv)
{
	mgr->WaitForCounter(counter, targetValue, false);
	cv->notify_all();
}

void nmd::fiber::Manager::WaitForCounter(detail::BaseCounter *counter, uint32_t targetValue, bool blocking)
{
	if (counter == nullptr || counter->GetValue() == targetValue) {
		return;
	}

	auto tls = GetCurrentTLS();
	if (blocking) {
		if (counter->GetValue() == targetValue) {
			return;
		}

		std::condition_variable cv;
		ScheduleJob(JobPriority::High, WaitForCounter_Proxy, this, counter, targetValue, &cv);

		std::mutex mutex;
		std::unique_lock<std::mutex> lock(mutex);
		cv.wait(lock);

		return;
	}

	auto fiberStored = new std::atomic_bool(false);
		
	// Check if we're already done
	if (counter->AddWaitingFiber(tls->_currentFiberIndex, targetValue, fiberStored)) {
		delete fiberStored;
		return;
	}

	// Update TLS
	tls->_previousFiberIndex = tls->_currentFiberIndex;
	tls->_previousFiberDestination = FiberDestination::Waiting;
	tls->_previousFiberStored = fiberStored;

	// Switch to idle Fiber
	tls->_currentFiberIndex = FindFreeFiber();
	tls->_threadFiber.SwitchTo(&_fibers[tls->_currentFiberIndex], this);

	// Cleanup
	CleanupPreviousFiber();
}

void nmd::fiber::Manager::WaitForSingle(JobPriority prio, JobInfo info)
{
	nmd::fiber::detail::TinyCounter ctr(this);
	info.SetCounter(&ctr);

	ScheduleJob(prio, info);
	WaitForCounter(&ctr);
}