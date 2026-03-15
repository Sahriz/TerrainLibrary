project "MarchingCubesDemo"
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
       "%{wks.location}/Vendor/glm",
       "%{wks.location}/Vendor/glm/gtc",

	  -- Include Core
	  "%{wks.location}/Core/Source",
      "%{wks.location}/Vendor/glfw/include",
      "%{wks.location}/Vendor/Glad/include",
      "%{wks.location}/Vendor/imgui",
      "%{wks.location}/Vendor/imgui/backends",
      "%{wks.location}/Vendor/glfw/backends",
      "%{wks.location}/Vendor/JoltPhysics"
   }

   links
   {
      "Core",
      "GLFW",
      "Glad",
      "ImGui",
      "opengl32.lib",
      "JoltPhysics"

   }

   targetdir ("%{wks.location}/Binaries/" .. OutputDir .. "/%{prj.name}")
   objdir ("%{wks.location}/Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

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