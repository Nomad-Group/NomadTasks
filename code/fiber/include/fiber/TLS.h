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
#include <stdint.h>
#include <vector>
#include <atomic>
#include "Fiber.h"

namespace nmd::fiber
{
	enum class FiberDestination : uint8_t
	{
		None,
		Waiting,
		Pool
	};

	struct TLS
	{
		TLS() = default;
		~TLS() = default;

		// Thread Index
		uint8_t _threadIndex = UINT8_MAX;
		bool _hasAffinity = false;
		bool _isIO = false;

		// Thread Fiber
		Fiber _threadFiber;

		// Current Fiber
		uint16_t _currentFiberIndex = UINT16_MAX;

		// Previous Fiber
		uint16_t _previousFiberIndex = UINT16_MAX;
		std::atomic_bool* _previousFiberStored = nullptr;
		FiberDestination _previousFiberDestination = FiberDestination::None;

		// Ready Fibers
		std::vector<std::pair<uint16_t, std::atomic_bool*>> _readyFibers;

		inline void Cleanup() {
			_previousFiberIndex = UINT16_MAX;
			_previousFiberDestination = FiberDestination::None;
			_previousFiberStored = nullptr;
		}
	};
}