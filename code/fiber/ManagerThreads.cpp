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
#ifdef _WIN32
#include <Windows.h>
#else
#error Linux is not supported!
#endif

uint8_t nmd::fiber::Manager::GetCurrentThreadIndex() const
{
#ifdef _WIN32
	uint32_t idx = GetCurrentThreadId();
	for (uint8_t i = 0; i < _numThreads; i++) {
		if (_threads[i].GetID() == idx) {
			return i;
		}
	}
#endif
	// TODO macos/linux impl

	return UINT8_MAX;
}

nmd::fiber::Thread* nmd::fiber::Manager::GetCurrentThread() const
{
#ifdef _WIN32
	uint32_t idx = GetCurrentThreadId();
	for (uint8_t i = 0; i < _numThreads; i++) {
		if (_threads[i].GetID() == idx) {
			return &_threads[i];
		}
	}
#endif
	// TODO macos/linux impl

	return nullptr;
}

nmd::fiber::TLS* nmd::fiber::Manager::GetCurrentTLS() const
{
#ifdef _WIN32
	uint32_t idx = GetCurrentThreadId();
	for (uint8_t i = 0; i < _numThreads; i++) {
		if (_threads[i].GetID() == idx) {
			return _threads[i].GetTLS();
		}
	}
#endif
	// TODO macos/linux impl

	return nullptr;
}