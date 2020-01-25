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

#include <fiber/Manager.h>
#include <fiber/Thread.h>
#include <fiber/Fiber.h>
#include <fiber/Counter.h>
#include <optick.h>

void nmd::fiber::Manager::ThreadCallback_Worker(nmd::fiber::Thread* thread)
{
	auto manager = reinterpret_cast<nmd::fiber::Manager*>(thread->GetUserdata());
	auto tls = thread->GetTLS();

	// Thread Name
	if (tls->_hasAffinity) {
		thread->SetAffinity(tls->_threadIndex);
		OPTICK_START_THREAD("Worker (CPU-bound)");
	} else if (tls->_isIO) {
		OPTICK_START_THREAD("Worker (IO)");
	} else {
		OPTICK_START_THREAD("Worker");
	}

	// Setup Thread Fiber
	tls->_threadFiber.FromCurrentThread();
	tls->_currentFiberIndex = manager->FindFreeFiber();

	const auto fiber = &manager->_fibers[tls->_currentFiberIndex];
	tls->_threadFiber.SwitchTo(fiber, manager);

	OPTICK_STOP_THREAD();
}

void nmd::fiber::Manager::FiberCallback_Main(nmd::fiber::Fiber* fiber)
{
	auto manager = reinterpret_cast<nmd::fiber::Manager*>(fiber->GetUserdata());
	
	if (manager->_mainCallback) {
		manager->_mainCallback(manager);
	}

	// Should we shutdown after main?
	if (!manager->_shutdownAfterMain && manager->_mainCallback) {
		// Switch to idle Fiber
		const auto tls = manager->GetCurrentTLS();
		tls->_currentFiberIndex = manager->FindFreeFiber();
		
		const auto fiber = &manager->_fibers[tls->_currentFiberIndex];
		tls->_threadFiber.SwitchTo(fiber, manager);
	}

	if (manager->_shutdownAfterMain) {
		manager->Shutdown(false);
	}

	// Switch back to Thread
	fiber->SwitchBack();
}

void nmd::fiber::Manager::FiberCallback_Worker(nmd::fiber::Fiber* fiber)
{
	auto manager = reinterpret_cast<nmd::fiber::Manager*>(fiber->GetUserdata());
	manager->CleanupPreviousFiber();

	JobInfo job;

	while (!manager->IsShuttingDown()) {
		auto tls = manager->GetCurrentTLS();
		
		if (manager->GetNextJob(job, tls)) {
			job.Execute();
			continue;
		}

		Thread::SleepFor(1);
	}
	
	// Switch back to Thread
	fiber->SwitchBack();
}