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

#include <fiber/Fiber.h>
#include <fiber/Exception.h>

#ifdef _WIN32
#include <Windows.h>
#endif

// TODO: Add exceptions for invalid stuff?

static void LaunchFiber(nmd::fiber::Fiber* fiber)
{
	auto callback = fiber->GetCallback();
	if (callback == nullptr) {
		throw nmd::fiber::Exception("LaunchFiber: callback is nullptr");
	}

	callback(fiber);
}

nmd::fiber::Fiber::Fiber()
{
#ifdef _WIN32
	_fiber = CreateFiber(0, (LPFIBER_START_ROUTINE)LaunchFiber, this);
	_threadFiber = false;
#endif
	// TODO mac/linux impl
}

nmd::fiber::Fiber::~Fiber()
{
#ifdef _WIN32
	if (_fiber && !_threadFiber) {
		DeleteFiber(_fiber);
	}
#endif
	// TODO mac/linux impl
}

void nmd::fiber::Fiber::FromCurrentThread()
{
#ifdef _WIN32
	if (_fiber && !_threadFiber) {
		DeleteFiber(_fiber);
	}

	_fiber = ConvertThreadToFiber(nullptr);
	_threadFiber = true;
#endif
	// TODO mac/linux impl
}

void nmd::fiber::Fiber::SetCallback(Callback_t cb)
{
	if (cb == nullptr) {
		//throw nmd::fiber::Exception("EX_CALLBACK_IS_NULL");
	}

	_callback = cb;
}

void nmd::fiber::Fiber::SwitchTo(nmd::fiber::Fiber* fiber, void* userdata)
{
	if (fiber == nullptr || fiber->_fiber == nullptr) {
		throw nmd::fiber::Exception("EX_INVALID_FIBER");
	}

	fiber->_userData = userdata;
	fiber->_returnFiber = this;

	SwitchToFiber(fiber->_fiber);
}

void nmd::fiber::Fiber::SwitchBack()
{
	if (_returnFiber && _returnFiber->_fiber) {
		SwitchToFiber(_returnFiber->_fiber);
		return;
	}

	throw nmd::fiber::Exception("EX_UNABLE_TO_SWITCH_BACK_FROM_FIBER");
}