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
#include <thread>

nmd::fiber::Manager::Manager(const ManagerOptions& options) :
	_numThreads(options.NumThreads + 1 /* IO */),
	_hasThreadAffinity(options.ThreadAffinity),
	_autoSpawnThreads(options.AutoSpawnThreads),
	_numFibers(options.NumFibers),
	_highPriorityQueue(options.HighPriorityQueueSize),
	_normalPriorityQueue(options.NormalPriorityQueueSize),
	_lowPriorityQueue(options.LowPriorityQueueSize),
	_ioQueue(options.IOQueueSize),
	_shutdownAfterMain(options.ShutdownAfterMainCallback)
{}

nmd::fiber::Manager::~Manager()
{
	delete[] _threads;
	delete[] _fibers;
	delete[] _idleFibers;
}

nmd::fiber::Thread *nmd::fiber::Manager::GetThread(uint8_t idx)
{
	assert(idx < _numThreads);
	return &_threads[idx];
}

bool nmd::fiber::Manager::SpawnThread(uint8_t idx)
{
	return GetThread(idx)->Spawn(ThreadCallback_Worker, this);
}

bool nmd::fiber::Manager::SetupThread(uint8_t idx)
{
	auto thread = GetThread(idx);
	if (thread->HasSpawned()) {
		return false;
	}

	auto tls = GetCurrentTLS();
	if (tls) {
		return false;
	}

	thread->FromCurrentThread();
	tls = GetCurrentTLS();
	assert(tls->_threadIndex == idx);

	tls->_threadFiber.FromCurrentThread();
	tls->_currentFiberIndex = FindFreeFiber();
	
	return true;
}

nmd::fiber::Manager::ReturnCode nmd::fiber::Manager::Run(Main_t main)
{
	if (_threads || _fibers) {
		return ReturnCode::AlreadyInitialized;
	}

	_threads = new Thread[_numThreads];

	// Current (Main) Thread
	auto mainThread = &_threads[0];
	mainThread->FromCurrentThread();

	TLS *mainThreadTLS = mainThread->GetTLS();
	mainThreadTLS->_threadFiber.FromCurrentThread();

	if (main) {
		if (_hasThreadAffinity) {
			mainThread->SetAffinity(1);
		}
	}

	// Create Fibers
	// This has to be done after Thread is converted to Fiber!
	if (_numFibers == 0) {
		return ReturnCode::InvalidNumFibers;
	}

	_fibers = new Fiber[_numFibers];
	_idleFibers = new std::atomic_bool[_numFibers];

	for (uint16_t i = 0; i < _numFibers; i++) {
		_fibers[i].SetCallback(FiberCallback_Worker);
		_idleFibers[i].store(true, std::memory_order_relaxed);
	}

	// Thread Affinity
	if (_hasThreadAffinity && (_numThreads == 0 || _numThreads > std::thread::hardware_concurrency() + 1)) {
		return ReturnCode::ErrorThreadAffinity;
	}

	// Spawn Threads
	for (uint8_t i = 0; i < _numThreads; i++) {
		auto itTls = _threads[i].GetTLS();
		itTls->_threadIndex = i;

		if (i > 0) // 0 is Main Thread
		{
			if (i == (_numThreads - 1)) {
				// IO Thread
				itTls->_isIO = true;
			} else {
				itTls->_hasAffinity = _hasThreadAffinity;
			}

			if (_autoSpawnThreads && !SpawnThread(i)) {
				return ReturnCode::OSError;
			}
		}
	}

	mainThreadTLS->_currentFiberIndex = FindFreeFiber();

	// Main
	_mainCallback = main;
	if (_mainCallback == nullptr && _shutdownAfterMain) {
		return ReturnCode::NullCallback;
	}

	// Setup main Fiber
	const auto mainFiber = &_fibers[mainThreadTLS->_currentFiberIndex];
	mainFiber->SetCallback(FiberCallback_Main);

	if (_mainCallback) {
		mainThreadTLS->_threadFiber.SwitchTo(mainFiber, this);
	}

	if (_mainCallback) {
		// Wait for all Threads to shut down
		for (uint8_t i = 1; i < _numThreads; i++) {
			_threads[i].Join();
		}
	}

	return ReturnCode::Succes;
}

void nmd::fiber::Manager::Shutdown(bool blocking)
{
	_shuttingDown.store(true, std::memory_order_release);

	if (blocking)
	{
		for (uint8_t i = 1; i < _numThreads; i++) {
			_threads[i].Join();
		}
	}
}

uint16_t nmd::fiber::Manager::FindFreeFiber()
{
	while (true)
	{
		for (uint16_t i = 0; i < _numFibers; i++)
		{
			if (!_idleFibers[i].load(std::memory_order_relaxed) 
				|| !_idleFibers[i].load(std::memory_order_acquire)) {
				continue;
			}

			bool expected = true;
			if (std::atomic_compare_exchange_weak_explicit(&_idleFibers[i], &expected, false, std::memory_order_release, std::memory_order_relaxed)) {
				return i;
			}
		}

		// TODO: Add Debug Counter and error message
	}
}

void nmd::fiber::Manager::CleanupPreviousFiber(TLS* tls)
{
	if (tls == nullptr) {
		tls = GetCurrentTLS();
	}
	
	switch (tls->_previousFiberDestination)
	{
	case FiberDestination::None:
		return;

	case FiberDestination::Pool:
		_idleFibers[tls->_previousFiberIndex].store(true, std::memory_order_release);
		break;

	case FiberDestination::Waiting:
		tls->_previousFiberStored->store(true, std::memory_order_relaxed);
		break;

	default:
		break;
	}

	tls->Cleanup();
}