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

namespace nmd::job
{
	template <typename F>
	struct job_function_traits;

	// function pointer
	template <typename R, typename... Args>
	struct job_function_traits<R (*)(Args...)> : public job_function_traits<R(Args...)> {
	};

	// member function pointer
	template <typename C, typename R, typename... Args>
	struct job_function_traits<R (C::*)(Args...)> : public job_function_traits<R(C &, Args...)> {
	};

	// const member function pointer
	template <typename C, typename R, typename... Args>
	struct job_function_traits<R (C::*)(Args...) const> : public job_function_traits<R(C &, Args...)> {
	};

	// member object pointer
	template <typename C, typename R>
	struct job_function_traits<R(C::*)> : public job_function_traits<R(C &)> {
	};

	// functor
	template <typename F>
	struct job_function_traits {
	  private:
		using call_type = job_function_traits<decltype(&F::operator())>;

	  public:
		using return_type = typename call_type::return_type;
		static constexpr std::size_t arity = call_type::arity - 1;

		template <std::size_t N>
		struct argument {
			static_assert(N < arity, "error: invalid parameter index.");
			using type = typename call_type::template argument<N + 1>::type;
		};

		struct argT {
			using args = typename call_type::argT::args;
		};
	};

	template <typename R, typename... Args>
	struct job_function_traits<R(Args...)> {
	  public:
		using argTuple = typename std::tuple<Args...>;
		using return_type = R;

		static constexpr int32_t arity = sizeof...(Args);

		template <int N>
		struct argument {
			static_assert(N < arity, "error: invalid parameter index.");
			using type = typename std::tuple_element<N, std::tuple<Args...>>::type;
		};

		struct argT {
			using args = argTuple;
		};
	};
} // namespace nmd::job