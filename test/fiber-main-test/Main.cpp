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

#include <Windows.h>
#include <fiber/Counter.h>
#include <fiber/List.h>
#include <fiber/Manager.h>
#include <fiber/Queue.h>
#include <iostream>
#include <optick.h>

void test_job_1(int32_t *x)
{
	std::cout << "test_job_1 with " << *x << std::endl;
	(*x)++;
}

struct test_job_2 {
	void Execute(int32_t *x)
	{
		std::cout << "test_job_2::Execute with " << *x << std::endl;
		(*x)++;
	}

	void operator()(int32_t *x)
	{
		std::cout << "test_job_2::operator() with " << *x << std::endl;
		(*x)++;
	}
};

void main_test(nmd::fiber::Manager *mgr)
{
	OPTICK_FRAME("MainThread");
	int32_t count = 1;

	// 1: Function
	mgr->WaitForSingle(nmd::fiber::JobPriority::Normal, test_job_1, &count);

	// 2: Lambda
	for (uint16_t i = 0, length = 10000; i < length; ++i) {
		mgr->WaitForSingle(nmd::fiber::JobPriority::Normal, [&count]() {
			//std::cout << "lambda with " << count << std::endl;
			count++;
		});
	}

	// 3: Member Function
	test_job_2 tj2_inst;
	mgr->WaitForSingle(nmd::fiber::JobPriority::Normal, &test_job_2::Execute, &tj2_inst, &count);

	// 3: Class operator()
	mgr->WaitForSingle(nmd::fiber::JobPriority::Normal, &tj2_inst, &count);

	// Counter
	nmd::fiber::Counter counter(mgr);

	// It's also possible to create a JobInfo yourself
	// First argument can be a Counter
	nmd::fiber::JobInfo test_job(&counter, test_job_1, &count);
	mgr->ScheduleJob(nmd::fiber::JobPriority::Normal, test_job);
	mgr->WaitForCounter(&counter);

	// List / Queues
	nmd::fiber::List list(mgr);
	list.Add(nmd::fiber::JobPriority::Normal, test_job_1, &count);
	//list += test_job; This would be unsafe, Jobs might execute in parallel

	list.Wait();

	nmd::fiber::Queue queue(mgr, nmd::fiber::JobPriority::High); // default Priority is high
	queue.Add(test_job_1, &count);
	queue += test_job; // Safe, Jobs are executed consecutively
	queue += nmd::fiber::JobInfo(test_job_1, &count);

	queue.Execute();
}

int main()
{
	OPTICK_START_CAPTURE();

	// Setup Job Manager
	nmd::fiber::ManagerOptions managerOptions;
	managerOptions.NumFibers = managerOptions.NumThreads * 10;
	managerOptions.ThreadAffinity = true;

	managerOptions.HighPriorityQueueSize = 128;
	managerOptions.NormalPriorityQueueSize = 256;
	managerOptions.LowPriorityQueueSize = 256;

	managerOptions.ShutdownAfterMainCallback = true;

	// Manager
	nmd::fiber::Manager manager(managerOptions);

	if (manager.Run(main_test) != nmd::fiber::Manager::ReturnCode::Succes)
		std::cout << "oh no" << std::endl;
	else
		std::cout << "done" << std::endl;

	// Don't close
	OPTICK_STOP_CAPTURE();
	OPTICK_SAVE_CAPTURE("fibber-main-test.opt");
	//getchar();
	return 0;
}