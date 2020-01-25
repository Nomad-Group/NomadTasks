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
#include <exception>
#include <string>

namespace nmd::fiber
{
	class Exception : public std::exception
	{
		const std::string _exStr;

	public:
		Exception() throw() = default;
		Exception(const std::string& str) throw() : _exStr(str) {};
		~Exception() throw() = default;

		virtual const char* what() const override { return _exStr.c_str(); }
	};
}