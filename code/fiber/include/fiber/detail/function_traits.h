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

namespace nmd::fiber
{
	namespace detail
	{
		template<typename F>
		struct function_traits;

		// Function Pointer
		template <typename Ret, typename... Args>
		struct function_traits<Ret(*)(Args...)> : public function_traits<Ret(Args...)>
		{};

		// Member Function Pointer
		template <class T, typename Ret, typename... Args>
		struct function_traits<Ret(T::*)(Args...)> : public function_traits<Ret(T*, Args...)>
		{};
		template <class T, typename Ret, typename... Args>
		struct function_traits<Ret(T::*)(Args...) const> : public function_traits <Ret(T*, Args...)>
		{};

		// function_traits
		template <typename F>
		struct function_traits
		{
		private:
			using func_traits = function_traits<decltype(&F::operator())>;

			template <typename T>
			struct remove_this_arg;

			template <typename T, typename... Args>
			struct remove_this_arg<std::tuple<T, Args...>>
			{
				using type = std::tuple<Args...>;
			};

		public:
			using args = typename remove_this_arg<typename func_traits::args>::type;
			using return_type = typename func_traits::return_type;

			static constexpr size_t num_args = func_traits::num_args - 1; // instance

			template <size_t N>
			struct argument
			{
				static_assert(N < num_args, "function_traits: Invalid Argument Index");
				using type = typename func_traits::template argument<N + 1>::type;
			};
		};

		template<typename Ret, typename... Args>
		struct function_traits<Ret(Args...)>
		{
		public:
			using args = typename std::tuple<Args...>;
			using return_type = Ret;

			static constexpr size_t num_args = sizeof...(Args);

			template <size_t N>
			struct argument
			{
				static_assert(N < num_args, "function_traits: Invalid Argument Index");
				using type = typename std::tuple_element<N, typename args>::type;
			};
		};
	}
}