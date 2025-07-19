project "GLFW"
    kind "StaticLib"
    language "C"
    staticruntime "on"

    targetdir ("../../bin/" .. OutputDir .. "/%{prj.name}")
    objdir ("../../bin-int/" .. OutputDir .. "/%{prj.name}")

    files{
        "include/GLFW/glfw3.h",
        "include/GLFW/glfw3native.h",
        "src/*.c",
        "src/*.h"
    }

    includedirs{
        "include"
    }

    filter "system:windows"
        systemversion "latest"
        files{
            "src/win32_*",
            "src/wgl_context.c",
            "src/egl_context.c",
            "src/osmesa_context.c"
        }
        defines {"_GLFW_WIN32"}

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"
    
    filter "configurations:Release"
        runtime "Release"
        optimize "on"