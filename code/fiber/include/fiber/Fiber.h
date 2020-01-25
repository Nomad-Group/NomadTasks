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

namespace nmd::fiber
{
	class Fiber
	{
	public:
		using Callback_t = void(*)(Fiber*);

	private:
		void* _fiber = nullptr;
		bool _threadFiber = false;

		Fiber* _returnFiber = nullptr;

		Callback_t _callback = nullptr;
		void* _userData = nullptr;

		Fiber(void* fiber) :
			_fiber(fiber)
		{};

	public:
		Fiber();
		Fiber(const Fiber&) = delete;
		~Fiber();

		// Converts current Thread to a Fiber
		void FromCurrentThread();

		// Set Callback
		void SetCallback(Callback_t);

		// Fiber Switching
		void SwitchTo(Fiber*, void* = nullptr);
		void SwitchBack();
		
		// Getter
		inline Callback_t GetCallback() const { return _callback; };
		inline void* GetUserdata() const { return _userData; };
		inline bool IsValid() const { return _fiber && _callback; };
	};
}