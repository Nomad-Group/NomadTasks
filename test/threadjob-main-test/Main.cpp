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

#include <threadjob/Manager.h>
#include <iostream>
#include <Windows.h>

void test_job_1(int32_t* x)
{
	std::cout << "test_job_1 with " << *x << std::endl;
	(*x)++;
}

struct test_job_2
{
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

void main_test(nmd::job::JobSystem* mgr)
{
	static int32_t counter = 0;

	for (int16_t i = 0, length = 10000; i < length; ++i) {
		mgr->AddJob([]() {
			std::cout << "job with " << counter << std::endl;
			++counter;
		});
	}
}

int main()
{
	// Manager
	nmd::job::JobSystem manager;
	main_test(&manager);
	manager.Run();

	// Don't close
	getchar();
	return 0;
}