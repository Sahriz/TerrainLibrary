project "VoxelAccelerator"
   kind "StaticLib"
   language "C++"
   cppdialect "C++17"
   staticruntime "off"
   
   -- Use 8.3 format to avoid space issues
   local hip_path = "C:/PROGRA~1/AMD/ROCm/7.1/"
   
   targetdir ("../Binaries/" .. OutputDir .. "/%{prj.name}")
   objdir ("../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

   files { 
      "Source/**.hip",
      "Source/**.h"     
   }

   includedirs {
      "Source",
      "../Vendor/glm",
      "C:/Program Files/AMD/ROCm/7.1/include",
      "C:/Program Files/AMD/ROCm/7.1/lib/clang/17.0.0/include",
      "C:/Program Files/AMD/ROCm/7.1/lib/clang/17.0.0/include/cuda_wrappers"
   }

   filter "system:windows"
       systemversion "latest"
       defines { "WINDOWS" }

   -- DEBUG configuration with Debug runtime
   filter { "configurations:Debug", "files:**.hip" }
       buildmessage "HIP Compiling %{file.name} (Debug)..."
       buildcommands {
             '"' .. hip_path .. 'bin/hipcc.exe" -c "%{file.abspath}" -o "%{cfg.objdir}/%{file.basename}.obj" -std=c++17 --offload-arch=gfx1201 -I"' .. hip_path .. 'include" -fms-extensions -fms-compatibility -fms-runtime-lib=dll_dbg -D_DEBUG -D_ITERATOR_DEBUG_LEVEL=2 -g'
       }
       buildoutputs { "%{cfg.objdir}/%{file.basename}.obj" }

   filter "configurations:Debug"
       defines { "DEBUG" }
       runtime "Debug"
       symbols "On"

   -- RELEASE configuration with Release runtime
   filter { "configurations:Release", "files:**.hip" }
       buildmessage "HIP Compiling %{file.name} (Release)..."
       buildcommands {
             '"' .. hip_path .. 'bin/hipcc.exe" -c "%{file.abspath}" -o "%{cfg.objdir}/%{file.basename}.obj" -std=c++17 --offload-arch=gfx1201 -I"' .. hip_path .. 'include" -fms-extensions -fms-compatibility -fms-runtime-lib=dll -DNDEBUG -D_ITERATOR_DEBUG_LEVEL=0 -O2'
       }
       buildoutputs { "%{cfg.objdir}/%{file.basename}.obj" }

   filter "configurations:Release"
       defines { "RELEASE" }
       runtime "Release"
       optimize "On"
       symbols "On"

   -- DIST configuration with Release runtime
   filter { "configurations:Dist", "files:**.hip" }
       buildmessage "HIP Compiling %{file.name} (Dist)..."
       buildcommands {
             '"' .. hip_path .. 'bin/hipcc.exe" -c "%{file.abspath}" -o "%{cfg.objdir}/%{file.basename}.obj" -std=c++17 --offload-arch=gfx1201 -I"' .. hip_path .. 'include" -fms-extensions -fms-compatibility -fms-runtime-lib=dll -DNDEBUG -D_ITERATOR_DEBUG_LEVEL=0 -O2'
       }
       buildoutputs { "%{cfg.objdir}/%{file.basename}.obj" }

   filter "configurations:Dist"
       defines { "DIST" }
       runtime "Release"
       optimize "On"
       symbols "Off"