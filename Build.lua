-- premake5.lua
workspace "TerrainLibrary"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "App"
   OutputDir = "%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"
   -- Workspace-wide build options for MSVC
   filter "system:windows"
      buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }



group "Core"
	include "Core/Build-Core.lua"
group ""

include "HeightMapDemo/Build-HeightMapDemo.lua"

include "ChunkDemo/Build-ChunkDemo.lua"

include "Vendor/glfw/Build-glfw.lua"

include "Vendor/Glad/Build-Glad.lua"

include "Vendor/imgui/Build-imgui.lua"