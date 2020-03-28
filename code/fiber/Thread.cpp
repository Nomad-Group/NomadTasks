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

#include <fiber/Exception.h>
#include <fiber/Fiber.h>
#include <fiber/Thread.h>
#include <optick.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#if LINUX
#include <libgen.h>
#include <signal.h>
#include <unistd.h>
#endif

#if MACOS
#include <unistd.h>
#endif

// TODO overall macos/linux impl

#ifdef _WIN32
static void WINAPI LaunchThread(void *ptr)
{
	auto thread = reinterpret_cast<nmd::fiber::Thread *>(ptr);
	auto callback = thread->GetCallback();

	if (callback == nullptr) {
		throw nmd::fiber::Exception("LaunchThread: callback is nullptr");
	}

	thread->WaitForReady();
	callback(thread);
}
#endif

bool nmd::fiber::Thread::Spawn(Callback_t callback, void *userdata)
{
	_handle = nullptr;
	_id = UINT32_MAX;
	_callback = callback;
	_userdata = userdata;

	{
		std::lock_guard lock(_startupIdMutex);
#ifdef _WIN32
		_handle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)LaunchThread, this, 0, (DWORD *)&_id);
#endif
	}

	_cvReceivedId.notify_all();
	return HasSpawned();
}

void nmd::fiber::Thread::SetAffinity(size_t i)
{
#ifdef _WIN32
	if (!HasSpawned()) {
		return;
	}

	DWORD_PTR mask = 1ull << i;
	SetThreadAffinityMask(_handle, mask);
#endif
}

void nmd::fiber::Thread::Join()
{
	if (!HasSpawned()) {
		return;
	}

#ifdef _WIN32
	WaitForSingleObject(_handle, INFINITE);
#endif
}

void nmd::fiber::Thread::FromCurrentThread()
{
	_handle = GetCurrentThread();
	_id = GetCurrentThreadId();
}

void nmd::fiber::Thread::WaitForReady()
{
	// Check if we have an ID already
	{
		std::lock_guard lock(_startupIdMutex);
		if (_id != UINT32_MAX) {
			return;
		}
	}

	// Wait
	std::mutex mutex;

	std::unique_lock<std::mutex> lock(mutex);
	_cvReceivedId.wait(lock);
}

void nmd::fiber::Thread::SleepFor(uint32_t ms)
{
#ifdef _WIN32
	Sleep(ms);
#elif LINUX
	sleep(ms);
#elif MACOS
	usleep(ms * 1000); // unix takes microseconds, not milliseconds (https://linux.die.net/man/3/usleep)
#endif
}