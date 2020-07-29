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
#include <fiber/Counter.h>
#include <fiber/Manager.h>
#include <fiber/TLS.h>

nmd::fiber::detail::BaseCounter::BaseCounter(Manager* mgr, uint8_t numWaitingFibers, WaitingFibers* waitingFibers, std::atomic_bool* freeWaitingSlots) :
	_manager(mgr),
	_numWaitingFibers(numWaitingFibers),
	_waitingFibers(waitingFibers),
	_freeWaitingSlots(freeWaitingSlots)
{
}

void nmd::fiber::detail::BaseCounter::InternalInit()
{
	for (uint8_t i = 0; i < _numWaitingFibers; i++) {
		_freeWaitingSlots[i] = true;
	}
}

nmd::fiber::Counter::Counter(Manager* mgr) :
	BaseCounter(mgr, MAX_WAITING, _impl_waitingFibers, _impl_freeWaitingSlots)
{
	InternalInit();
}

nmd::fiber::detail::TinyCounter::TinyCounter(Manager* mgr) :
	BaseCounter(mgr, 1, &_waitingFiber, &_freeWaitingSlot)
{
	InternalInit();
}

nmd::fiber::Counter::Unit_t nmd::fiber::detail::BaseCounter::Increment(Unit_t by)
{
	Unit_t prev = _counter.fetch_add(by);
	CheckWaitingFibers(prev + by);

	return prev;
}

nmd::fiber::Counter::Unit_t nmd::fiber::detail::BaseCounter::Decrement(Unit_t by)
{
	Unit_t prev = _counter.fetch_sub(by);
	CheckWaitingFibers(prev - by);

	return prev;
}

nmd::fiber::Counter::Unit_t nmd::fiber::detail::BaseCounter::GetValue() const
{
	return _counter.load(std::memory_order_seq_cst);
}

bool nmd::fiber::detail::BaseCounter::AddWaitingFiber(uint16_t fiberIndex, Unit_t targetValue, std::atomic_bool* fiberStored)
{
	for (uint8_t i = 0; i < _numWaitingFibers; i++) {
		// Acquire Free Waiting Slot
		bool expected = true;
		if (!std::atomic_compare_exchange_strong_explicit(&_freeWaitingSlots[i], &expected, false, std::memory_order_seq_cst, std::memory_order_relaxed)) {
			continue;
		}

		// Setup Slot
		auto slot = &_waitingFibers[i];
		slot->_fiberIndex = fiberIndex;
		slot->_fiberStored = fiberStored;
		slot->_targetValue = targetValue;
		slot->_inUse.store(false);

		// Check if we are done already
		Unit_t counter = _counter.load(std::memory_order_relaxed);
		if (slot->_inUse.load(std::memory_order_acquire)) {
			return false;
		}

		if (slot->_targetValue != counter) {
			return false;
		}

		expected = false;
		if (!std::atomic_compare_exchange_strong_explicit(&slot->_inUse, &expected, true, std::memory_order_seq_cst, std::memory_order_relaxed)) {
			return false;
		}

		_freeWaitingSlots[i].store(true, std::memory_order_release);
		return true;
	}

	// Waiting Slots are full
	throw nmd::fiber::Exception("EX_COUNTER_WAITINGS_SLOTS_FULL");
}

void nmd::fiber::detail::BaseCounter::CheckWaitingFibers(Unit_t value)
{
	for (size_t i = 0; i < _numWaitingFibers; i++) {
		if (_freeWaitingSlots[i].load(std::memory_order_acquire)) {
			continue;
		}

		auto waitingSlot = &_waitingFibers[i];
		if (waitingSlot->_inUse.load(std::memory_order_acquire)) {
			continue;
		}

		if (waitingSlot->_targetValue != value) {
			continue;
		}

		bool expected = false;
		if (!std::atomic_compare_exchange_strong_explicit(&waitingSlot->_inUse, &expected, true, std::memory_order_seq_cst, std::memory_order_relaxed)) {
			continue;
		}

		_manager->GetCurrentTLS()->_readyFibers.emplace_back(waitingSlot->_fiberIndex, waitingSlot->_fiberStored);
		_freeWaitingSlots[i].store(true, std::memory_order_release);
	}
}