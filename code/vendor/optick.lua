project "optick"
	targetname "optick"
	language "C++"
	kind "SharedLib"
	staticruntime "off"
	
	location "../../build/vendor"
	objdir "../../build/vendor/obj/%{prj.name}"
	targetdir "../../bin/%{cfg.buildcfg}"
	
	defines { 
		"_CRT_SECURE_NO_WARNINGS",
		"OPTICK_LIB=1",
		"OPTICK_EXPORTS",
		"OPTICK_ENABLE_GPU_D3D12=0",
		"OPTICK_ENABLE_GPU_VULKAN=0",
		"OPTICK_UWP=1",
		"USE_OPTICK=1",
		"OPTICK_FIBERS=1"
	}
	
	cppdialect "C++11"
	symbols "On"
	flags {
		"NoManifest",
		"FatalWarnings"
	}
	
	disablewarnings {
		"4127", -- Conditional expression is constant
		"4091", -- 'typedef ': ignored on left of '' when no variable is declared
	}
	
	vpaths {
		["api"] = { 
			"./optick/src/optick.h",
			"./optick/src/optick.config.h",
		},
	}
	
	files {
		"./optick/src/**"
	}
	
	configuration "Release"
		defines {
			"NDEBUG",
			"MT_INSTRUMENTED_BUILD"
		}
		optimize "Speed"
	
	configuration "Debug"
		defines {
			"_DEBUG",
			"_CRTDBG_MAP_ALLOC",
			"MT_INSTRUMENTED_BUILD"
		}
