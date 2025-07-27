project "ChunkDemo"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++20"
   targetdir "Binaries/%{cfg.buildcfg}"
   staticruntime "off"
   systemversion "latest"

   files { "Source/**.h", "Source/**.cpp", "Include/**.h", "Shaders/shader.frag", "Shaders/shader.vert", "**.h", "**.h"}

   includedirs
   {
      "Source",
      "Include",
      "Shaders",
       "../Vendor/glm",
       "../Vendor/glm/gtc",

	  -- Include Core
	  "../Core/Source",
      "../Vendor/glfw/include",
      "../Vendor/Glad/include",
      "../Vendor/imgui",
      "../Vendor/imgui/backends",
      "../Vendor/glfw/backends",
   }

   links
   {
      "Core",
      "GLFW",
      "Glad",
      "ImGui",
      "opengl32.lib"

   }

   targetdir ("../Binaries/" .. OutputDir .. "/%{prj.name}")
   objdir ("../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

   filter "system:windows"
       systemversion "latest"
       defines { "WINDOWS" }

   filter "configurations:Debug"
       defines { "DEBUG" }
       runtime "Debug"
       symbols "On"

   filter "configurations:Release"
       defines { "RELEASE" }
       runtime "Release"
       optimize "On"
       symbols "On"

   filter "configurations:Dist"
       defines { "DIST" }
       runtime "Release"
       optimize "On"
       symbols "Off"