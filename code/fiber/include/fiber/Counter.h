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
#include <atomic>

namespace nmd::fiber
{
	class Manager;

	namespace detail
	{
		class BaseCounter
		{
			friend class Manager;

		protected:
			using Unit_t = uint32_t;

			// Counter
			std::atomic<Unit_t> _counter = 0;

			// Waiting Fibers
			struct WaitingFibers
			{
				uint16_t _fiberIndex = UINT16_MAX;
				std::atomic_bool* _fiberStored = nullptr;
				Unit_t _targetValue = 0;

				std::atomic_bool _inUse = true;
			};

			const uint8_t _numWaitingFibers = 0;
			WaitingFibers* _waitingFibers = nullptr;
			std::atomic_bool* _freeWaitingSlots = nullptr;

			Manager* _manager = nullptr;

			// Methods
		protected:
			bool AddWaitingFiber(uint16_t, Unit_t, std::atomic_bool*);
			void CheckWaitingFibers(Unit_t);

		public:
			BaseCounter(Manager* mgr, uint8_t numWaitingFibers, WaitingFibers* waitingFibers, std::atomic_bool* freeWaitingSlots);
			virtual ~BaseCounter() = default;

			// Modifiers
			Unit_t Increment(Unit_t by = 1);
			Unit_t Decrement(Unit_t by = 1);

			// Counter Value
			Unit_t GetValue() const;
		};

		struct TinyCounter : public detail::BaseCounter
		{
			TinyCounter(Manager*);
			~TinyCounter() = default;

			std::atomic_bool _freeWaitingSlot;
			WaitingFibers _waitingFiber;
		};
	}

	class Counter :
		public detail::BaseCounter
	{
	public:
		static const constexpr uint8_t MAX_WAITING = 4;
		
	private:
		std::atomic_bool _impl_freeWaitingSlots[MAX_WAITING];
		WaitingFibers _impl_waitingFibers[MAX_WAITING];

	public:
		Counter(Manager*);
		~Counter() = default;
	};
}