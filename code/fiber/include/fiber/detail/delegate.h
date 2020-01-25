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
#include <tuple>
#include "detail.h"
#include "function_checks.h"

namespace nmd::fiber
{
	namespace detail
	{
		struct base_delegate
		{
			virtual ~base_delegate() = default;
			virtual void Call() = 0;
		};

		// callable: function pointer or lambda class
		template <typename TCallable, typename... Args>
		struct delegate_callable : base_delegate
		{
			TCallable _callable;
			std::tuple<Args...> _args;

			delegate_callable(TCallable callable, Args... args) :
				_callable(callable),
				_args(args...)
			{};

			virtual ~delegate_callable() = default;

			virtual void Call() override
			{
				apply(_callable, _args);
			}
		};

		// member: member function
		template <class TClass, typename Ret, typename... Args>
		struct delegate_member : base_delegate
		{
			using function_t = Ret(TClass::*)(Args...);
			function_t _function;
			TClass* _instance;
			std::tuple<Args...> _args;

			delegate_member(function_t function, TClass* inst, Args... args) :
				_function(function),
				_instance(inst),
				_args(args...)
			{};

			virtual ~delegate_member() = default;

			virtual void Call() override
			{
				apply(_instance, _function, _args);
			}
		};
	}
}