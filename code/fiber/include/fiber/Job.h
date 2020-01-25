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
#include "detail/mpmc_queue.h"
#include "detail/delegate.h"
#include "Counter.h"

namespace nmd::fiber
{
	class Counter;
	namespace detail { class BaseCounter; };

	class JobInfo
	{
	public:
		// Delegate Buffer
		static constexpr size_t BufferSize = sizeof(void*) * (8);

	private:
		char m_buffer[BufferSize];
		inline detail::base_delegate* GetDelegate() { return reinterpret_cast<detail::base_delegate*>(m_buffer); };
		inline bool IsNull() const { return *(void**)m_buffer == nullptr; };

		inline void Reset()
		{
			if (!IsNull())
			{
				GetDelegate()->~base_delegate();
				*(void**)m_buffer = nullptr;
			}
		}

		// Store
		template <typename delegate_t, typename... Args>
		inline void StoreJobInfo(Args... args)
		{
			detail::delegate_size_checker<sizeof(delegate_t), BufferSize>::check();
			new(m_buffer) delegate_t(args...);
		}

		template <class TClass, typename... Args>
		inline void StoreJobInfo(TClass* inst, Args... args)
		{
			using Ret = typename detail::function_traits<TClass>::return_type;
			StoreJobInfo<typename detail::delegate_member<TClass, Ret, Args...>>(&TClass::operator(), inst, args...);
		}

		// Counter
		detail::BaseCounter* _counter = nullptr;

	public:
		JobInfo()
		{
			*(void **)m_buffer = nullptr;
		}

		// Callable class (Lambda / function with operator())
		template <typename TCallable, typename... Args>
		JobInfo(Counter* ctr, TCallable callable, Args... args) :
			_counter(ctr)
		{
			detail::function_checker<TCallable, Args...>::check();

			StoreJobInfo<typename detail::delegate_callable<TCallable, Args...>>(callable, args...);
		}

		// Function
		template <typename Ret, typename... Args>
		JobInfo(Counter* ctr, Ret(*function)(Args...), Args... args) :
			_counter(ctr)
		{
			detail::function_checker<decltype(function), Args...>::check();

			StoreJobInfo<typename detail::delegate_callable<decltype(function), Args...>>(function, args...);
		}

		// Pointer to a callable class (operator())
		template <class TCallable, typename... Args>
		JobInfo(Counter* ctr, TCallable* callable, Args... args) :
			_counter(ctr)
		{
			detail::function_checker<TCallable, Args...>::check();

			StoreJobInfo(callable, args...);
		}

		// Member Function
		template <class TClass, typename Ret, typename... Args>
		JobInfo(Counter* ctr, Ret(TClass::* callable)(Args...), TClass* inst, Args... args) :
			_counter(ctr)
		{
			detail::function_checker<decltype(callable), TClass*, Args...>::check();

			StoreJobInfo<typename detail::delegate_member<TClass, Ret, Args...>>(callable, inst, args...);
		}

		// Constructor without Counter
		template <typename TCallable, typename... Args>
		JobInfo(TCallable callable, Args... args) :
			JobInfo((Counter*)nullptr, callable, args...)
		{};

		template <typename Ret, typename... Args>
		JobInfo(Ret(*function)(Args...), Args... args) :
			JobInfo((Counter*)nullptr, function, args...)
		{};

		template <class TCallable, typename... Args>
		JobInfo(TCallable* callable, Args... args) :
			JobInfo((Counter*)nullptr, callable, args...)
		{};

		template <class TClass, typename Ret, typename... Args>
		JobInfo(Ret(TClass::* callable)(Args...), TClass* inst, Args... args) :
			JobInfo((Counter*)nullptr, callable, inst, args...)
		{};

		// copy-constructors
		JobInfo(const JobInfo &other) :
			_counter(other._counter)
		{
			memcpy(m_buffer, other.m_buffer, BufferSize);
		}

		JobInfo(JobInfo &&other) :
			_counter(other._counter)
		{
			memcpy(m_buffer, other.m_buffer, BufferSize);
		}

		// Destructor
		~JobInfo()
		{
			Reset();
		}

		// Counter
		inline void SetCounter(detail::BaseCounter* ctr)
		{
			_counter = ctr;
		}

		inline detail::BaseCounter* GetCounter() const
		{
			return _counter;
		}

		// Execute Job
		void Execute();

		// Assign Operator
		JobInfo& operator=(const JobInfo& other)
		{
			Reset();

			memcpy(m_buffer, other.m_buffer, BufferSize);
			_counter = other._counter;

			return *this;
		}
	};

	static_assert(sizeof(JobInfo) == (JobInfo::BufferSize + sizeof(void *)), "JobInfo size mismatch");

	enum class JobPriority : uint8_t
	{
		High,		// Jobs are executed ASAP
		Normal,
		Low,

		IO
	};

	namespace detail
	{
		// avoid confusion between fjs::Queue and fjs::JobQueue
		using JobQueue = detail::mpmc_queue<JobInfo>;
	}
}