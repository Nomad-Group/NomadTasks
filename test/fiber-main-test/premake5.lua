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

project "nomad-fiber-main-test"
    language "C++"
    kind "ConsoleApp"
	set_proj("fiber-main-test")
    targetname "fiber-main-test"
	
    targetdir "..\\..\\bin\\%{cfg.buildcfg}"
	
    vpaths { ["*"] = "*" }

    defines {
        'WIN32_LEAN_AND_MEAN',
		'PUGIXML_WCHAR_MODE',
		'FMT_HEADER_ONLY'
    }

    includedirs {
        ".",
		"../../code/fiber/include",
		"../../code/vendor/optick/src"
    }

	disablewarnings {
		"4098",
		"4217"
	}

    links {
		"nomad-fiber"
    }

    files {
        "premake5.lua",
        "**.h",
        "**.hpp",
        "**.cpp",
        "**.rc"
    }

	filter "configurations:Debug or Release"
		defines { "USE_OPTICK" }
		links { "optick" }
	
	filter {}