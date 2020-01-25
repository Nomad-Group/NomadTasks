-- /*
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
 
workspace "NOMAD-TASK"
	location "./build"
    configurations { "Debug", "Release", "Shipping" }
    platforms { "x64", "x86" }
	cppdialect "c++17"
	
	symbols "On"
    characterset "Unicode"
    pic "On"
	
	flags "MultiProcessorCompile"
    vectorextensions "SSE4.1"
    debugformat "C7"
	
	filter "platforms:x64"
        architecture "x86_64"
		 
    filter "configurations:Debug"
        optimize "Debug"

    filter "configurations:Release or Shipping"
		--staticruntime "on"
        optimize "Speed"
	
	-- Generate PDB files at \build\Symbols
    filter {"system:windows", "configurations:Release", "kind:not StaticLib"}
        --linkoptions { "/PDB:\"$(OutDir)\\symbols\\$(ProjectName).pdb\"" }

    -- Enable visual styles
    -- TODO: Make this per-project
    filter { "system:windows", "kind:not StaticLib" }
        linkoptions "/manifestdependency:\"type='Win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\""

    -- Disable deprecation warnings and errors
    filter "action:vs*"
        defines
        {
            "_CRT_SECURE_NO_WARNINGS",
            "_CRT_SECURE_NO_DEPRECATE",
            "_CRT_NONSTDC_NO_WARNINGS",
            "_CRT_NONSTDC_NO_DEPRECATE",
            "_SCL_SECURE_NO_WARNINGS",
            "_SCL_SECURE_NO_DEPRECATE"
            
            --"_WINSOCK_DEPRECATED_NO_WARNINGS"
        }
		
function set_proj(desc)
	-- resource setup
	defines { "rsc_company=\"NomadGroup\"" }
    defines { "rsc_copyright=\"© NomadGroup. All rights reserved\""} 
    defines { "rsc_fileversion=\"1.0.0.0\"", "rsc_productversion=\"1.0.0.0\"" }
    defines { "rsc_internalname=\"%{prj.name}\"", "rsc_productname=\"LegionOnline\"", "rsc_originalname=\"%{prj.name}\"" }
    defines { "rsc_description=\"" .. desc .. "\"" }
end

-- Projects
group("NOMAD-TASK")
	include "code/fiber"
	include "code/threadjob"
	include "test/fiber-main-test"
	include "test/threadjob-main-test"

group "vendor"
	include "code/vendor/optick.lua"