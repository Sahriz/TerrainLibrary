project "Core"
   kind "StaticLib"
   language "C++"
   cppdialect "C++20"
   staticruntime "off"

   files { "Source/**.h", "Source/**.cpp" , "Source/**.comp"}

   includedirs
   {
      "Source",
      "../Vendor/glm",
      "../Vendor/glm/gtc",
      "../Vendor/glfw/include",
      "../Vendor/glfw/backends",
      "../Vendor/Glad/include",
      "C:/Program Files/AMD/ROCm/7.1/include"

   }
    
    links { "VoxelAccelerator" }
    libdirs { "C:/Program Files/AMD/ROCm/7.1/lib" }
    links { "amdhip64" }

   targetdir ("../Binaries/" .. OutputDir .. "/%{prj.name}")
   objdir ("../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

   filter "system:windows"
       systemversion "latest"
       defines { }

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