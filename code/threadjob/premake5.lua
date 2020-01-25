--/*
-- * All or portions of this file Copyright (c) NOMAD Group<nomad-group.net> or its affiliates or
-- * its licensors.
-- *
-- * For complete copyright and license terms please see the LICENSE at the root of this
-- * distribution (the "License"). All use of this software is governed by the License,
-- * or, if provided, by the license below or the license accompanying this file. Do not
-- * remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
-- * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
-- *
-- */

project "nomad-threadjob"
    language "C++"
    kind "StaticLib"
	set_proj("threadjob")
    targetname "threadjob"
	
    targetdir "..\\..\\bin\\%{cfg.buildcfg}"
	
    vpaths { ["*"] = "*" }

    defines {
        'WIN32_LEAN_AND_MEAN',
		'PUGIXML_WCHAR_MODE',
		'FMT_HEADER_ONLY'
    }

    includedirs {
        ".",
		"./include"
    }

	disablewarnings {
		"4098",
		"4217"
	}
	
    files {
        "premake5.lua",
        "**.h",
        "**.hpp",
        "**.cpp",
        "**.rc"
    }